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

struct subprocess {
	pid_t pid;
	int stdin[2];
	int stdout[2];
	int stderr[2];

	adb_cb cb;
	void *userdata;
	struct adb_stream *stream;

	pthread_t thread;
};

/* read and write ends of a pipe */
#define R 0
#define W 1

struct adb {
	struct subprocess logcat;
};

static void *logcat_thread(void *arg)
{
	struct subprocess *proc = (struct subprocess *)arg;

	for (;;) {
		struct logger_entry header;
		char buf[4 * 1024];
		int payload_size;

		payload_size = read_logcat(proc->stream, &header,
					   buf, sizeof(buf));
		if (payload_size <= 0)
			break;
		proc->cb(&header, buf, payload_size, proc->userdata);
	}

	return NULL;
}

static int create_subprocess(struct subprocess *proc, void *(*thread)(void *),
			     adb_cb cb, void *userdata, char *const argv[])
{
	proc->stdin[0] = 0;
	proc->stdin[1] = 0;
	proc->stdout[0] = 0;
	proc->stdout[1] = 0;
	proc->stderr[0] = 0;
	proc->stderr[1] = 0;

	if (pipe(proc->stdin) < 0)
		goto bail;
	if (pipe(proc->stdout) < 0)
		goto bail;
	if (pipe(proc->stderr) < 0)
		goto bail;

	proc->pid = fork();
	switch (proc->pid) {
	case -1:
		/* error */
		goto bail;
	case 0:
		/* child */
		close(proc->stdin[W]);
		close(proc->stdout[R]);
		close(proc->stderr[R]);

		dup2(proc->stdin[R], STDIN_FILENO);
		dup2(proc->stdout[W], STDOUT_FILENO);
		dup2(proc->stderr[W], STDERR_FILENO);

		execvp("adb", argv);

		exit(EXIT_FAILURE);
	default:
		/* parent */
		close(proc->stdin[R]);
		close(proc->stdout[W]);
		close(proc->stderr[W]);
	}

	proc->cb = cb;
	proc->userdata = userdata;
	proc->stream = create_adb_stream(proc->stdout[R]);
	pthread_create(&proc->thread, NULL, thread, proc);
	return 0;
bail:
	close(proc->stdin[0]);
	close(proc->stdin[1]);
	close(proc->stdout[0]);
	close(proc->stdout[1]);
	close(proc->stderr[0]);
	close(proc->stderr[1]);
	return -1;
}

static void destroy_subprocess(const struct subprocess *proc)
{
	kill(proc->pid, SIGKILL);
	waitpid(proc->pid, NULL, 0);
	pthread_join(proc->thread, NULL);
	destroy_adb_stream(proc->stream);
}

struct adb *create_adb(adb_cb cb, void *userdata)
{
	struct adb *adb;
	adb = malloc(sizeof(*adb));

	char *argv[] = { "adb", "logcat", "-B", NULL };
	if (create_subprocess(&adb->logcat, logcat_thread,
			      cb, userdata, argv) < 0) {
		free(adb);
		return NULL;
	}

	return adb;
}

void destroy_adb(struct adb *adb)
{
	destroy_subprocess(&adb->logcat);
	free(adb);
}
