#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"
#include "test.h"

#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"

extern const struct test __start_test_section, __stop_test_section;

static void cprintf(const char *color, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	printf("%s", color);
	vprintf(fmt, ap);
	printf("%s", ANSI_RESET);
	va_end(ap);
}

static void run_single_test(const struct test *t, int *passed, int *failed)
{
	siginfo_t info;
	pid_t pid;

	cprintf(ANSI_RESET, "[ RUN      ] %s\n", t->name);
	pid = fork();
	switch (pid) {
		case -1:
			/* error */
			die("fork");
		case 0:
			/* child */
			t->func();
			exit(EXIT_SUCCESS);
	}
	/* parent */
	if (waitid(P_ALL, 0, &info, WEXITED) != 0)
		die("waitid");
	if (info.si_code == CLD_EXITED && info.si_status == EXIT_SUCCESS) {
		(*passed)++;
		cprintf(ANSI_RESET, "[       OK ] %s\n", t->name);
	} else {
		(*failed)++;
		cprintf(ANSI_RED, "[     FAIL ] %s\n", t->name);
	}
}

static void run_all_tests(int *passed, int *failed)
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		run_single_test(t, passed, failed);
	}
}

static void list_tests()
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		printf("%s\n", t->name);
	}
}

static const struct test *find_test(const char *name)
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		if (!strcmp(name, t->name))
			return t;
	}

	return NULL;
}

int test_main(int argc, char **argv)
{
	int passed = 0, failed = 0;
	if (argc > 1 && !strcmp(argv[1], "--list")) {
		list_tests();
		return 0;
	}

	if (argc > 1) {
		const struct test *t = find_test(argv[1]);
		if (t) {
			run_single_test(t, &passed, &failed);
		} else {
			fprintf(stderr, "%s: test not found\n", argv[1]);
			return 1;
		}
	} else {
		run_all_tests(&passed, &failed);
	}
	if (failed == 0) {
		cprintf(ANSI_GREEN, "[  PASSED  ] %d test%s\n", passed,
			passed == 1 ? "" : "s");
	} else {
		cprintf(ANSI_RED, "[   FAIL   ] %d test%s\n", failed,
			failed == 1 ? "" : "s");
	}

	return failed;
}
