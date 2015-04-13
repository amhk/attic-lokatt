#include <unistd.h>
#include <errno.h>

#include "adb.h"
#include "lokatt.h"

/*
 * This struct is taken from Android (system/core/include/log/logger.h); lokatt
 * silently discards the extra uint32_t present in logger_entry_v2 and
 * logger_entry_v3 structs so as to coalesce all three versions into one and
 * the same.
 */
struct logger_entry {
	uint16_t len;
	uint16_t __pad;
	int32_t pid;
	int32_t tid;
	int32_t sec;
	int32_t nsec;
	char msg[0];
} __attribute__((__packed__));

#define TEMP_FAILURE_RETRY(exp) \
	({ \
	 typeof (exp) _rc; \
	 do { \
	 _rc = (exp); \
	 } while (_rc == -1 && errno == EINTR); \
	 _rc; })

static ssize_t xread(int fd, void *buf, size_t count)
{
	ssize_t bytes_left = count;
	while (bytes_left > 0) {
		ssize_t r;

		r = TEMP_FAILURE_RETRY(read(fd, buf + count - bytes_left,
					    bytes_left));
		if (r < 0)
			return -1;
		bytes_left -= r;
	}
	return count;
}

ssize_t adb_read_lokatt_message(int fd, struct lokatt_message *out)
{
	struct logger_entry header;
	ssize_t r;
	uint32_t skip;

	r = xread(fd, &header, sizeof(header));
	if (r != sizeof(header))
		return -1;

	out->pid = header.pid;
	out->tid = header.tid;
	out->sec = header.sec;
	out->nsec = header.sec;

	/*
	 * Try to guess if we're reading v2 or v3 headers by looking at the
	 * padding, which is used to store the header size in v2 and v3. If we
	 * actually are reading any of the newer formats, skip the extra
	 * uint32_t present just before the payload.
	 */
	if (header.__pad != 0)
		xread(fd, (char *)&skip, sizeof(skip));

	r = xread(fd, out->payload, header.len);
	if (r != header.len)
		return -1;

	return 0;
}
