#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "lisp.h"

static StObject read_list(FILE *stream);
static StObject read_quote(FILE *stream);
static StObject read_integer(FILE *stream, int first_digit);
static StObject read_hash(FILE *stream);
static void read_comment(FILE *stream);
static StObject read_symbol(FILE *stream, char first_char);
static StObject read_string(FILE *stream);

#define SYMBOL_LENGTH 50
#define STRING_LENGTH 5000

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

static StObject read_expr(FILE* stream)
{
    skip_space(stream);

    int c = getc(stream);

    switch (c) {
    case EOF:
        return Eof;
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
    case '"':
        return read_string(stream);
    default:
        return read_symbol(stream, c);
    }
}

static StObject read_list(FILE* stream)
{
    StObject head = Nil;
    StObject tail = Nil;
    bool is_next_last = false;
    bool is_next_end = false;

    while (true) {
        skip_space(stream);

        int c = peek(stream);

        if (c == EOF)
        {
            getc(stream);
            St_Error("read: EOF in list");
        }

        if (c == ')')
        {
            if (is_next_last)
            {
                St_Error("read: dot must lead an expression");
            }
            getc(stream);
            return head;
        }

        if (is_next_end)
        {
            St_Error("read: elapsed expression after dot");
        }

        StObject i = read_expr(stream);

        if (i == NULL)
        {
            St_Error("read: unexpected in list");
        }

        if (i == St_Intern("."))
        {
            if (head == Nil)
            {
                St_Error("read: dot must have car part in front of it");
            }

            if (is_next_last)
            {
                St_Error("read: elapsed dot");
            }
            is_next_last = true;
            continue;
        }

        if (is_next_last)
        {
            ST_CDR_SET(tail, i);
            is_next_last = false;
            is_next_end = true;
        }
        else
        {
            ST_APPEND1(head, tail, i);
        }
    }
}

static StObject read_quote(FILE *stream)
{
    StObject expr = read_expr(stream);

    if (!expr)
    {
        St_Error("read: unexpected quote expr");
    }

    return St_Cons(St_Intern("quote"), St_Cons(expr, Nil));
}

static StObject read_integer(FILE *stream, int first_digit)
{
    int value = first_digit;

    while (isdigit(peek(stream))) {
        int c = getc(stream);
        value = value * 10 + c - '0';
    }

    return St_Integer(value);
}

static StObject read_hash(FILE *stream)
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

static bool word_char(int c)
{
    if (c == EOF)
    {
        return false;
    }
    if (isspace(c) || iscntrl(c))
    {
        return false;
    }
    if (c == '(' || c == ')' || c == '\'' || c == '#' || c == ';')
    {
        return false;
    }

    return true; // TODO: more strict
}

static StObject read_symbol(FILE *stream, char first_char)
{
    char buf[SYMBOL_LENGTH + 1] = { first_char };
    int p = 1;

    // TODO: buffer overflow
    while (word_char(peek(stream))) {
        buf[p++] = (char)getc(stream);
    }

    buf[p] = 0;

    return St_Intern(buf);
}

static StObject read_string(FILE *stream)
{
    char buf[STRING_LENGTH + 1];
    bool backslash = false;

    int p = 0;
    for (int c = getc(stream); ; c = getc(stream)) {
        if (c == EOF)
        {
            St_Error("read: unfinished string");
        }

        if (backslash)
        {
            switch (c) {
            case '0':
                buf[p++] = '\0';
                break;
            case 'a':
                buf[p++] = '\a';
                break;
            case 'b':
                buf[p++] = '\b';
                break;
            case 'f':
                buf[p++] = '\f';
                break;
            case 'n':
                buf[p++] = '\n';
                break;
            case 'r':
                buf[p++] = '\r';
                break;
            case 'v':
                buf[p++] = '\v';
                break;
            case '\\':
                buf[p++] = '\\';
                break;
            case '"':
                buf[p++] = '"';
                break;
            default:

                St_Error("read: unsupported backslash literal: %c", c);
            }
            backslash = false;
        }
        else if (c == '"')
        {
            break;
        }
        else if (c == '\\')
        {
            backslash = true;
        }
        else
        {
            buf[p++] = (char)c;
            if (p > STRING_LENGTH)
            {
                St_Error("read: too long string literal size");
            }
        }
    }

    return St_MakeString(p, buf);
}

StObject St_Read(FILE* stream)
{
    return read_expr(stream);
}
