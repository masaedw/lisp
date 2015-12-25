#include "lisp.h"

#define I(x) St_Intern(x)

bool tailP(Object *next)
{
    return ST_CAR(next) == I("return");
}

static Object *compile(Object *x, Object *next)
{
    if (ST_SYMBOLP(x))
    {
        return ST_LIST3(I("refer"), x, next);
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
            Object *body = ST_CADDR(x);

            return ST_LIST4(I("close"), vars, compile(body, ST_LIST1(I("return"))), next);
        }

        if (car == I("if"))
        {
            Object *testE = ST_CADR(x);
            Object *thenE = ST_CADDR(x);
            Object *elseE = ST_CADR(ST_CDDR(x));

            Object *thenC = compile(thenE, next);
            Object *elseC = compile(elseE, next);

            return compile(testE, ST_LIST3(I("test"), thenC, elseC));
        }

        if (car == I("define"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            return compile(v, ST_LIST3(I("define"), var, next));
        }

        if (car == I("set!"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            return compile(v, ST_LIST3(I("assign"), var, next));
        }

        if (car == I("call/cc"))
        {
            Object *v = ST_CADR(x);
            Object *c = ST_LIST2(I("conti"), ST_LIST2(I("argument"), compile(v, ST_LIST1(I("apply")))));

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
        for (Object *args = ST_CDR(x), *c = compile(ST_CAR(x), ST_LIST1(I("apply")));
             ;
             c = compile(ST_CAR(args), ST_LIST2(I("argument"), c)), args = ST_CDR(args))
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

Object *St_Compile(Object *expr, Object *next)
{
    return compile(expr, next);
}
