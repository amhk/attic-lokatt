#ifndef LIBLOKATT_FILTER_H
#define LIBLOKATT_FILTER_H

#include "strbuf.h"

struct filter;
struct lokatt_message;

struct token {
	enum {
		TOKEN_TRUE = 10,
		TOKEN_FALSE,

		TOKEN_WHITESPACE = 20,

		TOKEN_VALUE_INT = 30,
		TOKEN_VALUE_STRING,

		TOKEN_KEY_PID = 40,
		TOKEN_KEY_TID,
		TOKEN_KEY_SEC,
		TOKEN_KEY_NSEC,
		TOKEN_KEY_LEVEL,
		TOKEN_KEY_TAG,
		TOKEN_KEY_TEXT,
		TOKEN_KEY_PNAME,

		TOKEN_OP_LPAREN = 50, /*  (  */
		TOKEN_OP_RPAREN,      /*  )  */

		TOKEN_OP_OR = 60,  /*  ||  */

		TOKEN_OP_AND = 70, /*  &&  */

		TOKEN_OP_EQ = 80, /*  ==  */
		TOKEN_OP_NE,      /*  !=  */
		TOKEN_OP_LT,      /*  <   */
		TOKEN_OP_LE,      /*  <=  */
		TOKEN_OP_GT,      /*  >   */
		TOKEN_OP_GE,      /*  >=  */
		TOKEN_OP_MATCH,   /*  =~  */
		TOKEN_OP_NMATCH,  /*  !~  */
	} type;

	union {
		int value_int;
		struct strbuf value_string;
	};
};

struct filter *filter_create(const char *spec);
void filter_destroy(struct filter *f);
int filter_match(const struct filter *f, const struct lokatt_message *msg);

/*
 * The following functions are exposed only for the sake of testability. Do not
 * call them directly except from test-filter.c.
 */
int filter_tokenize(const char *input, struct token **out, size_t *out_size);
void filter_free_tokens(struct token *tokens, size_t size);

int filter_tokens_as_rpn(const struct token *tokens, size_t token_count,
			 struct token ***out, size_t *out_size);

#endif
