#include "liblokatt/stack.h"

#include "test.h"

TEST(stack, push_and_pop)
{
	struct stack s;
	int *p;

	stack_init(&s, sizeof(int));

	p = stack_push(&s);
	*p = 1;
	ASSERT_EQ(*(int *)stack_top(&s), 1);

	p = stack_push(&s);
	*p = 2;
	ASSERT_EQ(*(int *)stack_top(&s), 2);

	stack_pop(&s);
	ASSERT_EQ(*(int *)stack_top(&s), 1);

	stack_destroy(&s);
}

TEST(stack, push_many_items)
{
	struct stack s;
	int *p, i;

	stack_init(&s, sizeof(int));

	for (i = 0; i < 2; i++) {
		p = stack_push(&s);
		*p = i;
		ASSERT_EQ(*(int *)stack_top(&s), i);
	}

	stack_destroy(&s);
}
