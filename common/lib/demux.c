#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adb.h"
#include "dict.h"
#include "lokatt.h"
#include "ring-buffer.h"
#include "strbuf.h"

static void adb_logcat_cb(const struct logger_entry *entry, const char *payload,
			  size_t payload_size, void *userdata);

struct lokatt_session {
	struct ring_buffer *rb;
	int is_active;
	struct adb *adb;
	struct dict *pnames;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_rwlock_t lock;
};

struct lokatt_channel {
	struct lokatt_session *s;
	struct ring_buffer_iterator *iter;
	int is_closed;
	pthread_mutex_t mutex;
};

static void lookup_process_name(const struct lokatt_session *s, uint32_t pid,
				char *buf, size_t size)
{
	struct strbuf sb;
	char cmdline[128];

	sprintf(cmdline, "cat /proc/%" PRIu32 "/cmdline", pid);
	strbuf_init(&sb, size);
	memset(buf, 0, size);
	if (adb_shell(s->adb, cmdline, &sb) == 0) {
		strncpy(buf, sb.buf, size);
	} else {
		strncpy(buf, "???", size);
	}
	strbuf_destroy(&sb);
}

static void get_process_name(const struct lokatt_session *s, uint32_t pid,
			     char *buf, size_t size)
{
	const char *pname;

	pname = dict_get(s->pnames, pid);
	if (pname) {
		strncpy(buf, pname, size);
	} else {
		lookup_process_name(s, pid, buf, size);
		dict_put(s->pnames, pid, strdup(buf));
	}
}


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
	get_process_name(s, m.pid, m.pname, sizeof(m.pname));
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
	s->pnames = dict_create(4219);
	s->adb = create_adb(adb_logcat_cb, s);
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
	/* start shutdown of adb logcat thread */
	destroy_adb(s->adb);

	/* wait for any ongoing adb callbacks to finish before moving on */
	pthread_rwlock_wrlock(&s->lock);
	dict_destroy(s->pnames, free);
	destroy_ring_buffer(s->rb);
	pthread_rwlock_unlock(&s->lock);

	pthread_rwlock_destroy(&s->lock);
	pthread_cond_destroy(&s->cond);
	pthread_mutex_destroy(&s->mutex);
	free(s);
}

struct lokatt_channel *create_lokatt_channel(struct lokatt_session *s)
{
	struct lokatt_channel *c;

	c = malloc(sizeof(*c));
	c->s = s;
	c->iter = create_ring_buffer_iterator(s->rb);
	c->is_closed = 0;
	pthread_mutex_init(&c->mutex, NULL);
	return c;
}

void close_lokatt_channel(struct lokatt_channel  *c)
{
	struct lokatt_session *s = c->s;
	pthread_mutex_lock(&c->mutex);
	c->is_closed = 1;
	pthread_mutex_unlock(&c->mutex);
	/* wake all threads waiting for new messages to give closed channels a
	 * chance to finish gracefully */
	pthread_mutex_lock(&s->mutex);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
}

int is_closed(struct lokatt_channel *c)
{
	int closed;
	pthread_mutex_lock(&c->mutex);
	closed = c->is_closed;
	pthread_mutex_unlock(&c->mutex);
	return closed;
}

int read_lokatt_channel(const struct lokatt_channel *c,
			 struct lokatt_message *m)
{
	int retval = 0;

	for (;;) {
		ssize_t status;

		pthread_rwlock_rdlock(&c->s->lock);

		/* is the channel still open? */
		if (c->is_closed)
		{
			retval = -1;
			break;
		}

		/* is the session still active? */
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
	destroy_ring_buffer_iterator(c->iter);
	free(c);
}
