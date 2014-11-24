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

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* LOGGER_ENTRY_MAX_LEN should be read from adb logcat -g */
#define LOGGER_ENTRY_MAX_LEN (5 * 1024)

struct adb_stream {
	int fd;
	char buf[10 * LOGGER_ENTRY_MAX_LEN];
	char *begin, *end;
	int trailing_cr;
};

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
static size_t compact_crnl_to_nl(char *buf, size_t count, int *trailing_cr)
{
	char *p = buf;
	char *end = p + count;

	while (p < end - 1) {
		if (*p == 0x0d && *(p + 1) == 0x0a) {
			memmove(p, p + 1, end - p - 1);
			end--;
		}
		p++;
	}

	*trailing_cr = *(end - 1) == 0x0d;
	return end - buf;
}

static ssize_t xread(struct adb_stream *stream, char *buf, size_t count)
{
	/* populate buffer if nothing left since previous xread */
	if (stream->begin == stream->end) {
		ssize_t r;
		size_t n = sizeof(stream->buf);

		stream->begin = stream->buf;
		if (stream->trailing_cr) {
			*stream->begin++ = 0x0d;
			n--;
		}

		r = read(stream->fd, stream->begin, n);
		if (r < 0)
			return -1;

		n = compact_crnl_to_nl(stream->buf, r, &stream->trailing_cr);
		stream->end = stream->buf + n - (stream->trailing_cr ? 1 : 0);
	}

	/* copy data to out parameters */
	count = MIN((size_t)(stream->end - stream->begin), count);
	memcpy(buf, stream->begin, count);
	stream->begin += count;
	return count;
}

static ssize_t read_stream(struct adb_stream *stream, char *buf, size_t count)
{
	const size_t requested_count = count;
	buf += count;
	for (;;) {
		ssize_t r = xread(stream, buf - count, count);
		if (r < 0)
			return -1;
		if (r == 0)
			return 0;
		count -= r;
		if (count == 0)
			return requested_count;
	}
}

struct adb_stream *create_adb_stream(int open_fd)
{
	struct adb_stream *stream = malloc(sizeof(*stream));

	stream->fd = open_fd;
	stream->begin = stream->end = stream->buf;
	stream->trailing_cr = 0;

	return stream;
}

void destroy_adb_stream(struct adb_stream *stream)
{
	free(stream);
}

int read_logcat(struct adb_stream *stream, struct logger_entry *header,
		char *payload, size_t size)
{
	ssize_t r;
	uint32_t skip;

	r = read_stream(stream, (char *)header, sizeof(struct logger_entry));
	if (r <= 0)
	    return r;

	/*
	 * Try to guess if we're reading v2 or v3 headers by looking at the
	 * padding, which is used to store the header size in v2 and v3. If we
	 * actually are reading any of the newer formats, skip the extra
	 * uint32_t present just before the payload.
	 */
	if (header->__pad != 0)
		read_stream(stream, (char *)&skip, sizeof(uint32_t));

	if (size < header->len)
		/* we're lost, next call to read_logcat will fail */
		return -1;

	r = read_stream(stream, (char *)payload, header->len);
	if (r <= 0)
		return -1;

	return header->len;
}

struct adb {
	pid_t pid;
	int fd;
	adb_cb cb;
	void *userdata;
	struct adb_stream *stream;

	pthread_t thread;
};

static void *thread_main(void *arg)
{
	struct adb *adb = (struct adb *)arg;

	for (;;) {
		struct logger_entry header;
		char buf[4 * 1024];
		int payload_size;

		payload_size = read_logcat(adb->stream, &header,
					   buf, sizeof(buf));
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
	adb->stream = create_adb_stream(adb->fd);
	pthread_create(&adb->thread, NULL, thread_main, (void *)adb);
	return adb;
}

void destroy_adb(struct adb *adb)
{
	kill(adb->pid, SIGKILL);
	waitpid(adb->pid, NULL, 0);
	pthread_join(adb->thread, NULL);
	destroy_adb_stream(adb->stream);
	free(adb);
}
