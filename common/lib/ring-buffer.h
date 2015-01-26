#ifndef LOKATT_RING_BUFFER_H
#define LOKATT_RING_BUFFER_H

struct ring_buffer;
struct ring_buffer_iterator;

struct ring_buffer *create_ring_buffer(size_t size);
void destroy_ring_buffer(struct ring_buffer *rb);
int write_ring_buffer(struct ring_buffer *rb, const void *data, size_t size);

struct ring_buffer_iterator *
create_ring_buffer_iterator(const struct ring_buffer *rb);

/* Read next entry in ring-buffer. Return size of read entry, or -1 if the
 * iterator is at the (current) end of the buffer. */
ssize_t read_ring_buffer_iterator(struct ring_buffer_iterator *iter, void *out,
				  size_t max_size);

void destroy_ring_buffer_iterator(struct ring_buffer_iterator *iter);

#endif
