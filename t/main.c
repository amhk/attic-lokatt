#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "liblokatt/error.h"

#include "test.h"

#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
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

	cprintf(ANSI_RESET, "[ RUN      ] %s/%s\n", t->category, t->name);
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
		cprintf(ANSI_RESET, "[       OK ] %s/%s\n",
			t->category, t->name);
	} else if (info.si_code == CLD_EXITED &&
		   info.si_status == EXIT_SKIPPED) {
		cprintf(ANSI_YELLOW, "[  SKIPPED ] %s/%s\n",
			t->category, t->name);
	} else {
		(*failed)++;
		cprintf(ANSI_RED, "[     FAIL ] %s/%s\n", t->category, t->name);
	}
}

static void run_all_tests(const char *category, const char *name,
			  int *passed, int *failed)
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		int run = 0;

		if (!category)
			run = 1;
		else if (!strcmp(category, t->category))
			run = !name || !strcmp(name, t->name);

		if (run)
			run_single_test(t, passed, failed);
	}
}

static void print_results(int passed, int failed)
{
	if (failed == 0) {
		cprintf(ANSI_GREEN, "[  PASSED  ] %d test%s\n", passed,
			passed == 1 ? "" : "s");
	} else {
		cprintf(ANSI_RED, "[   FAIL   ] %d test%s\n", failed,
			failed == 1 ? "" : "s");
	}
}

static void list_all_tests()
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		printf("%s/%s\n", t->category, t->name);
	}
}

int main(int argc, char **argv)
{
	int passed = 0, failed = 0;
	char *category = NULL, *name = NULL;

	if (argc > 1 && !strcmp(argv[1], "--list")) {
		list_all_tests();
		return 0;
	}

	if (argc > 1) {
		category = argv[1];
		name = strchr(category, '/');
		if (name) {
			*name++ = '\0';
			if (strlen(name) == 0)
				name = NULL;
		}
	}

	run_all_tests(category, name, &passed, &failed);
	print_results(passed, failed);

	return failed;
}
