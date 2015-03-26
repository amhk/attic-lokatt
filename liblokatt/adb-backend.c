#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "backend.h"
#include "lokatt.h"

/*
 * This struct is taken from Android (system/core/include/log/logger.h); lokatt
 * silently discards the extra uint32_t present in logger_entry_v2 and
 * logger_entry_v3 structs so as to coalesce all three versions into one and
 * the same.
 */
struct logger_entry {
	uint16_t len;
	uint16_t __pad;
	int32_t pid;
	int32_t tid;
	int32_t sec;
	int32_t nsec;
	char msg[0];
} __attribute__((__packed__));

#define decode_logcat_payload(payload_ptr, level_ptr, tag_ptr, text_ptr) \
	do { \
		char *p; \
		\
		*(level_ptr) = (uint8_t)(((const char *)(payload_ptr))[0]); \
		(tag_ptr) = (const char *)(((const char *)(payload_ptr)) + 1); \
		(text_ptr) = (const char *)(strchr((tag_ptr), '\0') + 1); \
		\
		/* also strip trailing newlines from text */ \
		p = strchr(text_ptr, '\0') - 1; \
		while (*p == '\n') { \
			*p-- = '\0'; \
		} \
	} while (0)

#define TEMP_FAILURE_RETRY(exp) \
	({ \
	 typeof (exp) _rc; \
	 do { \
	 _rc = (exp); \
	 } while (_rc == -1 && errno == EINTR); \
	 _rc; })

struct self {
	struct {
		pid_t pid;
		int stdout[2];
		int stderr[2];
	} adb;
};

/* read and write ends of a pipe */
#define R 0
#define W 1

static int start_adb_logcat(struct self *self)
{
	self->adb.stdout[R] = 0;
	self->adb.stdout[W] = 0;
	self->adb.stderr[R] = 0;
	self->adb.stderr[W] = 0;

	if (pipe(self->adb.stdout) < 0)
		goto bail;
	if (pipe(self->adb.stderr) < 0)
		goto bail;

	self->adb.pid = fork();
	switch (self->adb.pid) {
	case -1:
		/* error */
		goto bail;
	case 0:
		/* child */
		close(self->adb.stdout[R]);
		close(self->adb.stderr[R]);

		dup2(self->adb.stdout[W], STDOUT_FILENO);
		dup2(self->adb.stderr[W], STDERR_FILENO);

		execlp("adb", "adb", "exec-out", "logcat", "-B", NULL);

		exit(EXIT_FAILURE);
	default:
		/* parent */
		close(self->adb.stdout[W]);
		close(self->adb.stderr[W]);
		break;
	}
	return 0;
bail:
	close(self->adb.stdout[R]);
	close(self->adb.stdout[W]);
	close(self->adb.stderr[R]);
	close(self->adb.stderr[W]);
	return -1;
};

static ssize_t xread(int fd, void *buf, size_t count)
{
	ssize_t bytes_left = count;
	while (bytes_left > 0) {
		ssize_t r;

		r = TEMP_FAILURE_RETRY(read(fd, buf + count - bytes_left,
					    bytes_left));
		if (r < 0)
			return -1;
		bytes_left -= r;
	}
	return count;
}

void *create_adb_backend(const char *serialno)
{
	(void)serialno; /* TODO: use this */
	struct self *self = calloc(1, sizeof(*self));

	if (start_adb_logcat(self) < 0) {
		free(self);
		return NULL;
	}

	return self;
}

static void destroy(void *userdata)
{
	free(userdata);
}

static int next_logcat_message(void *userdata, struct lokatt_message *out)
{
	struct logger_entry header;
	struct self *self = userdata;
	ssize_t r;
	uint32_t skip;
	char buf[4 * 1024];

	uint8_t level;
	const char *tag, *text;

	r = xread(self->adb.stdout[R], &header, sizeof(header));
	if (r != sizeof(header))
		return -1;

	/*
	 * Try to guess if we're reading v2 or v3 headers by looking at the
	 * padding, which is used to store the header size in v2 and v3. If we
	 * actually are reading any of the newer formats, skip the extra
	 * uint32_t present just before the payload.
	 */
	if (header.__pad != 0)
		xread(self->adb.stdout[R], (char *)&skip, sizeof(skip));

	r = xread(self->adb.stdout[R], buf, header.len);
	if (r != header.len)
		return -1;

	decode_logcat_payload(buf, &level, tag, text);

	out->pid = header.pid;
	strncpy(out->text, text, 128);
	out->text[127] = '\0';

	return 0;
}

static int pid_to_name(void *userdata, uint32_t pid, char out[128])
{
	/* TODO: implement this */
	(void)userdata;
	(void)pid;
	(void)out;
	return -1;
}

struct backend_ops adb_backend_ops = {
	.destroy = destroy,
	.next_logcat_message = next_logcat_message,
	.pid_to_name = pid_to_name,
};
