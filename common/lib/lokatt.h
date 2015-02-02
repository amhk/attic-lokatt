#ifndef LOKATT_LOKATT_H
#define LOKATT_LOKATT_H

#include <stdint.h>
#include <stddef.h>

struct lokatt_session;
struct lokatt_channel;

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
	char pname[128];
	char msg[MSG_MAX_PAYLOAD_SIZE];
};

struct lokatt_session *create_lokatt_session(size_t buffer_size);
void start_lokatt_session(struct lokatt_session *s);
void stop_lokatt_session(struct lokatt_session *s);
void destroy_lokatt_session(struct lokatt_session *s);

struct lokatt_channel *create_lokatt_channel(struct lokatt_session *s);
int read_lokatt_channel(const struct lokatt_channel *c,
			 struct lokatt_message *m);
void destroy_lokatt_channel(struct lokatt_channel *c);
void close_lokatt_channel(struct lokatt_channel *c);
int is_lokatt_channel_closed(struct lokatt_channel *c);

#endif
