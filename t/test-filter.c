#include <stdlib.h>
#include <string.h>

#include "liblokatt/filter.h"
#include "liblokatt/lokatt.h"
#include "liblokatt/strbuf.h"

#include "test.h"

TEST(lexer, single_valid_tokens)
{
	struct token *tokens;
	size_t count;
	int retval;

	/* empty input */
	retval = filter_tokenize("", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 0);
	filter_free_tokens(tokens, count);

	/* only whitespace */
	retval = filter_tokenize(" \t\n ", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 0);
	filter_free_tokens(tokens, count);

	/* key: pid */
	retval = filter_tokenize("pid", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_PID);
	filter_free_tokens(tokens, count);

	/* key: tid */
	retval = filter_tokenize("tid", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_TID);
	filter_free_tokens(tokens, count);

	/* key: sec */
	retval = filter_tokenize("sec", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_SEC);
	filter_free_tokens(tokens, count);

	/* key: nsec */
	retval = filter_tokenize("nsec", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_NSEC);
	filter_free_tokens(tokens, count);

	/* key: level */
	retval = filter_tokenize("level", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_LEVEL);
	filter_free_tokens(tokens, count);

	/* key: tag */
	retval = filter_tokenize("tag", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_TAG);
	filter_free_tokens(tokens, count);

	/* key: text */
	retval = filter_tokenize("text", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_TEXT);
	filter_free_tokens(tokens, count);

	/* op: eq */
	retval = filter_tokenize("==", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_EQ);
	filter_free_tokens(tokens, count);

	/* op: ne */
	retval = filter_tokenize("!=", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_NE);
	filter_free_tokens(tokens, count);

	/* op: lt */
	retval = filter_tokenize("<", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_LT);
	filter_free_tokens(tokens, count);

	/* op: le */
	retval = filter_tokenize("<=", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_LE);
	filter_free_tokens(tokens, count);

	/* op: gt */
	retval = filter_tokenize(">", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_GT);
	filter_free_tokens(tokens, count);

	/* op: ge */
	retval = filter_tokenize(">=", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_GE);
	filter_free_tokens(tokens, count);

	/* op: match */
	retval = filter_tokenize("=~", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_MATCH);
	filter_free_tokens(tokens, count);

	/* op: nmatch */
	retval = filter_tokenize("!~", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_NMATCH);
	filter_free_tokens(tokens, count);

	/* op: and */
	retval = filter_tokenize("&&", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_AND);
	filter_free_tokens(tokens, count);

	/* op: or */
	retval = filter_tokenize("||", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_OR);
	filter_free_tokens(tokens, count);

	/* op: lparen */
	retval = filter_tokenize("(", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_LPAREN);
	filter_free_tokens(tokens, count);

	/* op: rparen */
	retval = filter_tokenize(")", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_OP_RPAREN);
	filter_free_tokens(tokens, count);

	/* value: int */
	retval = filter_tokenize("1234", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens[0].value_int, 1234);
	filter_free_tokens(tokens, count);

	/* value: string (no escaped chars) */
	retval = filter_tokenize("\"foo bar\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_VALUE_STRING);
	ASSERT_EQ(strcmp(tokens[0].value_string.buf, "foo bar"), 0);
	filter_free_tokens(tokens, count);

	/* value: string (with simple escaped char) */
	retval = filter_tokenize("\"\\\"\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_VALUE_STRING);
	ASSERT_EQ(strcmp(tokens[0].value_string.buf, "\""), 0);
	filter_free_tokens(tokens, count);

	/* value: string (with messy escaped chars) */
	retval = filter_tokenize("\"x \\\" y \\\\ z\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_VALUE_STRING);
	ASSERT_EQ(strcmp(tokens[0].value_string.buf, "x \" y \\ z"), 0);
	filter_free_tokens(tokens, count);
}

TEST(lexer, multiple_valid_tokens)
{
	struct token *tokens;
	size_t count;
	int retval;

	/* key and value separated by whitespace */
	retval = filter_tokenize("pid 123", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 2);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens[1].type, TOKEN_VALUE_INT);
	filter_free_tokens(tokens, count);

	/* key and value not separated by whitespace */
	retval = filter_tokenize("pid123", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 2);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens[1].type, TOKEN_VALUE_INT);
	filter_free_tokens(tokens, count);

	/* random whitespace */
	retval = filter_tokenize("  pid  \"BAR\"   123 ", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 3);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens[1].type, TOKEN_VALUE_STRING);
	ASSERT_EQ(tokens[2].type, TOKEN_VALUE_INT);
	filter_free_tokens(tokens, count);

	/* many, many, many tokens! */
	retval = filter_tokenize("1 2 3 4 5 6 7 8 9 10 11 12", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 12);
	filter_free_tokens(tokens, count);
}

TEST(lexer, valid_input)
{
	struct token *tokens;
	size_t count;
	int retval;

	/* valid query (with whitespace) */
	retval = filter_tokenize("tag == \"PackageManager\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 3);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_TAG);
	ASSERT_EQ(tokens[1].type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens[2].type, TOKEN_VALUE_STRING);
	filter_free_tokens(tokens, count);

	/* valid query (without whitespace) */
	retval = filter_tokenize("tag==\"PackageManager\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 3);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_TAG);
	ASSERT_EQ(tokens[1].type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens[2].type, TOKEN_VALUE_STRING);
	filter_free_tokens(tokens, count);
}

TEST(lexer, invalid_input)
{
	struct token *tokens;
	size_t count;
	int retval;

	/* bad key */
	retval = filter_tokenize("FOO", &tokens, &count);
	ASSERT_EQ(retval, 1);

	/* error halfway through input */
	retval = filter_tokenize("pid BAR", &tokens, &count);
	ASSERT_EQ(retval, 5);

	/* single = is not a valid op */
	retval = filter_tokenize("pid=1234", &tokens, &count);
	ASSERT_EQ(retval, 4);
}

TEST(filter, rpn_input_from_hardcoded_tokens)
{
	struct token tokens[3];
	struct token **tokens_rpn;
	size_t count_rpn;
	int retval;

	tokens[0].type = TOKEN_KEY_PID;

	tokens[1].type = TOKEN_OP_EQ;

	tokens[2].type = TOKEN_VALUE_INT;
	tokens[2].value_int = 1234;

	retval = filter_tokens_as_rpn(tokens, 3, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 3);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);

	free(tokens_rpn);
}

TEST(filter, rpn_input_from_lexer)
{
	struct token *tokens, **tokens_rpn;
	size_t count, count_rpn;
	int retval;

	/* key == int */
	retval = filter_tokenize("pid == 1234", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 3);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 3);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);

	filter_free_tokens(tokens, count);
	free(tokens_rpn);

	/* key == int && key == int */
	retval = filter_tokenize("pid == 1234 && tid != 1234", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 7);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 7);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens_rpn[3]->type, TOKEN_KEY_TID);
	ASSERT_EQ(tokens_rpn[4]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[5]->type, TOKEN_OP_NE);
	ASSERT_EQ(tokens_rpn[6]->type, TOKEN_OP_AND);

	filter_free_tokens(tokens, count);
	free(tokens_rpn);

	/* precedence: key == int && key == int || key == int */
	retval = filter_tokenize("pid == 1 && tid != 2 || sec < 3", &tokens,
				 &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 11);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 11);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens_rpn[3]->type, TOKEN_KEY_TID);
	ASSERT_EQ(tokens_rpn[4]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[5]->type, TOKEN_OP_NE);
	ASSERT_EQ(tokens_rpn[6]->type, TOKEN_OP_AND);
	ASSERT_EQ(tokens_rpn[7]->type, TOKEN_KEY_SEC);
	ASSERT_EQ(tokens_rpn[8]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[9]->type, TOKEN_OP_LT);
	ASSERT_EQ(tokens_rpn[10]->type, TOKEN_OP_OR);

	filter_free_tokens(tokens, count);
	free(tokens_rpn);

	/* precedence: key == int || key == int && key == int */
	retval = filter_tokenize("pid == 1 || tid != 2 && sec < 3", &tokens,
				 &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 11);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 11);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens_rpn[3]->type, TOKEN_KEY_TID);
	ASSERT_EQ(tokens_rpn[4]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[5]->type, TOKEN_OP_NE);
	ASSERT_EQ(tokens_rpn[6]->type, TOKEN_KEY_SEC);
	ASSERT_EQ(tokens_rpn[7]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[8]->type, TOKEN_OP_LT);
	ASSERT_EQ(tokens_rpn[9]->type, TOKEN_OP_AND);
	ASSERT_EQ(tokens_rpn[10]->type, TOKEN_OP_OR);

	filter_free_tokens(tokens, count);
	free(tokens_rpn);

	/* parenthesis: (key == int || key == int) && key == int */
	retval = filter_tokenize("(pid == 1 || tid != 2) && sec < 3", &tokens,
				 &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 13);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count_rpn, 11);
	ASSERT_EQ(tokens_rpn[0]->type, TOKEN_KEY_PID);
	ASSERT_EQ(tokens_rpn[1]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[2]->type, TOKEN_OP_EQ);
	ASSERT_EQ(tokens_rpn[3]->type, TOKEN_KEY_TID);
	ASSERT_EQ(tokens_rpn[4]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[5]->type, TOKEN_OP_NE);
	ASSERT_EQ(tokens_rpn[6]->type, TOKEN_OP_OR);
	ASSERT_EQ(tokens_rpn[7]->type, TOKEN_KEY_SEC);
	ASSERT_EQ(tokens_rpn[8]->type, TOKEN_VALUE_INT);
	ASSERT_EQ(tokens_rpn[9]->type, TOKEN_OP_LT);
	ASSERT_EQ(tokens_rpn[10]->type, TOKEN_OP_AND);

	filter_free_tokens(tokens, count);
	free(tokens_rpn);
}

TEST(filter, rpn_invalid_input)
{
	struct token *tokens, **tokens_rpn;
	size_t count, count_rpn;
	int retval;

	/* mismatched parenthesis */
	retval = filter_tokenize("(pid == 1234", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 4);

	retval = filter_tokens_as_rpn(tokens, count, &tokens_rpn, &count_rpn);
	ASSERT_NE(retval, 0);
	filter_free_tokens(tokens, count);
}

int oneshot(const char *spec, const struct lokatt_message *msg)
{
	int retval;
	struct filter *f;

	f = filter_create(spec);
	ASSERT_NE(f, NULL);
	retval = filter_match(f, msg);
	filter_destroy(f);

	return retval;
}

TEST(filter, valid_input)
{
	const struct lokatt_message msg = {
		.pid = 1,
		.tid = 2,
		.sec = 3,
		.nsec = 4,
		.level = LEVEL_WARNING,
		.tag = "PackageManagerService",
		.text = "This is the text.",
	};
	const char *str;

	/* integer values */
	ASSERT_EQ(oneshot("pid == 1", &msg), 0);
	ASSERT_EQ(oneshot("tid == 2", &msg), 0);
	ASSERT_EQ(oneshot("sec == 3", &msg), 0);
	ASSERT_EQ(oneshot("nsec == 4", &msg), 0);
	ASSERT_EQ(oneshot("level == 5", &msg), 0);

	/* string values */
	ASSERT_EQ(oneshot("tag == \"PackageManagerService\"", &msg), 0);
	ASSERT_EQ(oneshot("text == \"This is the text.\"", &msg), 0);

	/* integer operations */
	ASSERT_EQ(oneshot("pid == 1", &msg), 0);
	ASSERT_GT(oneshot("pid == 10", &msg), 0);

	ASSERT_EQ(oneshot("pid != 0", &msg), 0);
	ASSERT_GT(oneshot("pid != 1", &msg), 0);

	ASSERT_EQ(oneshot("pid < 2", &msg), 0);
	ASSERT_GT(oneshot("pid < 1", &msg), 0);

	ASSERT_EQ(oneshot("pid <= 2", &msg), 0);
	ASSERT_EQ(oneshot("pid <= 1", &msg), 0);
	ASSERT_GT(oneshot("pid <= 0", &msg), 0);

	ASSERT_EQ(oneshot("pid > 0", &msg), 0);
	ASSERT_GT(oneshot("pid > 1", &msg), 0);

	ASSERT_EQ(oneshot("pid >= 0", &msg), 0);
	ASSERT_EQ(oneshot("pid >= 1", &msg), 0);
	ASSERT_GT(oneshot("pid >= 2", &msg), 0);

	/* string operations */
	ASSERT_EQ(oneshot("tag == \"PackageManagerService\"", &msg), 0);
	ASSERT_GT(oneshot("tag == \"foobar\"", &msg), 0);

	ASSERT_EQ(oneshot("tag != \"foobar\"", &msg), 0);
	ASSERT_GT(oneshot("tag != \"PackageManagerService\"", &msg), 0);

	/* TODO: add tests for =~ and !~ */

	/* logical operations */
	str = "pid == 1 && tag == \"PackageManagerService\"";
	ASSERT_EQ(oneshot(str, &msg), 0);
	str = "pid == 1 && tag == \"x\"";
	ASSERT_NE(oneshot(str, &msg), 0);
	str = "pid == 0 && tag == \"PackageManagerService\"";
	ASSERT_NE(oneshot(str, &msg), 0);

	str = "pid == 1 || tag == \"x\"";
	ASSERT_EQ(oneshot(str, &msg), 0);
	str = "pid == 0 || tag == \"PackageManagerService\"";
	ASSERT_EQ(oneshot(str, &msg), 0);
	str = "pid == 0 || tag == \"x\"";
	ASSERT_NE(oneshot(str, &msg), 0);
}

TEST(filter, invalid_input)
{
	struct filter *f;

	f = filter_create("tag tag tag tag");
	ASSERT_EQ(f, NULL);

	f = filter_create("1234 == pid");
	ASSERT_EQ(f, NULL);

	f = filter_create("(pid == 1 || tid != 2 && sec < 3");
	ASSERT_EQ(f, NULL);
}
