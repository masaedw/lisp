#ifndef SUBR_H
#define SUBR_H

#include "lisp.h"

// Subr definition utilities

#define ST_CHECK_CINFO_LEN(name, cinfo, len)                \
    do {                                                    \
        if (cinfo->count != len)                            \
        {                                                   \
            St_Error(name ": wrong number of arguments");   \
        }                                                   \
    }while (0)


#define ST_ARGS0(name, cinfo)                   \
    ST_CHECK_CINFO_LEN(name, cinfo, 0)

#define ST_ARGS1(name, cinfo, a1)               \
    ST_CHECK_CINFO_LEN(name, cinfo, 1);         \
    StObject (a1) = St_Arg(cinfo, 0)

#define ST_ARGS2(name, cinfo, a1, a2)           \
    ST_CHECK_CINFO_LEN(name, cinfo, 2);         \
    StObject (a1) = St_Arg(cinfo, 0);           \
    StObject (a2) = St_Arg(cinfo, 1)

#define ST_ARGS3(name, cinfo, a1, a2, a3)       \
    ST_CHECK_CINFO_LEN(name, cinfo, 3);         \
    StObject (a1) = St_Arg(cinfo, 0);           \
    StObject (a2) = St_Arg(cinfo, 1);           \
    StObject (a3) = St_Arg(cinfo, 2)


#define ST_ARGS4(name, cinfo, a1, a2, a3, a4)   \
    ST_CHECK_CINFO_LEN(name, cinfo, 4);         \
    StObject (a1) = St_Arg(cinfo, 0);           \
    StObject (a2) = St_Arg(cinfo, 1);           \
    StObject (a3) = St_Arg(cinfo, 2);           \
    StObject (a4) = St_Arg(cinfo, 3)

#define ST_ARG_FOREACH(i, n) for (int i = n; i < cinfo->count; i++)
#define ARG(o, i) StObject o = St_Arg(cinfo, i)

static inline StObject St_Arg(StCallInfo *cinfo, int i)
{
    return St_VectorRef(ST_OBJECT(cinfo->argstack), cinfo->offset + ~i);
}

#endif
