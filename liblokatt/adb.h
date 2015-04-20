#ifndef LIBLOKATT_ADB_H
#define LIBLOKATT_ADB_H
#include <sys/types.h>

struct lokatt_message;

ssize_t adb_read_lokatt_message(int fd, struct lokatt_message *out);

#endif
