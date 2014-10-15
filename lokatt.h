#ifndef LOKATT_LOKATT_H
#define LOKATT_LOKATT_H

struct lokatt_session;
struct lokatt_channel;

struct lokatt_message {
	char text[1024];
};

struct lokatt_session *create_lokatt_session(size_t buffer_size);
void start_lokatt_session(struct lokatt_session *s);
void stop_lokatt_session(struct lokatt_session *s);
void destroy_lokatt_session(struct lokatt_session *s);

struct lokatt_channel *create_lokatt_channel(struct lokatt_session *s);
int read_lokatt_channel(const struct lokatt_channel *c,
			 struct lokatt_message *m);
void destroy_lokatt_channel(struct lokatt_channel *c);

#endif
