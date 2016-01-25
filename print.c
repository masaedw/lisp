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

        print(stream, ST_CAR(obj));

        while (ST_PAIRP(ST_CDR(obj))) {
            obj = ST_CDR(obj);
            fprintf(stream, " ");
            print(stream, ST_CAR(obj));
        }

        if (!ST_NULLP(ST_CDR(obj)))
        {
            fprintf(stream, " . ");
            print(stream, ST_CDR(obj));
        }

        fprintf(stream, ")");

        break;

    case TVECTOR: {
        fprintf(stream, "#(");

        for (int i = 0; i < St_VectorLength(obj); i++) {
            print(stream, St_VectorRef(obj, i));

            if (i < St_VectorLength(obj) - 1)
            {
                fprintf(stream, " ");
            }
        }

        fprintf(stream, ")");

        break;
    }

    case TSTRING: {
        for (int i = 0; i < obj->string.len; i++) {
            fprintf(stream, "%c", obj->string.value[i]);
        }

        break;
    }

#define CASE(type, ...)                         \
        case type:                              \
            fprintf(stream, __VA_ARGS__);       \
            break

        CASE(TINT, "%d", obj->integer.value);
        CASE(TNIL, "()");
        CASE(TTRUE, "#t");
        CASE(TFALSE, "#f");
        CASE(TSYMBOL, "%s", obj->symbol.value);
        CASE(TSYNTAX, "#<syntax %s>", obj->syntax.name);
        CASE(TSUBR, "#<subr %s>", obj->subr.name);
        CASE(TLAMBDA, "#<lambda>");
        CASE(TLAMBDAVM, "#<lambda-vm>");
        CASE(TMACRO, "#<macro>");
        CASE(TUNBOUND, "#<unbound>");
#undef CASE

    default:
        St_Error("unknown type %d", obj->type);
    }

    fflush(stream);
}


void St_Print(Object *obj)
{
    print(stdout, obj);
}
