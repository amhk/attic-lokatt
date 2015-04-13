#ifndef LOKATT_BACKEND_H
#define LOKATT_BACKEND_H
#include <stdint.h>

struct lokatt_message;

struct backend_ops {
	void (*destroy)(void *userdata);
	int (*next_logcat_message)(void *userdata, struct lokatt_message *out);
	int (*pid_to_name)(void *userdata, uint32_t pid, char out[128]);
};

extern void *create_dummy_backend(const char *path);
extern struct backend_ops dummy_backend_ops;

extern void *create_adb_backend(const char *serialno);
extern struct backend_ops adb_backend_ops;

extern void *create_file_backend(const char *path);
extern struct backend_ops file_backend_ops;

#endif
