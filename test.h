#ifndef LOKATT_TEST_H
#define LOKATT_TEST_H

/* inspired by the tests in the Wayland project */

struct test {
	const char *name;
	void (*func)(void);
};

#define TEST(name) \
	static void name(void); \
	const struct test test_##name \
		__attribute__((section ("test_section"))) = \
	{ \
		#name, name \
	}; \
	static void name()

int test_main(int argc, char **argv);

#endif
