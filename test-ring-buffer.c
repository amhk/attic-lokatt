#include <stdint.h>
#include <stdio.h>

#include "error.h"
#include "ring-buffer.h"

static void test_create_iterator_before_insert()
{
	struct ring_buffer *rb;
	struct ring_buffer_iterator *iter;
	uint32_t i;

	rb = create_ring_buffer(100 * sizeof(uint32_t));
	ASSERT_NE(rb, NULL);

	iter = create_ring_buffer_iterator(rb);
	ASSERT_NE(iter, NULL);

	i = 1234;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	i = 0;
	ASSERT_EQ(read_ring_buffer_iterator(iter, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 1234);

	destroy_ring_buffer_iterator(iter);
	destroy_ring_buffer(rb);
}

static void test_mixed_inserts_and_reads()
{
	struct ring_buffer *rb;
	struct ring_buffer_iterator *iter;
	uint32_t i;

	rb = create_ring_buffer(100 * sizeof(uint32_t));
	ASSERT_NE(rb, NULL);

	iter = create_ring_buffer_iterator(rb);
	ASSERT_NE(iter, NULL);

	i = 1;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	i = 0;
	ASSERT_EQ(read_ring_buffer_iterator(iter, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 1);

	i = 2;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	i = 0;
	ASSERT_EQ(read_ring_buffer_iterator(iter, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 2);

	destroy_ring_buffer_iterator(iter);
	destroy_ring_buffer(rb);

}

static void test_multiple_iterators_are_independent()
{
	struct ring_buffer *rb;
	struct ring_buffer_iterator *iter1, *iter2;
	uint32_t i;

	rb = create_ring_buffer(100 * sizeof(uint32_t));
	ASSERT_NE(rb, NULL);

	i = 1;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	iter1 = create_ring_buffer_iterator(rb);
	ASSERT_NE(iter1, NULL);

	i = 2;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	ASSERT_EQ(read_ring_buffer_iterator(iter1, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 1);

	ASSERT_EQ(read_ring_buffer_iterator(iter1, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 2);

	i = 3;
	ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);

	iter2 = create_ring_buffer_iterator(rb);
	ASSERT_NE(iter2, NULL);

	ASSERT_EQ(read_ring_buffer_iterator(iter2, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 1);

	ASSERT_EQ(read_ring_buffer_iterator(iter2, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 2);

	ASSERT_EQ(read_ring_buffer_iterator(iter2, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 3);

	ASSERT_EQ(read_ring_buffer_iterator(iter1, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_EQ(i, 3);

	destroy_ring_buffer_iterator(iter2);
	destroy_ring_buffer_iterator(iter1);
	destroy_ring_buffer(rb);
}

static void test_buffer_wraps_around()
{
	struct ring_buffer *rb;
	struct ring_buffer_iterator *iter;
	uint32_t i, j;

	rb = create_ring_buffer(200 * sizeof(uint32_t));
	ASSERT_NE(rb, NULL);

	iter = create_ring_buffer_iterator(rb);
	ASSERT_NE(iter, NULL);

	for (j = 1; j <= 201; j++) {
		i = 10 * j;
		ASSERT_GE(write_ring_buffer(rb, &i, sizeof(uint32_t)), 0);
	}

	ASSERT_EQ(read_ring_buffer_iterator(iter, &i, sizeof(uint32_t)),
		  sizeof(uint32_t));
	ASSERT_GT(i, 0);
	ASSERT_LE(i, 2010);

	while (i != 2010) {
		j = i;
		ASSERT_EQ(read_ring_buffer_iterator(iter, &i, sizeof(uint32_t)),
			  sizeof(uint32_t));
		ASSERT_EQ(j + 10, i);
	}

	destroy_ring_buffer_iterator(iter);
	destroy_ring_buffer(rb);
}

int main()
{
	test_create_iterator_before_insert();
	test_mixed_inserts_and_reads();
	test_multiple_iterators_are_independent();
	test_buffer_wraps_around();
	return 0;
}
