#include <string.h>

#include "error.h"
#include "strbuf.h"
#include "test.h"

#define ASSERT_STRBUF_EQ(sb, str) \
	do { \
		ASSERT_EQ(strcmp((sb)->buf, (str)), 0); \
		ASSERT_EQ((sb)->str_size, strlen(str)); \
	} while (0)

TEST(grow)
{
	struct strbuf sb = STRBUF_INIT;
	const size_t old_size = sb.alloc_size;

	ASSERT_EQ(strcmp(sb.buf, ""), 0);

	strbuf_grow(&sb, 10);
	ASSERT_GE(sb.alloc_size - 10, old_size);

	ASSERT_EQ(strcmp(sb.buf, ""), 0);

	strbuf_destroy(&sb);
}

TEST(add_pre_allocated)
{
	struct strbuf sb;
	strbuf_init(&sb, 100);

	ASSERT_EQ(strcmp(sb.buf, ""), 0);

	strbuf_add(&sb, "foo", 3);
	ASSERT_STRBUF_EQ(&sb, "foo");

	strbuf_addstr(&sb, "bar");
	ASSERT_STRBUF_EQ(&sb, "foobar");

	strbuf_addf(&sb, " %d '%6s'", 1234, "abc");
	ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc'");

	strbuf_addch(&sb, 'x');
	ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc'x");

	strbuf_destroy(&sb);
}

TEST(add_alloc_automatically)
{
	struct strbuf sb = STRBUF_INIT;

	ASSERT_EQ(strcmp(sb.buf, ""), 0);

	strbuf_add(&sb, "foo", 3);
	ASSERT_STRBUF_EQ(&sb, "foo");

	strbuf_addstr(&sb, "bar");
	ASSERT_STRBUF_EQ(&sb, "foobar");

	strbuf_addf(&sb, " %d '%6s'", 1234, "abc");
	ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc'");

	strbuf_destroy(&sb);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
