#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "bridge.h"

struct async_channel
{
	pthread_t t;
	struct lokatt_channel *c;
	message_callback *callback;

};

void *run(void *param)
{
	struct lokatt_message m;
	struct async_channel *ac = (struct async_channel *) param;
	while (!is_closed(ac->c) && read_lokatt_channel(ac->c, &m) != -1)
	{
		ac->callback(&m);
	}
	return NULL;
}

struct async_channel *create_async_channel(struct lokatt_channel *c,
		message_callback *callback)
{
	struct async_channel *ac;
	ac = malloc(sizeof(*ac));
	ac->c = c;
	ac->callback = callback;

	return ac;
}

void destroy_async_channel(struct async_channel *ac)
{
	destroy_lokatt_channel(ac->c);
	free(ac);
}

struct async_channel *open_new_channel(struct lokatt_session *s,
		message_callback *callback)
{
	struct lokatt_channel *c = create_lokatt_channel(s);
	struct async_channel *ac = create_async_channel(c, callback);
	if (pthread_create(&ac->t, NULL, run, ac))
	{
		//TODO: How should we handle this error?
		printf("Error creating thread\n");
	}
	return ac;
}

void close_async_channel(struct async_channel *ac)
{
	close_lokatt_channel(ac->c);
	pthread_join(ac->t, NULL);
	destroy_async_channel(ac);
}
