#ifndef LOKATT_LOGCAT_H
#define LOKATT_LOGCAT_H

#include <stdint.h>
#include <stdlib.h>

/*
 * This struct taken from Android (system/core/include/log/logger.h).
 * Currently, lokatt doesn't support logger_entry_v2.
 */
struct logger_entry {
	uint16_t len;
	uint16_t __pad;
	int32_t pid;
	int32_t tid;
	int32_t sec;
	int32_t nsec;
	char msg[0];
};

/* Read logcat entry and payload from opened file descriptor. */
int read_logcat(int fd, struct logger_entry *header, char *payload,
		size_t size);

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
		}\
	} while (0)

#endif
