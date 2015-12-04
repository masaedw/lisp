#include <stdio.h>
#include <ctype.h>

#include "lisp.h"

static Object *read_list(FILE *stream);
static Object *read_quote(FILE *stream);
static Object *read_integer(FILE *stream, int first_digit);
static Object *read_hash(FILE *stream);
static void read_comment(FILE *stream);
static Object *read_symbol(FILE *stream, char first_char);

#define INT_LENGTH 9
#define SYMBOL_LENGTH 50

static int peek(FILE *stream)
{
    int c = getc(stream);
    ungetc(c, stream);
    return c;
}

static void skip_space(FILE *stream)
{
    while (isspace(peek(stream))) {
        getc(stream);
    }
}

static Object *read_expr(FILE* stream)
{
    skip_space(stream);

    int c = getc(stream);

    switch (c) {
    case '(':
        if (peek(stream) == ')')
        {
            getc(stream);
            return Nil;
        }
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
        return read_symbol(stream, c);
    }
}

static Object *read_list(FILE* stream)
{
    Object *head = Nil;
    Object *last = Nil;

    skip_space(stream);

    while (true) {
        int c = peek(stream);

        if (c == EOF)
        {
            getc(stream);
            St_Error("read: EOF in list");
        }

        // TODO: dotted pair

        if (c == ')')
        {
            getc(stream);
            return head;
        }

        Object *i = read_expr(stream);

        if (i == NULL)
        {
            St_Error("read: unexpected in list");
        }

        if (ST_NULLP(head))
        {
            head = last = St_Cons(i, Nil);
        }
        else
        {
            last->cdr = St_Cons(i, last);
        }
    }
}

static Object *read_quote(FILE *stream)
{
    return Nil;
}

static Object *read_integer(FILE *stream, int first_digit)
{
    return Nil;
}

static Object *read_hash(FILE *stream)
{
    skip_space(stream);

    int c = getc(stream);

    if (c == 't')
    {
        return True;
    }

    if (c == 'f')
    {
        return False;
    }

    St_Error("read: unexpected hash literal");
}

static void read_comment(FILE *stream)
{
    int c;

    do {
        c = getc(stream);
    } while (c != '\n');
}

static Object *read_symbol(FILE *stream, char first_char)
{
    return Nil;
}

Object *St_Read(FILE* stream)
{
    return read_expr(stream);
}

