#include "lisp.h"

#define I(x) St_Intern(x)

bool tailP(Object *next)
{
    return ST_CAR(next) == I("return");
}

static Object *compile(Object *x, Object *e, Object *next);

static Object *compile_body(Object *body, Object *e, Object *next)
{
    if (ST_NULLP(body))
    {
        return next;
    }

    return compile(ST_CAR(body), e, compile_body(ST_CDR(body), e, next));
}

static Object *compile_lookup(Object *env, Object *var)
{
    int rib = 0;
    for (Object *e = env; !ST_NULLP(e); e = ST_CAR(e), rib++) {
        int elt = 0;
        for (Object *p = ST_CADR(e); !ST_NULLP(p); p = ST_CDR(p), elt++) {
            if (ST_CAAR(p) == var)
            {
                return St_Cons(St_Integer(rib), St_Integer(elt));
            }
        }
    }

    St_Print(var);
    St_Error("compile: unbound variable");
}

static Object *extend(Object *env, Object *vars)
{
    return St_PushEnv(env, vars, vars);
}

static Object *compile(Object *x, Object *e, Object *next)
{
    if (ST_SYMBOLP(x))
    {
        return ST_LIST3(I("refer"), compile_lookup(e, x), next);
    }

    if (ST_PAIRP(x))
    {
        Object *car = ST_CAR(x);

        if (car == I("quote"))
        {
            Object *obj = ST_CADR(x);

            return ST_LIST3(I("constant"), obj, next);
        }

        if (car == I("lambda"))
        {
            Object *vars = ST_CADR(x);
            Object *body = ST_CDDR(x);

            return ST_LIST3(I("close"), compile_body(body, extend(e, vars), ST_LIST1(I("return"))), next);
        }

        if (car == I("if"))
        {
            Object *testE = ST_CADR(x);
            Object *thenE = ST_CADDR(x);
            Object *elseE = ST_CADR(ST_CDDR(x));

            Object *thenC = compile(thenE, e, next);
            Object *elseC = compile(elseE, e, next);

            return compile(testE, e, ST_LIST3(I("test"), thenC, elseC));
        }

        if (car == I("define"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            St_AddVariable(e, var, var);

            return compile(v, e, ST_LIST3(I("define"), var, next));
        }

        if (car == I("set!"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            return compile(v, e, ST_LIST3(I("assign"), var, next));
        }

        if (car == I("call/cc"))
        {
            Object *v = ST_CADR(x);
            Object *c = ST_LIST2(I("conti"), ST_LIST2(I("argument"), compile(v, e, ST_LIST1(I("apply")))));

            if (tailP(next))
            {
                return c;
            }
            else
            {
                return ST_LIST3(I("frame"), next, c);
            }
        }

        // else clause
        for (Object *args = ST_CDR(x), *c = compile(ST_CAR(x), e, ST_LIST1(I("apply")));
             ;
             c = compile(ST_CAR(args), e, ST_LIST2(I("argument"), c)), args = ST_CDR(args))
        {
            if (ST_NULLP(args))
            {
                if (tailP(next))
                {
                    return c;
                }
                else
                {
                    return ST_LIST3(I("frame"), next, c);
                }
            }
        }
    } // pair

    return ST_LIST3(I("constant"), x, next);
}

Object *St_Compile(Object *expr, Object *env, Object *next)
{
    return compile(expr, env, next);
}
