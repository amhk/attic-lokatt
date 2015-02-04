#ifndef LOKATT_FILTER_LEXER_H
#define LOKATT_FILTER_LEXER_H

#include <stddef.h>

#define TOKEN_MAX_TEXT_SIZE 100

struct token {
	enum {
		TOKEN_KEY,
		TOKEN_INT,
		TOKEN_STRING,
	} type;
	union {
		int value; /* TOKEN_INT */
		char text[TOKEN_MAX_TEXT_SIZE]; /* TOKEN_KEY, TOKEN_STRING */
	};
};

/* FIXME: filter_tokenize should accept an additional parameter of type char **,
 * which if given is set to the error string, if any (to be freed by the caller) */
int filter_tokenize(const char *input,
		    struct token **tokens, size_t *token_count);

#endif
