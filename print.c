#include <stdio.h>

#include "lisp.h"

static void print(FILE *stream, Object *obj)
{
    if (obj == NULL)
    {
        St_Error("print: NULL");
    }

    switch (obj->type)
    {
    case TCELL:
        fprintf(stream, "(");

        print(stream, obj->car);

        while (ST_PAIRP(obj->cdr)) {
            obj = obj->cdr;
            fprintf(stream, " ");
            print(stream, obj->car);
        }

        if (!ST_NULLP(obj->cdr))
        {
            fprintf(stream, " . ");
            print(stream, obj->cdr);
        }

        fprintf(stream, ")");

        break;

#define CASE(type, ...)                         \
        case type:                              \
            fprintf(stream, __VA_ARGS__);       \
            break

        CASE(TINT, "%d", obj->int_value);
        CASE(TNIL, "()");
        CASE(TTRUE, "#t");
        CASE(TFALSE, "#f");
        CASE(TSYMBOL, "%s", obj->symbol_value);
        CASE(TSTRING, "\"%s\"", obj->string_value);
        CASE(TSYNTAX, "#<syntax %s>", obj->syntax_name);
        CASE(TSUBR, "#<subr %s>", obj->subr_name);
        CASE(TLAMBDA, "#<lambda>");
        CASE(TMACRO, "#<macro>");
#undef CASE

    default:
        St_Error("unknown type");
    }

    fflush(stream);
}


void St_Print(Object *obj)
{
    print(stdout, obj);
}
