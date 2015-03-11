#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "lokatt.h"

struct filter {
};

struct filter *filter_create(const char *spec)
{
	(void)spec;
	struct filter *f = malloc(sizeof(*f));
	return f;
}

void filter_destroy(struct filter *f)
{
	free(f);
}

int filter_match(const struct filter *f, const struct lokatt_message *msg)
{
	(void)f;
	(void)msg;
	return 0;
}
