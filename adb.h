#ifndef LOKATT_ADB_H
#define LOKATT_ADB_H

#include <stdint.h>
#include <stdlib.h>

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

struct adb_stream *create_adb_stream(int open_fd);
void destroy_adb_stream(struct adb_stream *stream);
int read_logcat(struct adb_stream *stream, struct logger_entry *header,
		char *payload, size_t size);

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

#endif

struct adb;
typedef void (*adb_cb)(const struct logger_entry *entry, const char *payload,
		       size_t payload_size, void *userdata);

struct adb *create_adb(adb_cb cb, void *userdata, const char *content_path);
void destroy_adb(struct adb *adb);
