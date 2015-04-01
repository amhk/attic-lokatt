#include <stdlib.h>

#include "error.h"
#include "stack.h"

void stack_init(struct stack *stack, size_t item_size)
{
	stack->current_size = 0;
	stack->item_size = item_size;
	stack->max_size = 8;
	stack->data = calloc(stack->max_size, item_size);
}

void *stack_top(const struct stack *stack)
{
	void *p;

	if (stack->current_size == 0)
		die("empty stack");

	p = stack->data + (stack->current_size - 1) * stack->item_size;
	return p;
}

void *stack_push(struct stack *stack)
{
	if (stack->current_size == stack->max_size) {
		stack->max_size *= 2;
		stack->data = realloc(stack->data,
				      stack->max_size * stack->item_size);
	}

	stack->current_size++;
	return stack_top(stack);
}

void stack_pop(struct stack *stack)
{
	if (stack->current_size == 0)
		die("empty stack");

	stack->current_size--;
}

void stack_destroy(struct stack *stack)
{
	free(stack->data);
}
