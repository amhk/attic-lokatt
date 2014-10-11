#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ring-buffer.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct entry {
	enum {
		TYPE_INTERNAL,
		TYPE_USER,
	} type;
	struct entry *next;
	size_t size; /* size of payload only */
	uint32_t seq;
	char data[0];
};

struct ring_buffer {
	size_t size; /* size of buffer only */
	struct entry *head; /* points to newest entry */
	struct entry *tail; /* points to oldest entry */
	char buf[0];
};

struct ring_buffer_iterator {
	const struct ring_buffer* ring_buffer;
	const struct entry *current;
	uint32_t seq;
};

struct ring_buffer *create_ring_buffer(size_t size)
{
	struct ring_buffer *rb;
	struct entry *entry;

	rb = malloc(sizeof(*rb) + size);
	rb->size = size;
	entry = (struct entry *)rb->buf;
	entry->type = TYPE_INTERNAL;
	entry->size = 0;
	entry->next = NULL;
	entry->seq = 1;
	rb->head = entry;
	rb->tail = entry;
	return rb;
}

void destroy_ring_buffer(struct ring_buffer *rb)
{
	free(rb);
}

int write_ring_buffer(struct ring_buffer *rb, void *data, size_t size)
{
	static const char S = sizeof(struct entry);
	const size_t required_space = S + size;
	if (required_space > rb->size)
		return -1;
	const size_t space_left =
		rb->size - ((char *)rb->head - rb->buf) - S - rb->head->size;
	if (required_space <= space_left) {
		struct entry *entry =
			(struct entry *)((char *)rb->head + S + rb->head->size);
		assert((char *)entry < rb->buf + rb->size);

		const char *end_of_entry = (char *)entry + S + size;
		while (rb->head < rb->tail && (char *)rb->tail <= end_of_entry)
			rb->tail = rb->tail->next;

		entry->seq = rb->head->seq + 1;
		entry->type = TYPE_USER;
		entry->size = size;
		entry->next = NULL;
		memcpy(entry->data, data, size);

		rb->head->next = entry;
		rb->head = entry;

	} else {
		struct entry *entry = (struct entry *)rb->buf;
		const char *end_of_entry = (char *)entry + S + size;

		while ((char *)rb->tail <= end_of_entry && rb->tail->next)  {
			rb->tail = rb->tail->next;
		}
		if ((char *)rb->tail < end_of_entry)
			rb->tail = entry;

		entry->seq = rb->head->seq + 1;
		entry->type = TYPE_USER;
		entry->size = size;
		entry->next = NULL;
		memcpy(entry->data, data, size);

		if ((char *)rb->head > end_of_entry) {
			rb->head->next = entry;
			rb->head = entry;
		} else {
			rb->head = entry;
		}
	}
	return 0;
}

struct ring_buffer_iterator *
create_ring_buffer_iterator(const struct ring_buffer *rb)
{
	struct ring_buffer_iterator *iter;

	iter = malloc(sizeof(*iter));
	iter->ring_buffer = rb;
	iter->seq = 0;
	iter->current = rb->tail;
	return iter;
}

ssize_t read_ring_buffer_iterator(struct ring_buffer_iterator *iter, void *out,
				  size_t max_size)
{
	int type = TYPE_INTERNAL;
	ssize_t size;

	if (iter->seq < iter->ring_buffer->tail->seq)
		iter->seq = iter->ring_buffer->tail->seq;

	while (type != TYPE_USER) {
		if (iter->current->next == NULL)
			return -1;
		iter->current = iter->current->next;

		size = MIN(max_size, iter->current->size);
		type = iter->current->type;
		memcpy(out, iter->current->data, size);
		iter->seq = iter->current->seq;
	}
	return size;
}

void destroy_ring_buffer_iterator(struct ring_buffer_iterator *iter)
{
	free(iter);
}
