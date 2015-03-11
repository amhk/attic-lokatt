#include <stdlib.h>
#include <string.h>

#include "filter-lexer.h"
#include "filter.h"
#include "lokatt.h"
#include "stack.h"

struct filter {
};

struct filter *filter_create(const char *spec)
{
	(void)spec;
	struct filter *f = malloc(sizeof(*f));
	return f;
}

void filter_destroy(struct filter *f)
{
	free(f);
}

int filter_match(const struct filter *f, const struct lokatt_message *msg)
{
	(void)f;
	(void)msg;
	return 0;
}

/* Replace any pair of chars '\x' with 'x'. */
static void unescape(char *str)
{
	char *end = str + strlen(str);
	while (str < end) {
		if (*str == '\\') {
			char *p = str;
			while (p < end) {
				*p = *(p + 1);
				p++;
			}
			end--;
		}
		str++;
	}
}

int filter_tokenize(const char *input, struct token **out, size_t *out_size)
{
	yyscan_t scanner;
	YY_BUFFER_STATE handle;
	int pos = 1;
	struct stack stack;

	*out_size = 0;
	stack_init(&stack, sizeof(struct token));
	yylex_init(&scanner);
	handle = yy_scan_string(input, scanner);

	for (;;) {
		int status;

		status = yylex(scanner);

		if (status == 0)
			goto success;

		if (status < 0)
			goto failure;

		if (status == TOKEN_WHITESPACE) {
			/* do nothing */
		} else if (status == TOKEN_VALUE_INT) {
			struct token *t;

			t = stack_push(&stack);
			t->type = TOKEN_VALUE_INT;
			t->value_int = atoi(yyget_text(scanner));
		} else if (status == TOKEN_VALUE_STRING) {
			struct token *t;
			char *text;

			text = strdup(yyget_text(scanner));
			/* remove trailing quote ... */
			text[strlen(text) - 1] = '\0';
			unescape(text + 1);

			t = stack_push(&stack);
			t->type = TOKEN_VALUE_STRING;
			strbuf_init(&t->value_string, strlen(text));
			/* ... and don't add leading quote */
			strbuf_addstr(&t->value_string, text + 1);
			free(text);
		} else {
			struct token *t;

			t = stack_push(&stack);
			t->type = status;
		}

		pos += yyget_leng(scanner);
		if (status != TOKEN_WHITESPACE)
			*out_size += 1;
	}

success:
	yy_delete_buffer(handle, scanner);
	yylex_destroy(scanner);
	*out = (struct token *)stack.data;
	return 0;

failure:
	yy_delete_buffer(handle, scanner);
	yylex_destroy(scanner);
	free(stack.data);
	*out = NULL;
	*out_size = 0;
	return pos;
}

void filter_free_tokens(struct token *tokens, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++) {
		struct token *t = &tokens[i];

		if (t->type == TOKEN_VALUE_STRING)
			strbuf_destroy(&t->value_string);
	}
	free(tokens);
}
