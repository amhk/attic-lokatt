#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

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

int main()
{
	struct lokatt_message m;

	s = create_lokatt_session(1024);
	start_lokatt_session(s);
	c = create_lokatt_channel(s);

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);

	while (read_lokatt_channel(c, &m) != -1) {
		printf("%s\n", m.text);
	}

	destroy_lokatt_channel(c);
	destroy_lokatt_session(s);

	return EXIT_SUCCESS;
}
