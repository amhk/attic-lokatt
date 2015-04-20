#ifndef LOKATT_INDEX_H
#define LOKATT_INDEX_H

#include <stdint.h>

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

struct lokatt_event;

struct index {
	uint64_t current_size, max_size;
	struct lokatt_event **arena;
};

void index_init(struct index *idx);
void index_destroy(struct index *idx);

void index_append(struct index *idx, const struct lokatt_event *event);
const struct lokatt_event *index_get(struct index *idx, uint64_t id);

#endif
