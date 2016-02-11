#include <stdio.h>

#include "lisp.h"

static void print(FILE *stream, StObject obj)
{
    if (obj == NULL)
    {
        St_Error("print: NULL");
    }

    if (ST_INTP(obj))
    {
        fprintf(stream, "%ld", ST_INT_VALUE(obj));
        fflush(stream);
        return;
    }

    switch ((intptr_t)obj)
    {
#define CASE(type, ...)                         \
        case (intptr_t)type:                    \
            fprintf(stream, __VA_ARGS__);       \
            return

        CASE(Nil, "()");
        CASE(True, "#t");
        CASE(False, "#f");
        CASE(Eof, "#<eof-object>");
        CASE(Unbound, "#<unbound>");

#undef CASE
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

    case TBYTEVECTOR: {
        fprintf(stream, "#u8(");
        int len = St_BytevectorLength(obj);

        for (int i = 0; i < len; i++) {
            fprintf(stream, "%d", St_BytevectorU8Ref(obj, i));

            if (i < len - 1)
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

        CASE(TSYMBOL, "%s", obj->symbol.value);
        CASE(TSYNTAX, "#<syntax %s>", obj->syntax.name);
        CASE(TSUBR, "#<subr %s>", obj->subr.name);
        CASE(TLAMBDA, "#<lambda>");
        CASE(TMACRO, "#<macro>");
        CASE(TFDPORT, "#<port fd:%d>", obj->fd_port.fd);

#undef CASE

    default:
        St_Error("unknown type %d", obj->type);
    }

    fflush(stream);
}

void St_Display(StObject obj)
{
    print(stdout, obj);
}

void St_Print(StObject obj)
{
    print(stdout, obj);
    fprintf(stdout, "\n");
}
