#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#include "adb.h"
#include "error.h"
#include "test.h"

TEST(read_one_line_v1)
{
	int fd, status;
	struct logger_entry entry;
	struct adb_stream *stream;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/boot-v1.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	stream = create_adb_stream(fd);
	ASSERT_NE(stream, NULL);

	status = read_logcat(stream, &entry, buf, sizeof(buf));
	ASSERT_GE(status, 0);

	decode_logcat_payload(buf, &level, tag, text);
	ASSERT_EQ(level, 4);
	ASSERT_EQ(strcmp("tad", tag), 0);
	ASSERT_EQ(strcmp("Trim Area daemon starting.", text), 0);

	destroy_adb_stream(stream);
	close(fd);
}

TEST(read_one_line_v2)
{
	int fd, status;
	struct logger_entry entry;
	struct adb_stream *stream;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/boot-v2.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	stream = create_adb_stream(fd);
	ASSERT_NE(stream, NULL);

	status = read_logcat(stream, &entry, buf, sizeof(buf));
	ASSERT_GE(status, 0);

	decode_logcat_payload(buf, &level, tag, text);
	ASSERT_EQ(level, 4);
	ASSERT_EQ(strcmp("installd", tag), 0);
	ASSERT_EQ(strcmp("installd firing up", text), 0);

	destroy_adb_stream(stream);
	close(fd);
}

TEST(read_entire_file_v1)
{
	int fd, status;
	struct logger_entry entry;
	struct adb_stream *stream;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/shutdown-v1.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	stream = create_adb_stream(fd);
	ASSERT_NE(stream, NULL);

	status = read_logcat(stream, &entry, buf, sizeof(buf));
	while (status > 0) {
		decode_logcat_payload(buf, &level, tag, text);
		ASSERT_LT(level, 7);
		status = read_logcat(stream, &entry, buf, sizeof(buf));
	}
	ASSERT_EQ(status, 0);

	destroy_adb_stream(stream);
	close(fd);
}

TEST(read_entire_file_v2)
{
	int fd, status;
	struct logger_entry entry;
	struct adb_stream *stream;
	char buf[1024];
	uint8_t level;
	const char *tag, *text;

	fd = open("t/shutdown-v2.bin", O_RDONLY);
	ASSERT_GE(fd, 0);

	stream = create_adb_stream(fd);
	ASSERT_NE(stream, NULL);

	status = read_logcat(stream, &entry, buf, sizeof(buf));
	while (status > 0) {
		decode_logcat_payload(buf, &level, tag, text);
		ASSERT_LT(level, 7);
		status = read_logcat(stream, &entry, buf, sizeof(buf));
	}
	ASSERT_EQ(status, 0);

	destroy_adb_stream(stream);
	close(fd);
}

static void cb(const struct logger_entry *header, const char *payload,
	       size_t size, void *userdata)
{
	(void)header;
	(void)payload;
	(void)size;
	(void)userdata;
	exit(EXIT_SUCCESS);
}

TEST(adb_callback)
{
	struct adb *adb;

	/*
	 * If no callback arrives within 10 seconds, SIGALRM will be cause the
	 * process to exit with an error, which in turn will fail the test.
	 * Major caveat: this test case requires a connected device to pass.
	 */
	alarm(10);
	adb = create_adb(cb, NULL);
	usleep(2 * 1000 * 1000);
	destroy_adb(adb);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
