#include <stdlib.h>

#include "dict.h"

struct entry {
    uint32_t key;
    void *value;
    struct entry *next;
};

struct dict {
    uint32_t size;
    struct entry **table;
};

#define key_to_hash(map_p, key) \
    (key % map_p->size)

static struct entry *find_entry(const struct dict *map, uint32_t key)
{
    const uint32_t hash = key_to_hash(map, key);
    struct entry *entry = map->table[hash];
    while (entry) {
        if (entry->key == key)
            return entry;
        entry = entry->next;
    }
    return NULL;
}

struct dict *dict_create(uint32_t table_size)
{
    struct dict *map = malloc(sizeof(*map));
    map->size = table_size;
    map->table = calloc(table_size, sizeof(struct entry *));
    return map;
}

void dict_destroy(struct dict *map, void (*free_cb)(void *))
{
	size_t i;
	struct entry *entry;
	for (i = 0; i < map->size; i++) {
		entry = map->table[i];
		while (entry) {
			struct entry *tmp = entry;
			entry = entry->next;
			if (free_cb)
				free_cb(tmp->value);
			free(tmp);
		}
	}
	free(map->table);
	free(map);
}

int dict_put(struct dict *map, uint32_t key, void *value)
{
    const uint32_t hash = key_to_hash(map, key);
    struct entry *entry = find_entry(map, key);
    if (entry)
        return -1; /* duplicates not allowed */

    entry = calloc(1, sizeof(*entry));
    entry->key = key;
    entry->value = value;
    entry->next = map->table[hash];
    map->table[hash] = entry;

    return 0;
}

void *dict_get(const struct dict *map, uint32_t key)
{
    struct entry *entry = find_entry(map, key);
    return entry ? entry->value : NULL;
}
