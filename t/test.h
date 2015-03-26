#ifndef TEST_TEST_H
#define TEST_TEST_H
#include "liblokatt/error.h"

/* inspired by the tests in the Wayland project */

struct test {
	const char *category;
	const char *name;
	void (*func)(void);
};

#define TEST(category, name) \
	static void lokatt_test_##category_##name(void); \
	const struct test test_##category_##name \
		__attribute__((section ("test_section"))) = \
	{ \
		#category, #name, lokatt_test_##category_##name \
	}; \
	static void lokatt_test_##category_##name()

#define EXIT_SKIPPED 127

#define ASSERT_EQ(expr, value) do { \
	if ((expr) != (value)) \
		die("assertion '%s' == '%s' failed", #expr, #value); \
} while (0)

#define ASSERT_NE(expr, value) do { \
	if ((expr) == (value)) \
		die("assertion '%s' != '%s' failed", #expr, #value); \
} while (0)

#define ASSERT_GT(expr, value) do { \
	if ((expr) <= (value)) \
		die("assertion '%s' > '%s' failed", #expr, #value); \
} while (0)

#define ASSERT_GE(expr, value) do { \
	if ((expr) < (value)) \
		die("assertion '%s' >= '%s' failed", #expr, #value); \
} while (0)

#define ASSERT_LT(expr, value) do { \
	if ((expr) >= (value)) \
		die("assertion '%s' < '%s' failed", #expr, #value); \
} while (0)

#define ASSERT_LE(expr, value) do { \
	if ((expr) > (value)) \
		die("assertion '%s' <= '%s' failed", #expr, #value); \
} while (0)

#endif
