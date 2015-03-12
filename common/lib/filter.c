#include <stdlib.h>
#include <string.h>

#include "filter-lexer.h"
#include "filter.h"
#include "lokatt.h"
#include "stack.h"

struct filter {
	struct token *tokens;
	size_t token_count;

	struct token **rpn;
	size_t rpn_count;
};

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

#define is_key(type) \
	((type) == TOKEN_KEY_PID || (type) == TOKEN_KEY_TID || \
	 (type) == TOKEN_KEY_SEC || (type) == TOKEN_KEY_NSEC || \
	 (type) == TOKEN_KEY_LEVEL || (type) == TOKEN_KEY_TAG || \
	 (type) == TOKEN_KEY_TEXT || (type) == TOKEN_KEY_PNAME)

#define is_value(type) \
	((type) == TOKEN_VALUE_INT || (type) == TOKEN_VALUE_STRING)

#define is_logical_operator(type) \
	((type) == TOKEN_OP_AND || (type) == TOKEN_OP_OR)

#define is_operator(type) \
	((type) == TOKEN_OP_EQ || (type) == TOKEN_OP_NE || \
	 (type) == TOKEN_OP_LT || (type) == TOKEN_OP_LE || \
	 (type) == TOKEN_OP_GT || (type) == TOKEN_OP_GE || \
	 (type) == TOKEN_OP_MATCH || (type) == TOKEN_OP_NMATCH || \
	 is_logical_operator(type))

#define precedence_is_le(type1, type2) \
	((type1) / 10 <= (type2) / 10)

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

static int shunting_yard(const struct token *tokens, size_t token_count,
			 struct stack *out)
{
	size_t i;
	struct stack stack;
	int retval = 0;

	stack_init(&stack, sizeof(struct token *));

	for (i = 0; i < token_count; i++) {
		const struct token *t = &tokens[i];

		if (is_operator(t->type)) {
			const struct token **p;

			while (stack.current_size > 0) {
				const struct token **q;

				q = stack_top(&stack);
				if (precedence_is_le(t->type, (*q)->type)) {
					stack_pop(&stack);
					p = stack_push(out);
					*p = *q;
				} else {
					break;
				}
			}

			p = stack_push(&stack);
			*p = t;
		} else if (t->type == TOKEN_OP_LPAREN) {
			const struct token **p;

			p = stack_push(&stack);
			*p = t;
		} else if (t->type == TOKEN_OP_RPAREN) {
			int done = 0;
			while (stack.current_size > 0) {
				const struct token **p;
				const struct token **q;

				q = stack_top(&stack);
				stack_pop(&stack);
				if ((*q)->type == TOKEN_OP_LPAREN) {
					done = 1;
					break;
				}
				p = stack_push(out);
				*p = *q;
			}
			if (!done) {
				retval = -1;
				goto bail;
			}
		} else {
			const struct token **p;

			p = stack_push(out);
			*p = t;
		}
	}

	while (stack.current_size > 0) {
		const struct token **p;
		const struct token **q;

		p = stack_top(&stack);
		if ((*p)->type == TOKEN_OP_LPAREN) {
			retval = -1;
			goto bail;
		}
		stack_pop(&stack);
		q = stack_push(out);
		*q = *p;
	}

bail:
	stack_destroy(&stack);
	return retval;
}

int filter_tokens_as_rpn(const struct token *tokens, size_t token_count,
			 struct token ***out, size_t *out_size)
{
	int retval = 0;
	struct stack rpn;

	stack_init(&rpn, sizeof(struct token *));
	retval = shunting_yard(tokens, token_count, &rpn);

	if (retval == 0) {
		*out = (struct token **)rpn.data;
		*out_size = rpn.current_size;
	} else {
		stack_destroy(&rpn);
		*out = NULL;
		*out_size = 0;
	}
	return retval;
}

struct filter *filter_create(const char *spec)
{
	static struct lokatt_message dummy_msg = {
		.pid = 0,
		.tid = 0,
		.sec = 0,
		.nsec = 0,
		.level = LEVEL_ASSERT,
		.tag = "",
		.text = "",
		.pname = "",
	};
	struct filter *f = calloc(1, sizeof(*f));

	if (filter_tokenize(spec, &f->tokens, &f->token_count))
		goto bail;

	if (filter_tokens_as_rpn(f->tokens, f->token_count, &f->rpn,
				 &f->rpn_count))
		goto bail;

	/* check for syntax error, eg "tag == tag" */
	if (filter_match(f, &dummy_msg) == -1)
		goto bail;

	return f;
bail:
	filter_destroy(f);
	return NULL;
}

void filter_destroy(struct filter *f)
{
	free(f->rpn);
	filter_free_tokens(f->tokens, f->token_count);
	free(f);
}

static int get_int(const struct token *t, const struct lokatt_message *msg,
		   int32_t *out)
{
	switch (t->type) {
	case TOKEN_KEY_PID:
		*out = msg->pid;
		return 0;
	case TOKEN_KEY_TID:
		*out = msg->tid;
		return 0;
	case TOKEN_KEY_SEC:
		*out = msg->sec;
		return 0;
	case TOKEN_KEY_NSEC:
		*out = msg->nsec;
		return 0;
	case TOKEN_KEY_LEVEL:
		*out = msg->level;
		return 0;
	default:
		return -1;
	}
}

static int get_string(const struct token *t, const struct lokatt_message *msg,
		      const char **out)
{
	switch (t->type) {
	case TOKEN_KEY_TAG:
		*out = msg->tag;
		return 0;
	case TOKEN_KEY_TEXT:
		*out = msg->text;
		return 0;
	case TOKEN_KEY_PNAME:
		*out = msg->pname;
		return 0;
	default:
		return -1;
	}
}

static int evaluate_eq(const struct token *left,
		       const struct token *right,
		       const struct lokatt_message *msg,
		       int *out)
{
	if (!is_key(left->type))
		return -1;
	if (!is_value(right->type))
		return -1;

	if (right->type == TOKEN_VALUE_INT) {
		int32_t value;
		if (get_int(left, msg, &value))
			return -1;
		*out = value == right->value_int;
		return 0;
	} else {
		const char *str;
		if (get_string(left, msg, &str))
			return -1;
		*out = !strcmp(str, right->value_string.buf);
		return 0;
	}
}

static int evaluate_lt(const struct token *left,
		       const struct token *right,
		       const struct lokatt_message *msg,
		       int *out)
{
	int32_t value;

	if (!is_key(left->type))
		return -1;
	if (right->type != TOKEN_VALUE_INT)
		return -1;
	if (get_int(left, msg, &value))
		return -1;
	*out = value < right->value_int;
	return 0;
}

/*
 * Return value:
 *   - '0': match
 *   - '> 0': no match
 *   - '< 0': internal error
 */
int filter_match(const struct filter *f, const struct lokatt_message *msg)
{
	static const struct token TRUE = {
		.type = TOKEN_TRUE,
	};
	static const struct token FALSE = {
		.type = TOKEN_FALSE,
	};

	struct stack stack;
	const struct token *t;
	const struct token **p;
	int retval = -1;

	stack_init(&stack, sizeof(struct token *));
	for (size_t i = 0; i < f->rpn_count; i++) {
		t = f->rpn[i];

		if (is_operator(t->type)) {
			struct token **left, **right;
			int a, b;

			if (stack.current_size < 2)
				goto bail;

			right = stack_top(&stack);
			stack_pop(&stack);
			left = stack_top(&stack);
			stack_pop(&stack);

			switch (t->type) {
			case TOKEN_OP_EQ:
				if (evaluate_eq(*left, *right, msg, &a))
					goto bail;
				t = a ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_NE:
				if (evaluate_eq(*left, *right, msg, &a))
					goto bail;
				t = !a ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_LT:
				if (evaluate_lt(*left, *right, msg, &a))
					goto bail;
				t = a ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_LE:
				if (evaluate_lt(*left, *right, msg, &a))
					goto bail;
				if (evaluate_eq(*left, *right, msg, &b))
					goto bail;
				t = a || b ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_GT:
				if (evaluate_lt(*left, *right, msg, &a))
					goto bail;
				if (evaluate_eq(*left, *right, msg, &b))
					goto bail;
				t = !a && !b ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_GE:
				if (evaluate_lt(*left, *right, msg, &a))
					goto bail;
				t = !a ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_AND:
				if (*right != &TRUE && *right != &FALSE)
					goto bail;
				if (*left != &TRUE && *left != &FALSE)
					goto bail;
				a = *left == &TRUE;
				b = *right == &TRUE;
				t = a && b ? &TRUE : &FALSE;
				break;
			case TOKEN_OP_OR:
				if (*right != &TRUE && *right != &FALSE)
					goto bail;
				if (*left != &TRUE && *left != &FALSE)
					goto bail;
				a = *left == &TRUE;
				b = *right == &TRUE;
				t = a || b ? &TRUE : &FALSE;
				break;
			default:
				goto bail;
			}
		}

		p = stack_push(&stack);
		*p = t;
	}

	if (stack.current_size != 1)
		goto bail;
	p = stack_top(&stack);
	if (*p == &TRUE)
		retval = 0;
	else if (*p == &FALSE)
		retval = 1;
bail:
	stack_destroy(&stack);
	return retval;
}
