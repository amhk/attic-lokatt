#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "backend.h"
#include "lokatt.h"

static const char *fortunes[32] = {
	"Everything that you know is wrong, but you can be straightened out.",
	"Sheriff Chameleotoptor sighed with an air of weary sadness.",
	"Every cloud engenders not a storm.",
	"Good day to let down old friends who need help.",
	"Change your thoughts and you change your world.",
	"For there are moments when one can neither think nor feel.",
	"You will be successful in love.",
	"You're not my type. For that matter, you're not even my species!!!",
	"Tomorrow, this will be part of the unchangeable past but fortunately,",
	"It is a wise father that knows his own child.",
	"You may get an opportunity for advancement today. Watch it!",
	"Today is the first day of the rest of your life.",
	"When angry, count four; when very angry, swear.",
	"You may be recognized soon. Hide.",
	"You have a truly strong individuality.",
	"You feel a whole lot more like you do now than you did when you...",
	"Never look up when dragons fly overhead.",
	"Many pages make a thick book, except for pocket Bibles which are...",
	"You are capable of planning your future.",
	"You're a card which will have to be dealt with.",
	"You will be reincarnated as a toad; and you will be much happier.",
	"Your lucky number is 3552664958674928. Watch for it everywhere.",
	"You are standing on my toes.",
	"When in doubt, tell the truth.",
	"You will gain money by an immoral action.",
	"Don't read any sky-writing for the next two weeks.",
	"You will obey or molten silver will be poured into your ears.",
	"Like an expensive sports car, fine-tuned and well-built, Portia...",
	"There is no distinctly native American criminal class except...",
	"A classic is something that everyone wants to have read",
	"You will be married within a year.",
	"You love your home and want it to be beautiful.",
};

static void init(void **userdata)
{
	*userdata = malloc(sizeof(uint32_t));
	srand(time(NULL));
}

static void destroy(void *userdata)
{
	free(userdata);
}

static int next_logcat_message(void *userdata, struct lokatt_message *out)
{
	uint32_t *prev, i;

	prev = userdata;
	do {
		i = rand() % 32;
	} while (i == *prev);
	*prev = i;

	strncpy(out->text, fortunes[i], 128);
	out->pid = i / 2 + 1;
	return 0;
}

static int pid_to_name(void *userdata, uint32_t pid, char out[128])
{
	(void)userdata;
	snprintf(out, 128, "dummy-%" PRIu32, pid);
	return 0;
}

struct backend_ops dummy_backend_ops = {
	.init = init,
	.destroy = destroy,
	.next_logcat_message = next_logcat_message,
	.pid_to_name = pid_to_name,
};
