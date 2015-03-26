#ifndef LIBLOKATT_LOKATT_H
#define LIBLOKATT_LOKATT_H
#include <stdint.h>

struct lokatt_message {
	uint32_t pid;
	char text[128];
};

#define EVENT_DEVICE_DISCONNECTED (1<<0)
#define EVENT_DEVICE_CONNECTED (1<<1)
#define EVENT_LOGCAT_MESSAGE (1<<2)
#define EVENT_ANY 0xffffffff

struct lokatt_event {
	int type;
	uint64_t id;
	union {
		/* no data for EVENT_DEVICE_{,DIS}CONNECTED */

		/* EVENT_LOGCAT_MESSAGE */
		struct lokatt_message msg;
	};
};

struct lokatt_device;
struct lokatt_device *lokatt_open_adb_device(const char *serialno);
struct lokatt_device *lokatt_open_dummy_device();
struct lokatt_device *lokatt_open_file(const char *path);
void lokatt_close_device(struct lokatt_device *);

/*
 * Read the next event, as counted from event with id 'current_id', matching
 * the filter bitmask. Will block until a matching event becomes available.
 */
uint64_t lokatt_next_event(struct lokatt_device *dev,
			   uint64_t current_id,
			   unsigned int event_filter_bitmask,
			   struct lokatt_event *out);

#endif
