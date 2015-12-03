#include <stdio.h>
#include <ctype.h>

#include "lisp.h"

// 構文
// <expr>    ::= <term> | <list>
// <term>    ::= <integer> | <symbol> | '\'' <expr> | #t | #f
// <list>    ::= '(' <listsub>
// <listsub> ::= ')' | <expr> + ['.' <expr>] ')'
// <integer> ::= <数字> +
// <symbol>  ::= <見える文字> +

Object *read_term(FILE *stream);
Object *read_list(FILE *stream);
Object *read_listsub(FILE *stream);
Object *read_integer(FILE *stream);
Object *read_symbol(FILE *stream);

#define INT_LENGTH 9
#define SYMBOL_LENGTH 50

static int peek(FILE *stream)
{
    int c = getc(stream);
    ungetc(c, stream);
    return c;
}

static Object *read_expr(FILE* stream)
{
    return NULL;
}

static Object *read_term(FILE* stream)
{
    return NULL;
}

Object *St_Read(FILE* stream)
{
    return read_expr(stream);
}

