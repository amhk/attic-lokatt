#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "adb.h"
#include "backend.h"
#include "lokatt.h"

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
	struct self *self = userdata;
	return adb_read_lokatt_message(self->adb.stdout[R], out);
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
