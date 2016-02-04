#include <stdlib.h>
#include <string.h>

#include "lisp.h"

#define I(x) St_Intern(x)

static int module_add(Object *m, Object *sym)
{
    return St_ModuleFindOrInitialize(m, sym, Unbound);
}

static bool tailP(Object *next)
{
    return ST_CAR(next) == I("return");
}

static Object *compile(Object *x, Object *m, Object *e, Object *s, Object *next);

static Object *compile_body(Object *body, Object *m, Object *e, Object *s, Object *next)
{
    if (ST_NULLP(body))
    {
        return next;
    }

    return compile(ST_CAR(body), m, e, s, compile_body(ST_CDR(body), m, e, s, next));
}

static Object *compile_lookup(Object *module, Object *env, Object *x, Object *next, const char* insn)
{
    char buf[strlen(insn) + 9];
    strcpy(buf, insn);
    char *bp = buf + strlen(insn);

    int nl = 0;
    ST_FOREACH(locals, ST_CAR(env)) {
        if (ST_CAR(locals) == x)
        {
            strcpy(bp, "-local");
            return ST_LIST3(I(buf), St_Integer(nl), next);
        }
        nl++;
    }

    int nf = 0;
    ST_FOREACH(free, ST_CDR(env)) {
        if (ST_CAR(free) == x)
        {
            strcpy(bp, "-free");
            return ST_LIST3(I(buf), St_Integer(nf), next);
        }
        nf++;
    }

    strcpy(bp, "-module");
    int nm = module_add(module, x);
    return ST_LIST3(I(buf), St_Integer(nm), next);
}

static Object *compile_refer(Object *module, Object *env, Object *x, Object *next)
{
    return compile_lookup(module, env, x, next, "refer");
}

static Object *compile_assign(Object *module, Object *env, Object *x, Object *next)
{
    return compile_lookup(module, env, x, next, "assign");
}

static Object *collect_free(Object *module, Object *env, Object *vars, Object *next)
{
    ST_FOREACH(p, vars) {
        next = compile_refer(module, env, ST_CAR(p), ST_LIST2(I("argument"), next));
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
            Object *nb = b;

            if (ST_SYMBOLP(vars))
            {
                nb = St_SetCons(vars, nb);
            }
            if (ST_PAIRP(vars))
            {
                Object *p;
                for (p = vars; ST_PAIRP(p); p = ST_CDR(p)) {
                    nb = St_SetCons(ST_CAR(p), b);
                }
                if (!ST_NULLP(p))
                {
                    nb = St_SetCons(p, b);
                }
            }
            return find_free(body, nb);
        }

        CASE(let) {
            Object *bindings = ST_CADR(x);
            Object *body = ST_CDDR(x);
            Object *f = Nil;
            Object *nb = b;

            ST_FOREACH(p, bindings) {
                Object *bn = ST_CAR(p);
                f = St_SetUnion(find_free(ST_CADR(bn), b), f);
                nb = St_SetCons(ST_CAR(bn), nb);
            }
            return St_SetUnion(find_free(body, nb), f);
        }

        CASE(if) {
            Object *testc = ST_CADR(x);
            Object *thenc = ST_CADDR(x);
            Object *elsec = ST_CDR(ST_CDDR(x));

            return St_SetUnion(find_free(testc, b),
                               St_SetUnion(find_free(thenc, b),
                                           find_free(elsec, b)));
        }

        CASE(set!) {
            Object *var = ST_CADR(x);
            Object *exp = ST_CADDR(x);

            return St_SetUnion(St_SetMemberP(var, b) ? Nil : ST_LIST1(var),
                               find_free(exp, b));
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

static Object *find_sets(Object *x, Object *v)
{
    if (ST_SYMBOLP(x))
    {
        return Nil;
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

            return find_sets(body, St_SetMinus(v, vars));
        }

        CASE(if) {
            Object *testc = ST_CADR(x);
            Object *thenc = ST_CADDR(x);
            Object *elsec = ST_CDR(ST_CDDR(x));

            return St_SetUnion(find_sets(testc, v),
                               St_SetUnion(find_sets(thenc, v),
                                           find_sets(elsec, v)));
        }

        CASE(set!) {
            Object *var = ST_CADR(x);
            Object *x2 = ST_CADDR(x);

            return St_SetUnion(St_SetMemberP(var, v) ? ST_LIST1(var) : Nil,
                               find_sets(x2, v));
        }

        CASE(call/cc) {
            Object *exp = ST_CADR(x);

            return find_sets(exp, v);
        }

        else {
            Object *r = Nil;

            ST_FOREACH(p, x) {
                r = St_SetUnion(find_sets(ST_CAR(p), v), r);
            }

            return r;
        }

#undef CASE

    }

    return Nil;
}

static Object *make_boxes(Object *sets, Object *vars, Object *next, int n)
{
    if (ST_NULLP(vars))
    {
        return next;
    }

    Object *next2 = make_boxes(sets, ST_CDR(vars), next, n + 1);

    return St_SetMemberP(ST_CAR(vars), sets)
        ? ST_LIST3(I("box"), St_Integer(n), next2)
        : next2;
}

static int macro_arity(Object *m)
{
    return m->macro.proc->lambda.arity;
}

static Object *macroexpand(Object *m, Object *x)
{
    if (ST_PAIRP(x))
    {
        Object *car = ST_CAR(x);

#define CASE(sym) if (car == I(#sym))

        CASE(quote) {
            return x;
        }

        if (ST_SYMBOLP(car))
        {
            Object *o = St_ModuleFind(m, car);
            if (ST_MACROP(o))
            {
                int arity = macro_arity(o);
                int len = St_Length(x) - 1;

                if (arity >= 0)
                {
                    if (arity != len)
                    {
                        St_Error("%s: wrong number of arguments: required %d but got %d", o->macro.symbol->symbol.value, arity, len);
                    }
                }
                else
                {
                    int required = -arity - 1;
                    if (required > len)
                    {
                        St_Error("%s: wrong number of arguments: required %d but got %d", o->macro.symbol->symbol.value, required, len);
                    }
                }

                Object *nx = St_Apply(o->macro.proc, ST_CDR(x));
                return macroexpand(m, nx);
            }
        }

        Object *p, *h = Nil, *t = Nil;

        for (p = x; ST_PAIRP(p); p = ST_CDR(p)) {
            Object *nx = macroexpand(m, ST_CAR(p));
            ST_APPEND1(h, t, nx);
        }

        if (!ST_NULLP(p))
        {
            Object *nx = macroexpand(m, p);
            ST_CDR_SET(t, nx);
        }

        return h;

#undef CASE

    }

    return x;
}

static Object *compile_and(Object *xs, Object *m, Object *e, Object *s, Object *next)
{
    if (ST_NULLP(xs))
    {
        return ST_LIST3(I("constant"), True, next);
    }

    if (ST_NULLP(ST_CDR(xs)))
    {
        return compile(ST_CAR(xs), m, e, s, next);
    }

    return compile(ST_CAR(xs), m, e, s, ST_LIST3(I("test"), compile_and(ST_CDR(xs), m, e, s, next), next));
}

static Object *compile_or(Object *xs, Object *m, Object *e, Object *s, Object *next)
{
    if (ST_NULLP(xs))
    {
        return ST_LIST3(I("constant"), False, next);
    }

    if (ST_NULLP(ST_CDR(xs)))
    {
        return compile(ST_CAR(xs), m, e, s, next);
    }

    return compile(ST_CAR(xs), m, e, s, ST_LIST3(I("test"), next, compile_or(ST_CDR(xs), m, e, s, next)));
}

static Object *compile(Object *x, Object *m, Object *e, Object *s, Object *next)
{
    if (ST_SYMBOLP(x))
    {
        return compile_refer(m, e, x, St_SetMemberP(x, s) ? ST_LIST2(I("indirect"), next) : next);
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
            Object *params = ST_CADR(x);
            Object *body = ST_CDDR(x);
            Object *module_vars = St_ModuleSymbols(m);

            Object *p;
            int arity = 0;
            Object *vars = Nil;
            Object *tail = Nil;

            for (p = params; ST_PAIRP(p); p = ST_CDR(p))
            {
                arity++;
                ST_APPEND1(vars, tail, ST_CAR(p));
            }

            if (!ST_NULLP(p))
            {
                arity = -arity - 1;
                ST_APPEND1(vars, tail, p);
            }

            Object *free = find_free(body, St_SetUnion(vars, module_vars));
            Object *sets = find_sets(body, vars);

            return collect_free(m,
                                e,
                                free,
                                ST_LIST5(I("close"),
                                         St_Integer(arity),
                                         St_Integer(St_Length(free)),
                                         make_boxes(sets, vars,
                                                    compile_body(body, m, St_Cons(vars, free), St_SetUnion(sets, St_SetIntersect(s, free)),
                                                                 ST_LIST2(I("return"), St_Integer(abs(arity)))),
                                                    0),
                                         next));
        }

        if (car == I("if"))
        {
            Object *testE = ST_CADR(x);
            Object *thenE = ST_CADDR(x);
            Object *elseE = ST_CDR(ST_CDDR(x));

            Object *thenC = compile(thenE, m, e, s, next);
            Object *elseC = next;

            if (!ST_NULLP(elseE))
            {
                elseC = compile(ST_CAR(elseE), m, e, s, next);
            }

            return compile(testE, m, e, s, ST_LIST3(I("test"), thenC, elseC));
        }

        if (car == I("set!"))
        {
            Object *var = ST_CADR(x);
            Object *x2 = ST_CADDR(x);

            return compile(x2, m, e, s, compile_assign(m, e, var, next));
        }

        if (car == I("call/cc"))
        {
            Object *x2 = ST_CADR(x);
            return ST_LIST3(I("frame"),
                            next,
                            ST_LIST2(I("conti"),
                                     ST_LIST2(I("argument"),
                                              compile(x2, m, e, s, tailP(next)
                                                      ? ST_LIST4(I("shift"), St_Integer(1), ST_CADR(next), ST_LIST1(I("apply")))
                                                      : ST_LIST1(I("apply"))))));
        }

        if (car == I("define")) // not internal define for now
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            return compile(v, m, e, s, compile_assign(m, e, var, next));
        }

        if (car == I("define-macro"))
        {
            Object *var = ST_CADR(x);
            Object *v = ST_CADDR(x);

            return compile(v, m, e, s, ST_LIST3(I("macro"), var, compile_assign(m, e, var, next)));
        }

        if (car == I("and"))
        {
            return compile_and(ST_CDR(x), m, e, s, next);
        }

        if (car == I("or"))
        {
            return compile_or(ST_CDR(x), m, e, s, next);
        }

        if (ST_SYMBOLP(car))
        {
            Object *o = St_ModuleFind(m, car);

            if (ST_SYNTAXP(o))
            {
                return compile(o->syntax.body(x), m, e, s, next);
            }
        }

        // else clause
        for (Object *args = ST_CDR(x), *c = compile(ST_CAR(x), m, e, s, tailP(next) ? ST_LIST4(I("shift"), St_Integer(St_Length(ST_CDR(x))), ST_CADR(next), ST_LIST1(I("apply"))) : ST_LIST1(I("apply")));
             ;
             c = compile(ST_CAR(args), m, e, s, ST_LIST2(I("argument"), c)), args = ST_CDR(args))
        {
            if (ST_NULLP(args))
            {
                return tailP(next) ? c : ST_LIST3(I("frame"), next, c);
            }
        }
    } // pair

    return ST_LIST3(I("constant"), x, next);
}

Object *St_Compile(Object *expr, Object *module, Object *env, Object *next)
{
    return compile(macroexpand(module, expr), module, St_Cons(Nil, Nil), Nil, next);
}

Object *St_MacroExpand(Object *module, Object *expr)
{
    return macroexpand(module, expr);
}
