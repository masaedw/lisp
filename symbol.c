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

    while (!ST_NULLP(p) && strcmp(symbol_value, p->car->symbol_value) != 0) {
        p = p->cdr;
    }

    if (ST_PAIRP(p))
    {
        return p->car;
    }

    Object *symbol = St_Alloc(TSYMBOL);
    size_t len = strlen(symbol_value);
    symbol->symbol_value = (char*)St_Malloc(len);
    strcpy(symbol->symbol_value, symbol_value);

    Symbols = St_Cons(symbol, Symbols);

    return symbol;
}
