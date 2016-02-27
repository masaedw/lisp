#include <stdarg.h>
#include <stdio.h>

#include "lisp.h"

static void print(StObject obj, StObject port)
{
    if (obj == NULL)
    {
        St_Error("print: NULL");
    }

    if (ST_INTP(obj))
    {
        char buf[20];
        sprintf(buf, "%ld", ST_INT_VALUE(obj));
        St_WriteCString(buf, port);
        return;
    }

    switch ((intptr_t)obj)
    {
#define CASE(type, str)                         \
        case (intptr_t)type:                    \
            St_WriteCString(str, port);         \
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
        St_WriteU8('(', port);

        print(ST_CAR(obj), port);

        while (ST_PAIRP(ST_CDR(obj))) {
            obj = ST_CDR(obj);
            St_WriteU8(' ', port);
            print(ST_CAR(obj), port);
        }

        if (!ST_NULLP(ST_CDR(obj)))
        {
            St_WriteCString(" . ", port);
            print(ST_CDR(obj), port);
        }

        St_WriteU8(')', port);

        break;

    case TVECTOR: {
        St_WriteCString("#(", port);

        for (size_t i = 0; i < ST_VECTOR_LENGTH(obj); i++) {
            print(St_VectorRef(obj, i), port);

            if (i < ST_VECTOR_LENGTH(obj) - 1)
            {
                St_WriteU8(' ', port);
            }
        }

        St_WriteU8(')', port);

        break;
    }

    case TBYTEVECTOR: {
        St_WriteCString("#u8(", port);
        int len = ST_BYTEVECTOR_LENGTH(obj);

        for (int i = 0; i < len; i++) {
            char buf[4];
            sprintf(buf, "%d", St_BytevectorU8Ref(obj, i));
            St_WriteCString(buf, port);

            if (i < len - 1)
            {
                St_WriteU8(' ', port);
            }
        }

        St_WriteU8(')', port);

        break;
    }

    case TSTRING: {
        St_WriteBuffer(ST_STRING_VALUE(obj), ST_STRING_LENGTH(obj), port);

        break;
    }

    case TSYMBOL: {
        St_WriteCString(ST_SYMBOL_VALUE(obj), port);

        break;
    }

    case TSYNTAX: {
        St_WriteCString("#<syntax ", port);
        St_WriteCString(ST_SYNTAX_NAME(obj), port);
        St_WriteCString(">", port);

        break;
    }

    case TSUBR: {
        St_WriteCString("#<subr ", port);
        St_WriteCString(ST_SUBR_NAME(obj), port);
        St_WriteCString(">", port);

        break;
    }

    case TLAMBDA: {
        St_WriteCString("#<lambda>", port);

        break;
    }

    case TMACRO: {
        St_WriteCString("#<macro>", port);

        break;
    }

    case TFDPORT: {
        St_WriteCString("#<port fd:", port);
        char buf[20];
        sprintf(buf, "%d", ST_FDPORT_FD(obj));
        St_WriteCString(buf, port);
        St_WriteU8('>', port);
    }

    default:
        St_Error("unknown type %d", obj->type);
    }
}

void St_Display(StObject obj, StObject port)
{
    print(obj, port);
}

void St_Print(StObject obj, StObject port)
{
    print(obj, port);
    St_Newline(port);
}
