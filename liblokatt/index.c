#include <stdlib.h>
#include <string.h>

#include "index.h"
#include "lokatt.h"

void index_init(struct index *idx)
{
	idx->current_size = 0;
	idx->max_size = 1024;
	idx->arena = calloc(idx->max_size, sizeof(struct lokatt_event *));
}

void index_destroy(struct index *idx)
{
	uint64_t i;
	for (i = 0; i < idx->current_size; i++)
		free(idx->arena[i]);
	free(idx->arena);
}

void index_append(struct index *idx, const struct lokatt_event *event)
{
	struct lokatt_event *copy = malloc(sizeof(*copy));
	memcpy(copy, event, sizeof(*copy));
	if (idx->current_size == idx->max_size) {
		idx->max_size += 1024;
		idx->arena = realloc(idx->arena, idx->max_size *
				     sizeof(struct lokatt_event *));
	}
	idx->arena[idx->current_size] = copy;
	copy->id = idx->current_size++;
}

const struct lokatt_event *index_get(struct index *idx, uint64_t id)
{
	if (id < idx->current_size)
		return idx->arena[id];
	return NULL;
}
