#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lokatt.h"

struct lokatt_session *s;
struct lokatt_channel *c;

static void on_signal(int signum)
{
	(void)signum;
	stop_lokatt_session(s);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
}

static int is_filtered_out(const struct lokatt_message *m)
{
	/*
	 * Silly audit_log, acting all grown-up by including ANSI escape
	 * characters in its text (which will mess up the terminal).
	 */
	return !strcmp(m->tag, "audit_log") ? 1 : 0;
}

static void print_message(const struct lokatt_message *m)
{
	printf("%-10s %8d %8d %12d %12d %d %-32s %s\n",
	       m->pname, m->pid, m->tid, m->sec, m->nsec,
	       m->level, m->tag, m->text);
}

int main()
{
	struct lokatt_message m;

	s = create_lokatt_session(1024 * 1024);
	start_lokatt_session(s);
	c = create_lokatt_channel(s);

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);

	while (read_lokatt_channel(c, &m) != -1) {
		if (!is_filtered_out(&m))
			print_message(&m);
	}

	destroy_lokatt_channel(c);
	destroy_lokatt_session(s);

	return EXIT_SUCCESS;
}
