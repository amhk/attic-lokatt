#ifndef LOKATT_FILTER_H
#define LOKATT_FILTER_H

struct filter;
struct lokatt_message;

struct filter *filter_create(const char *spec);
void filter_destroy(struct filter *f);
int filter_match(const struct filter *f, const struct lokatt_message *msg);

#endif
