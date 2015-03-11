#ifndef LOKATT_FILTER_H
#define LOKATT_FILTER_H

#include "strbuf.h"

struct filter;
struct lokatt_message;

struct token {
	enum {
		TOKEN_WHITESPACE = 1,

		TOKEN_KEY_PID,
		TOKEN_KEY_TID,
		TOKEN_KEY_SEC,
		TOKEN_KEY_NSEC,
		TOKEN_KEY_LEVEL,
		TOKEN_KEY_TAG,
		TOKEN_KEY_TEXT,
		TOKEN_KEY_PNAME,

		TOKEN_OP_EQ,     /*  ==  */
		TOKEN_OP_NE,     /*  !=  */
		TOKEN_OP_LT,     /*  <   */
		TOKEN_OP_LE,     /*  <=  */
		TOKEN_OP_GT,     /*  >   */
		TOKEN_OP_GE,     /*  >=  */
		TOKEN_OP_MATCH,  /*  =~  */
		TOKEN_OP_NMATCH, /*  !~  */

		TOKEN_OP_AND, /*  &&  */
		TOKEN_OP_OR,  /*  ||  */

		TOKEN_OP_LPAREN,  /*  (  */
		TOKEN_OP_RPAREN,  /*  )  */

		TOKEN_VALUE_INT,
		TOKEN_VALUE_STRING,
	} type;

	union {
		int value_int;
		struct strbuf value_string;
	};
};

struct filter *filter_create(const char *spec);
void filter_destroy(struct filter *f);
int filter_match(const struct filter *f, const struct lokatt_message *msg);

int filter_tokenize(const char *input, struct token **out, size_t *out_size);
void filter_free_tokens(struct token *tokens, size_t size);

#endif
