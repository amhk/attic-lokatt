#ifndef LOKATT_INDEX_H
#define LOKATT_INDEX_H

#include <stdint.h>

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
