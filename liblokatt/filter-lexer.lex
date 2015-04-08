%{
#include "liblokatt/filter.h"

/* flex will generate code that will upset clang */
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
%}

%option outfile="out/liblokatt/filter-lexer.c"
%option header-file="out/liblokatt/filter-lexer.h"
%option noyywrap
%option reentrant

%%

[ \t\n]+	{ return TOKEN_WHITESPACE; }

pid	{ return TOKEN_KEY_PID; }
tid	{ return TOKEN_KEY_TID; }
sec	{ return TOKEN_KEY_SEC; }
nsec	{ return TOKEN_KEY_NSEC; }
level	{ return TOKEN_KEY_LEVEL; }
tag	{ return TOKEN_KEY_TAG; }
text	{ return TOKEN_KEY_TEXT; }
pname	{ return TOKEN_KEY_PNAME; }

==	{ return TOKEN_OP_EQ; }
!=	{ return TOKEN_OP_NE; }
\<	{ return TOKEN_OP_LT; }
\<=	{ return TOKEN_OP_LE; }
>	{ return TOKEN_OP_GT; }
>=	{ return TOKEN_OP_GE; }
=~	{ return TOKEN_OP_MATCH; }
!~	{ return TOKEN_OP_NMATCH; }

&&	{ return TOKEN_OP_AND; }
\|\|	{ return TOKEN_OP_OR; }

\(	{ return TOKEN_OP_LPAREN; }
\)	{ return TOKEN_OP_RPAREN; }

[0-9]+	{ return TOKEN_VALUE_INT; }
\"(\\.|[^"\\])*\"	{ return TOKEN_VALUE_STRING; }

.	{ return -1; }
