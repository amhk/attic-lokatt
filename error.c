#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void __die(const char *file, unsigned int line, const char *func,
	   const char *fmt, ...)
{
	va_list ap;
	int saved_errno = errno;

	va_start(ap, fmt);
	fprintf(stderr, "%s:%d:%s: ", file, line, func);
	vfprintf(stderr, fmt, ap);
	if (saved_errno)
		fprintf(stderr, ": %d %s", errno, strerror(errno));
	fprintf(stderr, "\n");
	va_end(ap);

	abort();
}
