#ifndef LOKATT_TEST_H
#define LOKATT_TEST_H

/* inspired by the tests in the Wayland project */

struct test {
	const char *name;
	void (*func)(void);
};

#define TEST(name) \
	static void lokatt_test_##name(void); \
	const struct test test_##name \
		__attribute__((section ("test_section"))) = \
	{ \
		#name, lokatt_test_##name \
	}; \
	static void lokatt_test_##name()

int test_main(int argc, char **argv);

#define EXIT_SKIPPED 127

#endif
