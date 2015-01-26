#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>

#include "adb.h"
#include "error.h"
#include "strbuf.h"
#include "test.h"

/* read and write ends of a pipe */
#define R 0
#define W 1

static int is_device_connected()
{
	char buf[1024];
	pid_t pid;
	int fd[2];

	if (pipe(fd) < 0)
		return 0;

	memset(buf, 0, sizeof(buf));

	pid = fork();
	switch (pid) {
	case -1:
		return 0;
	case 0:
		close(fd[R]);
		dup2(fd[W], STDOUT_FILENO);
		execlp("adb", "adb", "get-state", NULL);
		exit(EXIT_FAILURE);
	default:
		close(fd[W]);
		waitpid(pid, NULL, 0);
		read(fd[R], buf, sizeof(buf));
		close(fd[R]);
		return !strcmp(buf, "device\n");
	}
}

static void cb_ignore(const struct logger_entry *header, const char *payload,
		      size_t size, void *userdata)
{
	(void)header;
	(void)payload;
	(void)size;
	(void)userdata;
}

static void cb_exit(const struct logger_entry *header, const char *payload,
		    size_t size, void *userdata)
{
	cb_ignore(header, payload, size, userdata);
	exit(EXIT_SUCCESS);
}

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

TEST(adb_callback)
{
	struct adb *adb;

	if (!is_device_connected())
		exit(EXIT_SKIPPED);

	/*
	 * If no callback arrives within 10 seconds, SIGALRM will be cause the
	 * process to exit with an error, which in turn will fail the test.
	 */
	alarm(10);
	adb = create_adb(cb_exit, NULL);
	usleep(2 * 1000 * 1000);
	destroy_adb(adb);
	exit(EXIT_FAILURE);
}

TEST(adb_shell)
{
	struct adb *adb;
	int retval;
	struct strbuf sb = STRBUF_INIT;

	if (!is_device_connected())
		exit(EXIT_SKIPPED);

	adb = create_adb(cb_ignore, NULL);

	retval = adb_shell(adb, "cat /proc/1/stat", &sb);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(strncmp(sb.buf, "1 (init) ", 9), 0);

	strbuf_destroy(&sb);
	strbuf_init(&sb, 128);

	retval = adb_shell(adb, "cat /proc/1/cmdline", &sb);
	ASSERT_EQ(retval, 0);

	destroy_adb(adb);
	strbuf_destroy(&sb);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
