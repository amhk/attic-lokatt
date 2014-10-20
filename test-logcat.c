#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#include "error.h"
#include "logcat.h"
#include "test.h"

TEST(read_one_line)
{
	int fd, status;
	struct logger_entry entry;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/boot.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	status = read_logcat(fd, &entry, buf, sizeof(buf));
	ASSERT_GE(status, 0);
	ASSERT_EQ(entry.__pad, 0);

	decode_logcat_payload(buf, &level, tag, text);
	ASSERT_EQ(level, 4);
	ASSERT_EQ(strcmp("tad", tag), 0);
	ASSERT_EQ(strcmp("Trim Area daemon starting.", text), 0);

	close(fd);
}

TEST(read_entire_file)
{
	int fd, status;
	struct logger_entry entry;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/shutdown.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	status = read_logcat(fd, &entry, buf, sizeof(buf));
	while (status > 0) {
		decode_logcat_payload(buf, &level, tag, text);
		ASSERT_EQ(entry.__pad, 0);
		ASSERT_LT(level, 7);
		status = read_logcat(fd, &entry, buf, sizeof(buf));
	}
	ASSERT_EQ(status, 0);

	close(fd);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
