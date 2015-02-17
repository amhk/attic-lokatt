#ifndef LOKATT_BRIDGE_H
#define LOKATT_BRIDGE_H

#include "lokatt.h"

typedef void *message_callback(const struct lokatt_message* m);

void *open_new_channel(struct lokatt_session *s,
							  message_callback *callback);

void close_async_channel(void *handle);

#endif
