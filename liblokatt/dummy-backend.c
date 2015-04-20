#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "backend.h"

void *create_dummy_backend(const char *path)
{
	srand(time(NULL));
	return create_file_backend(path);
}

static void destroy(void *userdata)
{
	file_backend_ops.destroy(userdata);
}

static int next_logcat_message(void *userdata, struct lokatt_message *out)
{
	usleep((rand() % 200) * 1000 + 100 * 1000);
	return file_backend_ops.next_logcat_message(userdata, out);
}

static int pid_to_name(void *userdata, uint32_t pid, char out[128])
{
	return file_backend_ops.pid_to_name(userdata, pid, out);
}

struct backend_ops dummy_backend_ops = {
	.destroy = destroy,
	.next_logcat_message = next_logcat_message,
	.pid_to_name = pid_to_name,
};
