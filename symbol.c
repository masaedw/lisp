#include <string.h>

#include "lisp.h"

Object *Symbols = NULL;

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

    size_t len = strlen(symbol_value);
    Object *symbol = St_Alloc(TSYMBOL, len);
    strcpy(symbol->symbol.value, symbol_value);

    Symbols = St_Cons(symbol, Symbols);

    return symbol;
}
