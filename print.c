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
    case TINT:
        fprintf(stream, "%d", obj->int_value);
        break;

    case TCELL:
        fprintf(stream, "(");

        print(stream, obj->car);

        while (ST_PAIRP(obj->cdr))
        {
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

    case TNIL:
        fprintf(stream, "()");
        break;

    case TTRUE:
        fprintf(stream, "#t");
        break;

    case TFALSE:
        fprintf(stream, "#f");
        break;

    case TSYMBOL:
        fprintf(stream, "%s", obj->symbol_value);
        break;

    case TSYNTAX:
        fprintf(stream, "#<syntax %s>", obj->syntax_name);
        break;

    case TSUBR:
        fprintf(stream, "#<subr %s>", obj->subr_name);
        break;

    case TLAMBDA:
        fprintf(stream, "#<lambda>");
        break;

    default:
        St_Error("unknown type");
    }

    fflush(stream);
}


void St_Print(Object *obj)
{
    print(stdout, obj);
}
