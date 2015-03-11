#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "filter.h"
#include "strbuf.h"
#include "test.h"

TEST(lexer_single_valid_tokens)
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

	/* key: pname */
	retval = filter_tokenize("pname", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	ASSERT_EQ(tokens[0].type, TOKEN_KEY_PNAME);
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

TEST(lexer_multiple_valid_tokens)
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

TEST(lexer_valid_input)
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

TEST(lexer_invalid_input)
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

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
