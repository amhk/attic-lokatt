#ifndef LIBLOKATT_ERROR_H
#define LIBLOKATT_ERROR_H

#define die(fmt, ...) \
	do { \
		__die(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); \
	} while (0)

void __die(const char *file, unsigned int line, const char *func,
	   const char *fmt, ...) \
	     __attribute__((__noreturn__, __format__(__printf__, 4, 5)));

#endif
