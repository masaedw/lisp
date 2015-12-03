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
Object *read_integer(FILE *stream, int first_digit);
Object *read_symbol(FILE *stream, char first_char);

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
    int c = getc(stream);

    switch (c) {
    case '(':
        return read_list(stream);
    case '\'':
        return read_quote(stream);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return read_integer(stream, c - '0');
    case '#':
        return read_hash(stream);
    case ';':
        read_comment(stream);
        return read_expr(stream);
    default:
        if (isspace(c))
        {
            do {
                c = getc(stream);
            } while (isspace(peek(stream)));
        }
        return read_symbol(stream, c);
    }
}

static Object *read_term(FILE* stream)
{
    return NULL;
}

Object *St_Read(FILE* stream)
{
    return read_expr(stream);
}

