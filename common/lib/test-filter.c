#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "filter-lexer.h"
#include "test.h"

TEST(lexer_single_valid_tokens)
{
	struct token *t;
	struct token *tokens;
	size_t count;
	int retval;

	/* empty input */
	retval = filter_tokenize("", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 0);
	free(tokens);

	/* only whitespace */
	retval = filter_tokenize(" \t\n ", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 0);
	free(tokens);

	/* key */
	retval = filter_tokenize("foo", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	t = &tokens[0];
	ASSERT_EQ(t->type, TOKEN_KEY);
	ASSERT_EQ(strcmp(t->text, "foo"), 0);
	free(tokens);

	/* int */
	retval = filter_tokenize("1234", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	t = &tokens[0];
	ASSERT_EQ(t->type, TOKEN_INT);
	ASSERT_EQ(t->value, 1234);
	free(tokens);

	/* string (no escaped chars) */
	retval = filter_tokenize("\"foo bar\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	t = &tokens[0];
	ASSERT_EQ(t->type, TOKEN_STRING);
	ASSERT_EQ(strcmp(t->text, "foo bar"), 0);
	free(tokens);

	/* string (with simple escaped char) */
	retval = filter_tokenize("\"\\\"\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	t = &tokens[0];
	ASSERT_EQ(t->type, TOKEN_STRING);
	ASSERT_EQ(strcmp(t->text, "\""), 0);
	free(tokens);

	/* string (with messy escaped chars) */
	retval = filter_tokenize("\"x \\\" y \\\\ z\"", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 1);
	t = &tokens[0];
	ASSERT_EQ(t->type, TOKEN_STRING);
	ASSERT_EQ(strcmp(t->text, "x \" y \\ z"), 0);
	free(tokens);
}

TEST(lexer_multiple_valid_tokens)
{
	struct token *tokens;
	size_t count;
	int retval;

	/* key and value separated by whitespace */
	retval = filter_tokenize("foo 123", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 2);
	free(tokens);

	/* key and value not separated by whitespace */
	retval = filter_tokenize("foo123", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 2);
	free(tokens);

	/* random whitespace */
	retval = filter_tokenize("  foo  \"bar\"   123 ", &tokens, &count);
	ASSERT_EQ(retval, 0);
	ASSERT_EQ(count, 3);
	free(tokens);
}

int main(int argc, char **argv)
{
	return test_main(argc, argv);
}
