#include <ctype.h>
#include <stdio.h> /* FIXME: rm */
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "filter-lexer.h"
#include "strbuf.h"

/*
 * Tokenizer for the filter language.
 *
 * For a larger grammar than the one employed to specify filter string, a
 * proper lexer/parser such as lex/yacc would make sense, but the overhead of
 * learning those tools again after not having touched them for well over ten
 * years did not appeal to me.
 *
 * This lexer is a finite state machine with the following parts:
 *
 *   - states (see enum state)
 *
 *   - recognized input classes (see enum sigma and sigma_value)
 *
 *   - operations:
 *
 *       - error: abort the FSM
 *       - push: push the current character onto the stack
 *       - token: create a new token based on the stack's contents and clear
 *                the stack
 *       - goto_state: change FSM state
 */

enum state {
	STATE_BEGIN,
	STATE_END,
	STATE_KEY,
	STATE_INT,
	STATE_STRING,
	STATE_ESCAPED_CHAR,
	NUMBER_OF_STATES,
};

enum sigma {
	SIGMA_WHITESPACE,
	SIGMA_ALPHA,
	SIGMA_INT,
	SIGMA_QUOTE,
	SIGMA_ESCAPE_CHAR,
	SIGMA_END,
	SIGMA_OTHER,
	NUMBER_OF_SIGMAS,
};

struct context {
	enum state state;
	struct token *tokens;
	size_t token_count;
	size_t max_token_count;
	struct strbuf chars;
};

static enum sigma sigma_value(char ch)
{
	if (ch == '\0')
		return SIGMA_END;
	if (ch == '"')
		return SIGMA_QUOTE;
	if (ch == '\\')
		return SIGMA_ESCAPE_CHAR;
	if (isdigit(ch))
		return SIGMA_INT;
	if (isalpha(ch))
		return SIGMA_ALPHA;
	if (isspace(ch))
		return SIGMA_WHITESPACE;
	return SIGMA_OTHER;
}

static void push(struct context *ctx, char ch)
{
	strbuf_addch(&ctx->chars, ch);
}

static void token(struct context *ctx, int token_type)
{
	struct token *token;

	if (ctx->token_count == ctx->max_token_count) {
		ctx->max_token_count *= 2;
		ctx->tokens = realloc(ctx->tokens, ctx->max_token_count);
	}

	token = &ctx->tokens[ctx->token_count++];
	token->type = token_type;
	switch (token_type) {
	case TOKEN_KEY:
	case TOKEN_STRING:
		/* FIXME: should check length... but not here or the state will be overwritten in the next goto_state */
		strncpy(token->text, ctx->chars.buf, TOKEN_MAX_TEXT_SIZE - 1);
		break;
	case TOKEN_INT:
		/* FIXME: check string to int conversion was successful */
		token->value = atoi(ctx->chars.buf);
		break;
	}

	strbuf_destroy(&ctx->chars);
	strbuf_init(&ctx->chars, 10);
}

static void goto_state(struct context *ctx, enum state new_state)
{
	ctx->state = new_state;
}

static void error(struct context *ctx)
{
	printf("error: unexpected char at FIXME\n");
	goto_state(ctx, STATE_END);
}

static int step_begin(struct context *ctx, enum sigma sigma, char ch)
{
	switch (sigma) {
	case SIGMA_WHITESPACE:
		return 0;
	case SIGMA_ALPHA:
		push(ctx, ch);
		goto_state(ctx, STATE_KEY);
		return 0;
	case SIGMA_INT:
		push(ctx, ch);
		goto_state(ctx, STATE_INT);
		return 0;
	case SIGMA_QUOTE:
		goto_state(ctx, STATE_STRING);
		return 0;
	case SIGMA_END:
		goto_state(ctx, STATE_END);
		return 0;
	default:
		error(ctx);
		return -1;
	}
}

static int step_end(struct context *ctx, enum sigma sigma, char ch)
{
	(void)ctx;
	(void)sigma;
	(void)ch;
	die("this function should never be called");
}

static int step_key(struct context *ctx, enum sigma sigma, char ch)
{
	switch (sigma) {
	case SIGMA_WHITESPACE:
		token(ctx, TOKEN_KEY);
		goto_state(ctx, STATE_BEGIN);
		return 0;
	case SIGMA_ALPHA:
		push(ctx, ch);
		return 0;
	case SIGMA_INT:
		token(ctx, TOKEN_KEY);
		push(ctx, ch);
		goto_state(ctx, STATE_INT);
		return 0;
	case SIGMA_END:
		token(ctx, TOKEN_KEY);
		goto_state(ctx, STATE_END);
		return 0;
	default:
		error(ctx);
		return -1;
	}

}

static int step_int(struct context *ctx, enum sigma sigma, char ch)
{
	switch (sigma) {
	case SIGMA_WHITESPACE:
		token(ctx, TOKEN_INT);
		goto_state(ctx, STATE_BEGIN);
		return 0;
	case SIGMA_ALPHA:
		token(ctx, TOKEN_INT);
		push(ctx, ch);
		goto_state(ctx, STATE_KEY);
		return 0;
	case SIGMA_INT:
		push(ctx, ch);
		return 0;
	case SIGMA_END:
		token(ctx, TOKEN_INT);
		goto_state(ctx, STATE_END);
		return 0;
	default:
		error(ctx);
		return -1;
	}
}

static int step_string(struct context *ctx, enum sigma sigma, char ch)
{
	switch (sigma) {
	case SIGMA_QUOTE:
		token(ctx, TOKEN_STRING);
		goto_state(ctx, STATE_BEGIN);
		return 0;
	case SIGMA_ESCAPE_CHAR:
		goto_state(ctx, STATE_ESCAPED_CHAR);
		return 0;
	case SIGMA_END:
		error(ctx);
		return -1;
	default:
		push(ctx, ch);
		return 0;
	}
}

static int step_escaped_char(struct context *ctx, enum sigma sigma, char ch)
{
	switch (sigma) {
	case SIGMA_QUOTE: /* fall through */
	case SIGMA_ESCAPE_CHAR:
		push(ctx, ch);
		goto_state(ctx, STATE_STRING);
		return 0;
	default:
		error(ctx);
		return -1;
	}
}

/* order of function pointers must match the order of enum state */
static int (*dispatch_table[])(struct context *, enum sigma, char) = {
	step_begin,
	step_end,
	step_key,
	step_int,
	step_string,
	step_escaped_char,
};

static int step(struct context *ctx, enum sigma sigma, char ch)
{
	int (*fn)(struct context *, enum sigma, char);
	if(ctx->state >= NUMBER_OF_STATES)
		die("unexpected state %d", ctx->state);
	fn = dispatch_table[ctx->state];
	return fn(ctx, sigma, ch);
}

static int run_state_machine(struct context *ctx,
			      const char *input)
{
	while (ctx->state != STATE_END) {
		if (step(ctx, sigma_value(*input), *input) != 0)
			return -1;
		input++;
	}
	return 0;
}

int filter_tokenize(const char *input,
		    struct token **out, size_t *out_size)
{
	struct context ctx;
	int retval;

	ctx.state = STATE_BEGIN;
	ctx.token_count = 0;
	ctx.max_token_count = 4;
	ctx.tokens = calloc(ctx.max_token_count, sizeof(struct token));
	strbuf_init(&ctx.chars, 10);

	if (run_state_machine(&ctx, input) == 0) {
		*out = ctx.tokens;
		*out_size = ctx.token_count;
		retval = 0;
	} else {
		*out = NULL;
		*out_size = 0;
		free(ctx.tokens);
		retval = -1;
	}

	strbuf_destroy(&ctx.chars);
	return retval;
}
