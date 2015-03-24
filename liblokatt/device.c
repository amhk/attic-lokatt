#include <inttypes.h>
#include <pthread.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend.h"
#include "index.h"
#include "lokatt.h"

struct lokatt_device {
	void *backend;
	struct backend_ops *ops;

	pthread_t logcat_thread;
	pthread_rwlock_t lock;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	struct index index;
};

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static void logcat_thread_sighandler(int signum)
{
	printf("thread got signal %d\n", signum);
	pthread_setspecific(key, (void *)1);
}

static void *logcat_thread_main(void *arg)
{
	struct lokatt_device *dev = (struct lokatt_device *)arg;
	struct lokatt_message msg;
	struct lokatt_event event;
	int status;
	int (*fn)(void *, struct lokatt_message *) = dev->ops->next_logcat_message;

	signal(SIGQUIT, logcat_thread_sighandler);
	event.type = EVENT_LOGCAT_MESSAGE;

	while (!pthread_getspecific(key)) {
		status = fn(dev->backend, &msg);
		if (status != 0)
			continue;

		pthread_rwlock_wrlock(&dev->lock);

		memcpy(&event.msg, &msg, sizeof(struct lokatt_message));
		index_append(&dev->index, &event);

		pthread_mutex_lock(&dev->mutex);
		pthread_cond_broadcast(&dev->cond);
		pthread_mutex_unlock(&dev->mutex);
		pthread_rwlock_unlock(&dev->lock);
	}
	printf("logcat_thread_main about to return\n");
	return NULL;
}

static void init_pthreads()
{
	pthread_key_create(&key, NULL);
}

static struct lokatt_device *create_device(void *initialized_backend,
					   struct backend_ops *ops)
{
	struct lokatt_device *dev;

	pthread_once(&key_once, init_pthreads);

	dev = calloc(1, sizeof(*dev));
	dev->backend = initialized_backend;
	dev->ops = ops;
	index_init(&dev->index);
	pthread_rwlock_init(&dev->lock, NULL);
	pthread_cond_init(&dev->cond, NULL);
	pthread_mutex_init(&dev->mutex, NULL);
	pthread_create(&dev->logcat_thread, NULL, logcat_thread_main, dev);

	return dev;
}

struct lokatt_device *lokatt_open_adb_device(const char *serialno)
{
	struct lokatt_device *dev;
	void *backend = create_adb_backend(serialno);
	if (!backend)
		return NULL;
	dev = create_device(backend, &adb_backend_ops);
	if (!dev) {
		adb_backend_ops.destroy(backend);
	}
	return dev;
}

struct lokatt_device *lokatt_open_dummy_device()
{
	struct lokatt_device *dev;
	void *backend = create_dummy_backend();
	if (!backend)
		return NULL;
	dev = create_device(backend, &dummy_backend_ops);
	if (!dev) {
		dummy_backend_ops.destroy(backend);
	}
	return dev;
}

struct lokatt_device *lokatt_open_file(const char *path)
{
	/* TODO: implement this */
	(void)path;
	return NULL;
}

void lokatt_close_device(struct lokatt_device *dev)
{
	pthread_kill(dev->logcat_thread, SIGQUIT);
	pthread_join(dev->logcat_thread, NULL);
	pthread_cond_destroy(&dev->cond);
	pthread_mutex_destroy(&dev->mutex);
	pthread_rwlock_destroy(&dev->lock);
	index_destroy(&dev->index);
	dev->ops->destroy(dev->backend);

	free(dev);
}

uint64_t lokatt_next_event(struct lokatt_device *dev,
			   uint64_t id,
			   unsigned int event_filter_bitmask,
			   struct lokatt_event *out)
{
	(void)event_filter_bitmask;
	const struct lokatt_event *event = NULL;

	while (!event) {
		pthread_rwlock_rdlock(&dev->lock);
		event = index_get(&dev->index, id++);
		if (event && !(event->type & event_filter_bitmask))
			event = NULL;
		if (!event) {
			pthread_mutex_lock(&dev->mutex);
			pthread_rwlock_unlock(&dev->lock);
			pthread_cond_wait(&dev->cond, &dev->mutex);
			pthread_mutex_unlock(&dev->mutex);
		}
	}
	memcpy(out, event, sizeof(*out));
	pthread_rwlock_unlock(&dev->lock);

	return 0;
}
