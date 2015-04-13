#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "liblokatt/lokatt.h"

int main(int argc, char **argv)
{
	struct lokatt_device *dev = NULL;
	struct lokatt_event event;
	struct lokatt_filter *filter;
	const char *filter_spec = NULL;
	uint64_t id = 0;

	if (argc > 1 && !strcmp(argv[1], "--dummy"))
		dev = lokatt_open_dummy_device();
	else if (argc > 1 && !strcmp(argv[1], "--file")) {
		if (argc > 2)
			dev = lokatt_open_file(argv[2]);
	} else {
		dev = lokatt_open_adb_device("some-serial-number");
		if (argc > 1)
			filter_spec = argv[1];
	}

	if (!dev) {
		fprintf(stdout, "failed to open device\n");
		return 1;
	}

	filter = lokatt_create_filter(EVENT_LOGCAT_MESSAGE, filter_spec);

	if (!filter) {
		fprintf(stderr, "failed to create filter\n");
		return 1;
	}

	for (;;) {
		lokatt_next_event(dev, id, filter, &event);
		id = event.id + 1;
		fprintf(stdout, "pid=%-4" PRIu32 " tid=%-4" PRIu32 " tag='%s' text='%s'\n",
			event.msg.pid, event.msg.tid, event.msg.tag,
			event.msg.text);
		fflush(stdout);
	}

	lokatt_destroy_filter(filter);
	lokatt_close_device(dev);

	return 0;
}
