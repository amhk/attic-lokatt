#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>

#include "logcat.h"
#include "lokatt.h"
#include "ring-buffer.h"

enum {
	CTL_STOP
};

static void *thread_main(void *arg);
static int fork_and_exec_adb();
static int read_ctl(int fd);
static void write_ctl(int fd, int value);
static void write_session(struct lokatt_session *s, const char *buf,
			  size_t size);
static int read_adb_output(struct lokatt_session *s, int fd);

struct lokatt_session {
	/* access to members 'rb' and 'is_active' is guarded by 'lock' */
	struct ring_buffer *rb;
	int is_active;

	int ctl_pipe[2];
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_rwlock_t lock;
};

struct lokatt_channel {
	struct lokatt_session *s;
	struct ring_buffer_iterator *iter;
};

struct lokatt_session *create_lokatt_session(size_t buffer_size)
{
	struct lokatt_session *s;

	s = malloc(sizeof(*s));
	if (pipe(s->ctl_pipe) < 0) {
		free(s);
		return NULL;
	}
	s->rb = create_ring_buffer(buffer_size);
	s->is_active = 1;
	pthread_mutex_init(&s->mutex, NULL);
	pthread_cond_init(&s->cond, NULL);
	pthread_rwlock_init(&s->lock, NULL);
	return s;
}

void start_lokatt_session(struct lokatt_session *s)
{
	pthread_create(&s->thread, NULL, thread_main, (void *)s);
}

void stop_lokatt_session(struct lokatt_session *s)
{
	write_ctl(s->ctl_pipe[1], CTL_STOP);
	pthread_join(s->thread, NULL);
	close(s->ctl_pipe[0]);
	close(s->ctl_pipe[1]);
}

void destroy_lokatt_session(struct lokatt_session *s)
{
	pthread_rwlock_destroy(&s->lock);
	pthread_cond_destroy(&s->cond);
	pthread_mutex_destroy(&s->mutex);
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

static void *thread_main(void *arg)
{
	struct lokatt_session *s = arg;
	const int ctl_fd = s->ctl_pipe[0];
	int adb_fd;
	fd_set read_fds;
	enum {
		STATE_CTL_STOP_RECIVED,
		STATE_CHILD_DIED,
		STATE_CHILD_IS_ALIVE
	} state = STATE_CHILD_IS_ALIVE;

	adb_fd = fork_and_exec_adb();
	if (adb_fd < 0)
		return NULL;

	while (state > STATE_CTL_STOP_RECIVED) {
		int ndfs = ctl_fd;
		FD_ZERO(&read_fds);
		FD_SET(ctl_fd, &read_fds);
		if (state == STATE_CHILD_IS_ALIVE) {
			FD_SET(adb_fd, &read_fds);
			if (ndfs < adb_fd)
				ndfs = adb_fd;
		}
		if (select(ndfs + 1, &read_fds, NULL, NULL, NULL) < 0)
			return NULL;

		if (FD_ISSET(ctl_fd, &read_fds)) {
			int ctl;
			ctl = read_ctl(ctl_fd);
			if (ctl == CTL_STOP) {
				state = STATE_CTL_STOP_RECIVED;
				pthread_rwlock_wrlock(&s->lock);
				s->is_active = 0;
				pthread_mutex_lock(&s->mutex);
				pthread_cond_broadcast(&s->cond);
				pthread_mutex_unlock(&s->mutex);
				pthread_rwlock_unlock(&s->lock);
			}
		}

		if (s->is_active && FD_ISSET(adb_fd, &read_fds)) {
			if (read_adb_output(s, adb_fd) <= 0)
				state = STATE_CHILD_DIED;
		}
	}

	close(adb_fd);

	return NULL;
}

static int fork_and_exec_adb()
{
	int pipe_fd[2];
	pid_t pid;

	if (pipe(pipe_fd) < 0)
		return -1;

	switch (pid = fork()) {
	case -1:
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return -1;
	case 0:
		/* child */
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		execlp("adb", "adb", "logcat", "-B", NULL);
		exit(EXIT_FAILURE);
	default:
		/* parent */
		close(pipe_fd[1]);
		return pipe_fd[0];
	}
}

static int read_ctl(int fd)
{
	int value;
	read(fd, &value, sizeof(int));
	return value;
}

static void write_ctl(int fd, int value)
{
	write(fd, &value, sizeof(int));
}

static void write_session(struct lokatt_session *s, const char *buf,
			  size_t size)
{
	pthread_rwlock_wrlock(&s->lock);
	write_ring_buffer(s->rb, buf, size);
	pthread_mutex_lock(&s->mutex);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	pthread_rwlock_unlock(&s->lock);
}

static int read_adb_output(struct lokatt_session *s, int fd)
{
	struct logger_entry entry;
	char buf[MSG_MAX_PAYLOAD_SIZE];
	int payload_size;

	payload_size = read_logcat(fd, &entry, buf, sizeof(buf));
	if (payload_size > 0) {
		struct lokatt_message m;
		size_t size = sizeof(m) - MSG_MAX_PAYLOAD_SIZE + payload_size;

		m.pid = entry.pid;
		m.tid = entry.tid;
		m.sec = entry.sec;
		m.nsec = entry.nsec;
		memcpy(m.msg, buf, payload_size);

		write_session(s, (const char *)&m, size);
	}
	return payload_size;
}
