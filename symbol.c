#include <string.h>

#include "lisp.h"

Object *Symbols = NULL;

static Object* push(const char* symbol_value)
{
    size_t len = strlen(symbol_value);
    Object *symbol = St_Alloc(TSYMBOL, len);
    strcpy(symbol->symbol.value, symbol_value);

    Symbols = St_Cons(symbol, Symbols);

    return symbol;
}

Object *St_Intern(const char *symbol_value)
{
    Object *p = Symbols;

    if (!p)
    {
        p = Symbols = Nil;
    }

    while (!ST_NULLP(p) && strcmp(symbol_value, ST_CAR(p)->symbol.value) != 0) {
        p = ST_CDR(p);
    }

    if (ST_PAIRP(p))
    {
        return ST_CAR(p);
    }

    return push(symbol_value);
}

Object *St_Gensym()
{
    static int c = 0;
    static const int buf_size = 30;

    char buf[buf_size];

    snprintf(buf, buf_size, "gensym_%d", c++);

    return push(buf);
}
