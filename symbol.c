#include <stdio.h>
#include <string.h>

#include "lisp.h"

static StObject Symbols = Nil;

static StObject push(const char* symbol_value)
{
    size_t len = strlen(symbol_value);
    StSymbol symbol = St_Alloc2(TSYMBOL, sizeof(struct StSymbolRec) + len);
    strcpy(symbol->value, symbol_value);

    Symbols = St_Cons(ST_OBJECT(symbol), Symbols);

    return ST_OBJECT(symbol);
}

StObject St_Intern(const char *symbol_value)
{
    ST_FOREACH(p, Symbols) {
        if (strcmp(symbol_value, ST_SYMBOL_VALUE(ST_CAR(p))) == 0)
        {
            return ST_CAR(p);
        }
    }

    return push(symbol_value);
}

StObject St_Gensym(void)
{
    static int c = 0;
    static const int buf_size = 30;

    char buf[buf_size];

    snprintf(buf, buf_size, "gensym_%d", c++);

    return push(buf);
}

StObject St_SymbolToString(StObject sym)
{
    return St_MakeStringFromCString(ST_SYMBOL_VALUE(sym));
}

StObject St_StringToSymbol(StObject str)
{
    return St_Intern(St_StringGetCString(str));
}
