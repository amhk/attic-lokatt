#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "liblokatt/lokatt.h"

int main(int argc, char **argv)
{
	struct lokatt_device *dev;
	struct lokatt_event event;
	uint64_t id = 0;

	if (argc > 1 && !strcmp(argv[1], "--dummy"))
		dev = lokatt_open_dummy_device();
	else
		dev = lokatt_open_adb_device("some-serial-number");

	for (int i = 0; i < 10000; i++) {
		lokatt_next_event(dev, id, EVENT_LOGCAT_MESSAGE, &event);
		id = event.id + 1;
		fprintf(stdout, "id=%" PRIu64 " type=%d text='%s'\n",
			event.id, event.type, event.msg.text);
		fflush(stdout);
	}

	lokatt_close_device(dev);

	return 0;
}
