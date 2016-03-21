#include <ctype.h>
#include <string.h>

#include "lisp.h"

static StObject read_list(StObject port, bool allow_dot);
static StObject read_quote(StObject port);
static StObject read_integer(StObject port, int first_digit);
static StObject read_hash(StObject port);
static void read_comment(StObject port);
static StObject read_symbol(StObject port, StObject first_char);
static StObject read_string(StObject port);

#define SYMBOL_LENGTH 50
#define STRING_LENGTH 5000

#define peek St_PeekChar

#define getc(p) St_ReadChar(p)
#define isspace_s(c) (!ST_EOFP(c) && isspace(ST_INT_VALUE(c)))
#define isdigit_s(c) (!ST_EOFP(c) && isdigit(ST_INT_VALUE(c)))
#define iscntrl_s(c) (!ST_EOFP(c) && iscntrl(ST_INT_VALUE(c)))

static void skip_space(StObject port)
{
    while (isspace_s(peek(port))) {
        getc(port);
    }
}

static StObject read_expr(StObject port)
{
    skip_space(port);

    StObject c = getc(port);

    if (ST_EOFP(c))
    {
        return Eof;
    }

    switch (ST_INT_VALUE(c)) {
    case '(':
        if (peek(port) == St_Integer(')'))
        {
            getc(port);
            return Nil;
        }
        return read_list(port, true);
    case '\'':
        return read_quote(port);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return read_integer(port, ST_INT_VALUE(c) - '0');
    case '#':
        return read_hash(port);
    case ';':
        read_comment(port);
        return read_expr(port);
    case '"':
        return read_string(port);
    case '-':
        if (isdigit_s(peek(port)))
        {
            return St_Integer(-ST_INT_VALUE(read_integer(port, 0)));
        }
        // fallthrough
    default:
        return read_symbol(port, c);
    }
}

static StObject read_list(StObject port, bool allow_dot)
{
    StObject head = Nil;
    StObject tail = Nil;
    bool is_next_last = false;
    bool is_next_end = false;

    while (true) {
        skip_space(port);

        StObject c = peek(port);

        if (ST_EOFP(c))
        {
            getc(port);
            St_Error("read: EOF in list");
        }

        if (ST_INT_VALUE(c) == ')')
        {
            if (is_next_last)
            {
                St_Error("read: dot must lead an expression");
            }
            getc(port);
            return head;
        }

        if (is_next_end)
        {
            St_Error("read: elapsed expression after dot");
        }

        StObject i = read_expr(port);

        if (ST_EOFP(i))
        {
            St_Error("read: unexpected in list");
        }

        if (i == St_Intern("."))
        {
            if (!allow_dot)
            {
                St_Error("read: unexpected dot");
            }

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

static StObject read_quote(StObject port)
{
    StObject expr = read_expr(port);

    if (!expr)
    {
        St_Error("read: unexpected quote expr");
    }

    return St_Cons(St_Intern("quote"), St_Cons(expr, Nil));
}

static StObject read_integer(StObject port, int first_digit)
{
    int value = first_digit;

    while (isdigit_s(peek(port))) {
        StObject c = getc(port);
        value = value * 10 + ST_INT_VALUE(c) - '0';
    }

    return St_Integer(value);
}

static StObject read_bytevector(StObject port)
{
    skip_space(port);
    StObject c = getc(port);

    if (ST_EOFP(c))
    {
        St_Error("read: eof in bytevector");
    }

    if (ST_INT_VALUE(c) == ')')
    {
        return Nil;
    }

    if (!isdigit_s(c))
    {
        St_Error("read: unexpected in bytevector");
    }

    StObject i = read_integer(port, ST_INT_VALUE(c) - '0');

    return St_Cons(i, read_bytevector(port));
}

static StObject read_hash(StObject port)
{
    skip_space(port);

    StObject c = getc(port);

    if (c == St_Integer('t'))
    {
        return True;
    }

    if (c == St_Integer('f'))
    {
        return False;
    }

    if (c == St_Integer('u'))
    {
        StObject d = getc(port);
        StObject e = getc(port);
        if (d == St_Integer('8') && e == St_Integer('('))
        {
            return St_MakeBytevectorFromList(read_bytevector(port));
        }
    }

    if (c == St_Integer('('))
    {
        StObject list = read_list(port, false);
        return St_MakeVectorFromList(list);
    }

    St_Error("read: unexpected hash literal");
}

static void read_comment(StObject port)
{
    StObject c;

    do {
        c = getc(port);
    } while (c != St_Integer('\n'));
}

static bool word_char(StObject c)
{
    if (ST_EOFP(c))
    {
        return false;
    }
    if (isspace_s(c) || iscntrl_s(c))
    {
        return false;
    }
    int x = ST_INT_VALUE(c);
    if (x == '(' || x == ')' || x == '\'' || x == '#' || x == ';')
    {
        return false;
    }

    return true; // TODO: more strict
}

static StObject read_symbol(StObject port, StObject first_char)
{
    char buf[SYMBOL_LENGTH + 1] = { (char)ST_INT_VALUE(first_char) };
    int p = 1;

    // TODO: buffer overflow
    while (word_char(peek(port))) {
        buf[p++] = ST_INT_VALUE(getc(port));
    }

    buf[p] = 0;

    return St_Intern(buf);
}

static StObject read_string(StObject port)
{
    char buf[STRING_LENGTH + 1];
    bool backslash = false;

    int p = 0;
    for (StObject c = getc(port); ; c = getc(port)) {
        if (ST_EOFP(c))
        {
            St_Error("read: unfinished string");
        }

        if (backslash)
        {
            switch (ST_INT_VALUE(c)) {
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

                St_Error("read: unsupported backslash literal: %c", (char)ST_INT_VALUE(c));
            }
            backslash = false;
        }
        else if (ST_INT_VALUE(c) == '"')
        {
            break;
        }
        else if (ST_INT_VALUE(c) == '\\')
        {
            backslash = true;
        }
        else
        {
            buf[p++] = (char)ST_INT_VALUE(c);
            if (p > STRING_LENGTH)
            {
                St_Error("read: too long string literal size");
            }
        }
    }

    return St_MakeString(p, buf);
}

StObject St_Read(StObject port)
{
    return read_expr(port);
}
