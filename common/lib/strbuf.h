#ifndef LOKATT_STRBUF_H
#define LOKATT_STRBUF_H
#include <stdlib.h>

extern char strbuf_default_buffer[];

struct strbuf {
	size_t str_size;
	size_t alloc_size;
	char *buf;
};

#define STRBUF_INIT { 0, 0, strbuf_default_buffer }

void strbuf_init(struct strbuf *sb, size_t hint);
void strbuf_destroy(struct strbuf *sb);

void strbuf_grow(struct strbuf *sb, size_t extra);

void strbuf_add(struct strbuf *sb, const char *data, size_t size);
void strbuf_addstr(struct strbuf *sb, const char *str);
void strbuf_addch(struct strbuf *sb, const char ch);
void strbuf_addf(struct strbuf *sb, const char *fmt, ...) \
	     __attribute__((__format__(__printf__, 2, 3)));

#endif
