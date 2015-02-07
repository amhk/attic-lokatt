#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "error.h"
#include "test.h"

TEST(single_insert)
{
    struct dict *map;
    const char *value;

    map = dict_create(11);
    ASSERT_NE(map, NULL);

    value = dict_get(map, 8);
    ASSERT_EQ(value, NULL);

    ASSERT_EQ(dict_put(map, 8, "foobar"), 0);

    value = dict_get(map, 8);
    ASSERT_NE(value, NULL);
    ASSERT_EQ(strcmp(value, "foobar"), 0);

    dict_destroy(map, NULL);
}

TEST(no_duplicates_allowed)
{
    struct dict *map;

    map = dict_create(11);
    ASSERT_NE(map, NULL);

    ASSERT_EQ(dict_put(map, 8, "foobar"), 0);
    ASSERT_NE(dict_put(map, 8, "foobar"), 0);
    ASSERT_NE(dict_put(map, 8, "baz"), 0);

    dict_destroy(map, NULL);
}

TEST(chained_entries)
{
#define N 10
    struct dict *map;
    int i;
    char *values[N] = {
        "value a",
        "value b",
        "value c",
        "value d",
        "value e",
        "value f",
        "value g",
        "value h",
        "value i",
        "value j",
    };

    map = dict_create(3);
    ASSERT_NE(map, NULL);

    for (i = 0; i < N; i++) {
        ASSERT_EQ(dict_put(map, i, values[i]), 0);
    }

    for (i = 0; i < N; i++) {
        const char *value;
        value = dict_get(map, i);
        ASSERT_NE(value, NULL);
        ASSERT_EQ(strcmp(value, values[i]), 0);
    }

    dict_destroy(map, NULL);
#undef N
}

static int count;

static void count_cb(void *p)
{
	count++;
	free(p);
}

TEST(free_callback)
{
    struct dict *map;

    map = dict_create(11);

    ASSERT_EQ(dict_put(map, 3, strdup("foo")), 0);
    ASSERT_EQ(dict_put(map, 7, strdup("bar")), 0);

    count = 0;
    dict_destroy(map, count_cb);
    ASSERT_EQ(count, 2);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
