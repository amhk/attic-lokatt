#ifndef LIBLOKATT_STACK_H
#define LIBLOKATT_STACK_H
#include <stddef.h>

struct stack {
	void *data;
	size_t item_size;
	size_t current_size;
	size_t max_size;
};

void stack_init(struct stack *stack, size_t item_size);
void *stack_top(const struct stack *stack);
void *stack_push(struct stack *stack);
void stack_pop(struct stack *stack);
void stack_destroy(struct stack *stack);

#endif
