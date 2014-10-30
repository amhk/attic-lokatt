#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "adb.h"
#include "lokatt.h"
#include "ring-buffer.h"

static void adb_logcat_cb(const struct logger_entry *entry, const char *payload,
			  size_t payload_size, void *userdata);

struct lokatt_session {
	struct ring_buffer *rb;
	int is_active;
	struct adb *adb;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_rwlock_t lock;
};

struct lokatt_channel {
	struct lokatt_session *s;
	struct ring_buffer_iterator *iter;
};

static void adb_logcat_cb(const struct logger_entry *entry, const char *payload,
			  size_t payload_size, void *userdata)
{
	struct lokatt_message m;
	size_t message_size = sizeof(m) - MSG_MAX_PAYLOAD_SIZE + payload_size;
	struct lokatt_session *s = (struct lokatt_session *)userdata;

	m.pid = entry->pid;
	m.tid = entry->tid;
	m.sec = entry->sec;
	m.nsec = entry->nsec;
	memcpy(m.msg, payload, payload_size);

	pthread_rwlock_wrlock(&s->lock);
	write_ring_buffer(s->rb, &m, message_size);
	pthread_mutex_lock(&s->mutex);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	pthread_rwlock_unlock(&s->lock);
}

struct lokatt_session *create_lokatt_session(size_t buffer_size)
{
	struct lokatt_session *s;

	s = malloc(sizeof(*s));
	s->rb = create_ring_buffer(buffer_size);
	s->is_active = 0;
	s->adb = create_adb(adb_logcat_cb, s, NULL);
	pthread_mutex_init(&s->mutex, NULL);
	pthread_cond_init(&s->cond, NULL);
	pthread_rwlock_init(&s->lock, NULL);
	return s;
}

void start_lokatt_session(struct lokatt_session *s)
{
	pthread_rwlock_wrlock(&s->lock);
	s->is_active = 1;
	pthread_mutex_lock(&s->mutex);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	pthread_rwlock_unlock(&s->lock);
}

void stop_lokatt_session(struct lokatt_session *s)
{
	pthread_rwlock_wrlock(&s->lock);
	s->is_active = 0;
	pthread_mutex_lock(&s->mutex);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	pthread_rwlock_unlock(&s->lock);
}

void destroy_lokatt_session(struct lokatt_session *s)
{
	pthread_rwlock_destroy(&s->lock);
	pthread_cond_destroy(&s->cond);
	pthread_mutex_destroy(&s->mutex);
	destroy_adb(s->adb);
	destroy_ring_buffer(s->rb);
	free(s);
}

struct lokatt_channel *create_lokatt_channel(struct lokatt_session *s)
{
	struct lokatt_channel *c;

	c = malloc(sizeof(*c));
	c->s = s;
	c->iter = create_ring_buffer_iterator(s->rb);
	return c;
}

int read_lokatt_channel(const struct lokatt_channel *c,
			 struct lokatt_message *m)
{
	int retval = 0;

	for (;;) {
		ssize_t status;

		/* is the session still active? */
		pthread_rwlock_rdlock(&c->s->lock);
		if (!c->s->is_active) {
			retval = -1;
			break;
		}

		/* anything new in the ring buffer? */
		status = read_ring_buffer_iterator(c->iter, m, sizeof(*m));
		if (status >= 0) {
			retval = 0;
			break;
		}

		/* nothing new in ring buffer, wait for new data to arrive */
		pthread_mutex_lock(&c->s->mutex);
		pthread_rwlock_unlock(&c->s->lock);
		pthread_cond_wait(&c->s->cond, &c->s->mutex);
		pthread_mutex_unlock(&c->s->mutex);
	}
	pthread_rwlock_unlock(&c->s->lock);
	decode_logcat_payload(m->msg, &m->level, m->tag, m->text);

	return retval;
}

void destroy_lokatt_channel(struct lokatt_channel *c)
{
	free(c);
}
