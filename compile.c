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

static Object *compile_refer(Object *env, Object *x, Object *next)
{
    int nl = 0;
    ST_FOREACH(locals, ST_CAR(env)) {
        if (ST_CAR(locals) == x)
        {
            return ST_LIST3(I("refer-local"), St_Integer(nl), next);
        }
        nl++;
    }

    int nf = 0;
    ST_FOREACH(free, ST_CDR(env)) {
        if (ST_CAR(free) == x)
        {
            return ST_LIST3(I("refer-free"), St_Integer(nf), next);
        }
        nf++;
    }

    return Nil; // never reached
}

static Object *collect_free(Object *env, Object *vars, Object *next)
{
    ST_FOREACH(p, vars) {
        next = compile_refer(env, ST_CAR(p), ST_LIST2(I("argument"), next));
    }
    return next;
}

static Object *find_free(Object *x, Object *b)
{
    if (ST_SYMBOLP(x))
    {
        return (St_SetMemberP(x, b))
            ? Nil
            : ST_LIST1(x);
    }

    if (ST_PAIRP(x))
    {
        Object *car = ST_CAR(x);

#define CASE(sym) if (car == I(#sym))

        CASE(quote) {
            return Nil;
        }

        CASE(lambda) {
            Object *vars = ST_CADR(x);
            Object *body = ST_CDDR(x);

            return find_free(body, St_SetUnion(vars, b));
        }

        CASE(if) {
            Object *testc = ST_CADR(x);
            Object *thenc = ST_CADDR(x);
            Object *elsec = ST_CADR(ST_CDDR(x));

            return St_SetUnion(find_free(testc, b),
                               St_SetUnion(find_free(thenc, b),
                                           find_free(elsec, b)));
        }

        CASE(call/cc) {
            Object *exp = ST_CADR(x);

            return find_free(exp, b);
        }

        else {
            Object *r = Nil;

            ST_FOREACH(p, x) {
                r = St_SetUnion(find_free(ST_CAR(p), b), r);
            }

            return r;
        }

#undef CASE

    }

    return Nil;
}

static Object *compile(Object *x, Object *e, Object *next)
{
    if (ST_SYMBOLP(x))
    {
        return compile_refer(e, x, next);
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

            Object *free = find_free(body, vars);

            return collect_free(e,
                                free,
                                ST_LIST4(I("close"),
                                         St_Integer(St_Length(free)),
                                         compile_body(body, St_Cons(vars, free),
                                                      ST_LIST2(I("return"), St_Integer(St_Length(vars)))),
                                         next));
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

        if (car == I("call/cc"))
        {
            Object *x2 = ST_CADR(x);
            return ST_LIST3(I("frame"),
                            next,
                            ST_LIST2(I("conti"),
                                     ST_LIST2(I("argument"),
                                              compile(x2, e, ST_LIST1(I("apply"))))));
        }

        /*
        if (car == I("define"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            St_AddVariable(e, var, var);

            int n, m;
            compile_lookup(e, var, &n, &m);

            return compile(v, e, ST_LIST4(I("define"), St_Integer(n), St_Integer(m), next));
        }

        if (car == I("set!"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            int n, m;
            compile_lookup(e, var, &n, &m);

            return compile(v, e, ST_LIST4(I("assign"), St_Integer(n), St_Integer(m), next));
        }
        */

        // else clause
        for (Object *args = ST_CDR(x), *c = compile(ST_CAR(x), e, ST_LIST1(I("apply")));
             ;
             c = compile(ST_CAR(args), e, ST_LIST2(I("argument"), c)), args = ST_CDR(args))
        {
            if (ST_NULLP(args))
            {
                return ST_LIST3(I("frame"), next, c);
            }
        }
    } // pair

    return ST_LIST3(I("constant"), x, next);
}

Object *St_Compile(Object *expr, Object *env, Object *next)
{
    Object *head = Nil;
    Object *tail = Nil;

    for (Object *p = env; !ST_NULLP(p); p = ST_CAR(p)) {
        ST_FOREACH(q, ST_CADR(p)) {
            ST_APPEND1(head, tail, ST_CAAR(q));
        }
    }
    return compile(expr, St_Cons(Nil, head), next);
}
