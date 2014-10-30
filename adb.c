#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

#include "adb.h"

/* LOGGER_ENTRY_MAX_LEN should be read from adb logcat -g */
#define LOGGER_ENTRY_MAX_LEN (5 * 1024)
#define BUFFER_SIZE (10 * LOGGER_ENTRY_MAX_LEN)

static char g_buf[BUFFER_SIZE];
static char *g_begin = g_buf;
static char *g_end = g_buf;
static int g_trailing_cr = 0;

/*
 * Due to Android shell's tty settings, every new line character (0x0a) is
 * rewritten as carriage return + new line (0x0d 0x0a). This is annoying when
 * scripting adb output (tr -d '\r' usually works), but downright wrong when
 * streaming binary data.
 *
 * To unescape the extra carriage returns, we read adb output in two steps:
 * xread keeps track of a raw buffer from which xread1 reads up to count number
 * of bytes. Should the last byte in the raw buffer be 0x0d we cannot return it
 * before reading the next character: if what follows 0x0d is 0x0a, the 0x0d
 * character should be stripped from output.
 *
 * The message payload of a logger_entry structure is always null terminated.
 * This means we run no risk of hanging when reading a logger_entry entry.
 */
static ssize_t xread(int fd, char *buf, size_t count)
{
	assert(g_begin <= g_end);

	if (g_begin == g_end) {
		char *p = g_buf;
		size_t n = BUFFER_SIZE;
		ssize_t r = 0;

		if (g_trailing_cr) {
			*p++ = 0x0d;
			n--;
		}
		r = read(fd, p, n);
		if (r < 0)
			return -1;
		g_begin = g_buf;
		g_end = g_begin + (p - g_begin) + r;

		/* compact 0x0d 0x0a to 0x0a */
		p = g_begin;
		while (p < g_end - 1) {
			if (*p == 0x0d && *(p + 1) == 0x0a) {
				char *q = p;
				while (q < g_end - 1) {
					*q = *(q + 1);
					q++;
				}
				g_end--;
			}
			p++;
		}

		if (*(g_end - 1) == 0x0d) {
			g_trailing_cr = 1;
			g_end--;
		} else {
			g_trailing_cr = 0;
		}
	}

	if ((size_t)(g_end - g_begin) < count)
		count = g_end - g_begin;
	memcpy(buf, g_begin, count);
	g_begin += count;
	return count;
}

static ssize_t xread1(int fd, char *buf, size_t count)
{
	const size_t requested_count = count;
	buf += count;
	for (;;) {
		ssize_t r = xread(fd, buf - count, count);
		if (r < 0)
			return -1;
		if (r == 0)
			return 0;
		count -= r;
		if (count == 0)
			return requested_count;
	}
}

int read_logcat(int fd, struct logger_entry *header, char *payload, size_t size)
{
	ssize_t r;
	uint32_t skip;

	r = xread1(fd, (char *)header, sizeof(struct logger_entry));
	if (r <= 0)
	    return r;

	/*
	 * Try to guess if we're reading v2 or v3 headers by looking at the
	 * padding, which is used to store the header size in v2 and v3. If we
	 * actually are reading any of the newer formats, skip the extra
	 * uint32_t present just before the payload.
	 */
	if (header->__pad != 0)
		xread(fd, (char *)&skip, sizeof(uint32_t));

	if (size < header->len)
		/* we're lost, next call to read_logcat will fail */
		return -1;

	r = xread1(fd, (char *)payload, header->len);
	if (r <= 0)
		return -1;

	return header->len;
}

struct adb {
	pid_t pid;
	int fd;
	adb_cb cb;
	void *userdata;

	pthread_t thread;
};

static void *thread_main(void *arg)
{
	struct adb *adb = (struct adb *)arg;

	for (;;) {
		struct logger_entry header;
		char buf[4 * 1024];
		int payload_size;

		payload_size = read_logcat(adb->fd, &header, buf, sizeof(buf));
		if (payload_size <= 0)
			break;
		adb->cb(&header, buf, payload_size, adb->userdata);
	}

	return NULL;
}

struct adb *create_adb(adb_cb cb, void *userdata, const char *content_path)
{
	struct adb *adb;
	int pipe_fd[2];
	pid_t pid;

	if (pipe(pipe_fd) < 0)
		return NULL;

	pid = fork();
	switch (pid) {
	case -1:
		/* error */
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return NULL;
	case 0:
		/* child */
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		if (content_path) {
			int fd;
			char buf[1024];
			ssize_t r;

			fd = open(content_path, O_RDONLY);
			if (fd < 0)
				exit(EXIT_FAILURE);
			while ((r = read(fd, buf, sizeof(buf))) > 0) {
				usleep(10);
				write(STDOUT_FILENO, buf, r);
			}
			close(fd);
			exit(EXIT_SUCCESS);
		} else {
			execlp("adb", "adb", "logcat", "-B", NULL);
			exit(EXIT_FAILURE);
		}
	default:
		/* parent */
		close(pipe_fd[1]);
		break;
	}


	adb = malloc(sizeof(*adb));
	adb->pid = pid;
	adb->fd = pipe_fd[0];
	adb->cb = cb;
	adb->userdata = userdata;
	pthread_create(&adb->thread, NULL, thread_main, (void *)adb);
	return adb;
}

void destroy_adb(struct adb *adb)
{
	kill(adb->pid, SIGKILL);
	waitpid(adb->pid, NULL, 0);
	pthread_join(adb->thread, NULL);
	free(adb);
}
