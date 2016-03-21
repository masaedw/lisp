#include "lisp.h"


StObject St_LogAnd(StObject lhs, StObject rhs)
{
    return St_Integer(ST_INT_VALUE(lhs) & ST_INT_VALUE(rhs));
}

StObject St_LogIor(StObject lhs, StObject rhs)
{
    return St_Integer(ST_INT_VALUE(lhs) | ST_INT_VALUE(rhs));
}

StObject St_LogXor(StObject lhs, StObject rhs)
{
    return St_Integer(ST_INT_VALUE(lhs) ^ ST_INT_VALUE(rhs));
}

StObject St_LogNot(StObject n)
{
    return St_Integer(~ST_INT_VALUE(n));
}

StObject St_LogTest(StObject j, StObject k)
{
    return ST_BOOLEAN(ST_INT_VALUE(j) & ST_INT_VALUE(k));
}

StObject St_Ash(StObject n, StObject count)
{
    if (ST_INT_VALUE(count) > 0)
    {
        return St_Integer(ST_INT_VALUE(n) << ST_INT_VALUE(count));
    }
    else
    {
        return St_Integer(ST_INT_VALUE(n) >> -ST_INT_VALUE(count));
    }
}

static StObject subr_logand(StObject args)
{
    StObject a = St_Integer(-1);

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("logand: integer requried");
        }
        a = St_LogAnd(a, ST_CAR(p));
    }
    return a;
}

static StObject subr_logior(StObject args)
{
    StObject a = St_Integer(0);

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("logior: integer requried");
        }
        a = St_LogIor(a, ST_CAR(p));
    }
    return a;
}

static StObject subr_logxor(StObject args)
{
    StObject a = St_Integer(0);

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("logxor: integer requried");
        }
        a = St_LogXor(a, ST_CAR(p));
    }
    return a;
}

static StObject subr_lognot(StObject args)
{
    ST_ARGS1("lognot", args, n);

    if (!ST_INTP(n))
    {
        St_Error("lognot: integer required");
    }

    return St_LogNot(n);
}

static StObject subr_logtest(StObject args)
{
    ST_ARGS2("logtest", args, j, k);

    if (!ST_INTP(j) || !ST_INTP(k))
    {
        St_Error("lognot: integer required");
    }

    return St_LogTest(j, k);
}

static StObject subr_ash(StObject args)
{
    ST_ARGS2("ash", args, n, count);

    if (!ST_INTP(n) || !ST_INTP(count))
    {
        St_Error("ash: integer required");
    }

    return St_Ash(n, count);
}

void St_InitSrfi60(void)
{
    StObject m = GlobalModule;

    St_AddSubr(m, "logand", subr_logand);
    St_AddSubr(m, "bitwise-and", subr_logand);
    St_AddSubr(m, "logior", subr_logior);
    St_AddSubr(m, "bitwise-ior", subr_logior);
    St_AddSubr(m, "logxor", subr_logxor);
    St_AddSubr(m, "bitwise-xor", subr_logxor);
    St_AddSubr(m, "lognot", subr_lognot);
    St_AddSubr(m, "bitwise-not", subr_lognot);
    St_AddSubr(m, "logtest", subr_logtest);
    St_AddSubr(m, "any-bits-set?", subr_logtest);
    St_AddSubr(m, "ash", subr_ash);
    St_AddSubr(m, "arithmetic-shift", subr_ash);
}
