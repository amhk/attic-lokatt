#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "liblokatt/lokatt.h"

int main()
{
	struct lokatt_device *dev;
	struct lokatt_event event;
	uint64_t id = 0;

	dev = lokatt_open_device("some-serial-number");

	for (int i = 0; i < 10000; i++) {
		lokatt_next_event(dev, id, EVENT_LOGCAT_MESSAGE, &event);
		id = event.id + 1;
		printf("id=%" PRIu64 " type=%d text='%s'\n",
		       event.id, event.type, event.msg.text);
	}

	lokatt_close_device(dev);

	return 0;
}
