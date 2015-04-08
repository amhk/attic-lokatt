#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "strbuf.h"

static void set_str_size(struct strbuf *sb, size_t new_size)
{
	assert(new_size < sb->alloc_size);
	sb->buf[new_size] = '\0';
	sb->str_size = new_size;
}

/*
 * Target for newly initialized strbuf buffers. Note: global variables will be
 * initialized to 0, so this is effectively the empty string.
 */
char strbuf_default_buffer[1];

void strbuf_init(struct strbuf *sb, size_t hint)
{
	sb->str_size = 0;
	sb->alloc_size = 0;
	sb->buf = strbuf_default_buffer;
	if (hint)
		strbuf_grow(sb, hint);
}

void strbuf_destroy(struct strbuf *sb)
{
	if (sb->alloc_size)
		free(sb->buf);
}

void strbuf_grow(struct strbuf *sb, size_t extra)
{
	if (sb->alloc_size) {
		size_t new_alloc_size = sb->alloc_size + extra + 1;
		sb->buf = realloc(sb->buf, new_alloc_size);
		if (!sb->buf)
			die("realloc new_alloc_size=%zd", new_alloc_size);
		sb->buf[sb->alloc_size + extra] = '\0';
		sb->alloc_size = new_alloc_size;
	} else {
		sb->buf = malloc(extra + 1);
		sb->alloc_size = extra + 1;
		set_str_size(sb, 0);
	}
}

void strbuf_add(struct strbuf *sb, const char *data, size_t size)
{
	strbuf_grow(sb, size);
	memcpy(sb->buf + sb->str_size, data, size);
	set_str_size(sb, sb->str_size + size);
}

void strbuf_addstr(struct strbuf *sb, const char *str)
{
	strbuf_add(sb, str, strlen(str));
}

void strbuf_addch(struct strbuf *sb, const char ch)
{
	strbuf_add(sb, &ch, 1);
}

void strbuf_vaddf(struct strbuf *sb, const char *fmt, va_list ap)
{
	va_list ap_cp;
	size_t required;
	size_t available = sb->alloc_size - sb->str_size - 1;

	va_copy(ap_cp, ap);

	required = vsnprintf(sb->buf + sb->str_size, available, fmt, ap_cp);
	if (required > available) {
		strbuf_grow(sb, required);
		available = sb->alloc_size - sb->str_size - 1;
		required = vsnprintf(sb->buf + sb->str_size, available,
				     fmt, ap);
	}
	set_str_size(sb, sb->str_size + required);

	va_end(ap_cp);
}

void strbuf_addf(struct strbuf *sb, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	strbuf_vaddf(sb, fmt, ap);
	va_end(ap);
}
