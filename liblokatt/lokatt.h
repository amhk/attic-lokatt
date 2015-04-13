#ifndef LIBLOKATT_LOKATT_H
#define LIBLOKATT_LOKATT_H
#include <stdint.h>

enum {
	LEVEL_VERBOSE = 2,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_ERROR,
	LEVEL_ASSERT,
};

#define MSG_MAX_PAYLOAD_SIZE (4 * 1024)

struct lokatt_message {
	int32_t pid;
	int32_t tid;
	int32_t sec;
	int32_t nsec;
	uint8_t level;
	const char *tag;
	const char *text;
	char payload[MSG_MAX_PAYLOAD_SIZE];
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
struct lokatt_device *lokatt_open_dummy_device(const char *path);
struct lokatt_device *lokatt_open_file(const char *path);
void lokatt_close_device(struct lokatt_device *);

struct lokatt_filter;
struct lokatt_filter *lokatt_create_filter(unsigned int event_bitmask,
					   const char *msg_spec);
void lokatt_destroy_filter(struct lokatt_filter *f);

/* returns non-zero on match */
int lokatt_filter_match(const struct lokatt_filter *f,
			const struct lokatt_event *event);

/*
 * Read the next event, as counted from event with id 'current_id', matching
 * the filter bitmask. Will block until a matching event becomes available.
 */
uint64_t lokatt_next_event(struct lokatt_device *dev,
			   uint64_t current_id,
			   const struct lokatt_filter *filter,
			   struct lokatt_event *out);

#endif
