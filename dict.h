#ifndef LOKATT_DICT_H
#define LOKATT_DICT_H
#include <stdint.h>

struct dict;

struct dict *dict_create(uint32_t table_size);
void dict_destroy(struct dict *, void (*free_cb)(void *));

int dict_put(struct dict *, uint32_t key, void *value);
void *dict_get(const struct dict *, uint32_t key);

#endif
