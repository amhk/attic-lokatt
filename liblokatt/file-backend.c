#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "adb.h"
#include "backend.h"
#include "lokatt.h"

/*
 * If the file descriptor returned from open(2) in create_file_backend is 0,
 * the function return value is indistinguishable to NULL. Define two macros to
 * add/subtract a constant to the actual file descriptor value to eliminate
 * this ambiguity.
 */
#define FD_TO_VOIDP(fd) ((void *)((fd) + 1))
#define VOIDP_TO_FD(p) ((int)((p) - 1))

void *create_file_backend(const char *path)
{
	intptr_t fd;
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return NULL;
	return FD_TO_VOIDP(fd);
}

static void destroy(void *userdata)
{
	int fd = VOIDP_TO_FD(userdata);
	close(fd);
}

static int next_logcat_message(void *userdata, struct lokatt_message *out)
{
	int fd = VOIDP_TO_FD(userdata);
	return adb_read_lokatt_message(fd, out);
}

static int pid_to_name(void *userdata, uint32_t pid, char out[128])
{
	/* TODO: implement this */
	(void)userdata;
	(void)pid;
	(void)out;
	return -1;
}

struct backend_ops file_backend_ops = {
	.destroy = destroy,
	.next_logcat_message = next_logcat_message,
	.pid_to_name = pid_to_name,
};
