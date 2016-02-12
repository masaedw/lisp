#include <stdio.h>
#include <string.h>

#include "lisp.h"

StObject Symbols = Nil;

static StObject push(const char* symbol_value)
{
    size_t len = strlen(symbol_value);
    StObject symbol = St_Alloc(TSYMBOL, len + 1);
    strcpy(symbol->symbol.value, symbol_value);

    Symbols = St_Cons(symbol, Symbols);

    return symbol;
}

StObject St_Intern(const char *symbol_value)
{
    ST_FOREACH(p, Symbols) {
        if (strcmp(symbol_value, ST_CAR(p)->symbol.value) == 0)
        {
            return ST_CAR(p);
        }
    }

    return push(symbol_value);
}

StObject St_Gensym()
{
    static int c = 0;
    static const int buf_size = 30;

    char buf[buf_size];

    snprintf(buf, buf_size, "gensym_%d", c++);

    return push(buf);
}
