#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lisp.h"

#define I(x) St_Intern(x)

static int module_add(StObject m, StObject sym)
{
    return St_ModuleFindOrInitialize(m, sym, Unbound);
}

static bool tailP(StObject next)
{
    return ST_CAR(next) == I("return");
}

static StObject compile(StObject x, StObject m, StObject e, StObject s, StObject next);

static StObject compile_body(StObject body, StObject m, StObject e, StObject s, StObject next)
{
    if (ST_NULLP(body))
    {
        return next;
    }

    return compile(ST_CAR(body), m, e, s, compile_body(ST_CDR(body), m, e, s, next));
}

static StObject compile_lookup(StObject module, StObject env, StObject x, StObject next, const char* insn)
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

static StObject compile_refer(StObject module, StObject env, StObject x, StObject next)
{
    return compile_lookup(module, env, x, next, "refer");
}

static StObject compile_assign(StObject module, StObject env, StObject x, StObject next)
{
    return compile_lookup(module, env, x, next, "assign");
}

static StObject collect_free(StObject module, StObject env, StObject vars, StObject next)
{
    ST_FOREACH(p, vars) {
        next = compile_refer(module, env, ST_CAR(p), ST_LIST2(I("argument"), next));
    }
    return next;
}

// returns (defined_symbols . definition_sequence_finished)
static StObject find_define_sub(StObject body)
{
    StObject defs = Nil;

    ST_FOREACH(p, body) {
        StObject x = ST_CAR(p);
        if (ST_PAIRP(x))
        {
            if (ST_CAR(x) == I("define"))
            {
                StObject sym = ST_CADR(x);
                if (St_SetMemberP(sym, defs))
                {
                    St_Error("define: multiple define: %s", ST_SYMBOL_VALUE(sym));
                }
                defs = St_Cons(ST_CADR(x), defs);
            }
            else if (ST_CAR(x) == I("begin"))
            {
                StObject r = find_define_sub(ST_CDR(x));
                defs = St_SetUnion(ST_CAR(r), defs);
                if (ST_CDR(r) == True)
                {
                    return St_Cons(defs, True);
                }
            }
            else
            {
                return St_Cons(defs, True);
            }
        }
    }
    return St_Cons(defs, False);
}

static StObject find_define(StObject body)
{
    return St_Reverse(ST_CAR(find_define_sub(body)));
}

static StObject find_free(StObject x, StObject b)
{
    if (ST_SYMBOLP(x))
    {
        return (St_SetMemberP(x, b))
            ? Nil
            : ST_LIST1(x);
    }

    if (ST_PAIRP(x))
    {
        StObject car = ST_CAR(x);

#define CASE(sym) if (car == I(#sym))

        CASE(quote) {
            return Nil;
        }

        CASE(lambda) {
            StObject vars = ST_CADR(x);
            StObject body = ST_CDDR(x);
            StObject nb = b;

            if (ST_SYMBOLP(vars))
            {
                nb = St_SetCons(vars, nb);
            }
            if (ST_PAIRP(vars))
            {
                StObject p;
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

        CASE(begin) {
            StObject body = ST_CDR(x);
            return find_free(body, b);
        }

        CASE(if) {
            StObject testc = ST_CADR(x);
            StObject thenc = ST_CADDR(x);
            StObject elsec = ST_CDR(ST_CDDR(x));

            return St_SetUnion(find_free(testc, b),
                               St_SetUnion(find_free(thenc, b),
                                           find_free(elsec, b)));
        }

        CASE(define) {
            StObject exp = ST_CADDR(x);

            return find_free(exp, b);
        }

        CASE(set!) {
            StObject var = ST_CADR(x);
            StObject exp = ST_CADDR(x);

            return St_SetUnion(St_SetMemberP(var, b) ? Nil : ST_LIST1(var),
                               find_free(exp, b));
        }

        CASE(call/cc) {
            StObject exp = ST_CADR(x);

            return find_free(exp, b);
        }

        else {
            StObject r = Nil;

            ST_FOREACH(p, x) {
                r = St_SetUnion(find_free(ST_CAR(p), b), r);
            }

            return r;
        }

#undef CASE

    }

    return Nil;
}

static StObject find_sets(StObject x, StObject v)
{
    if (ST_SYMBOLP(x))
    {
        return Nil;
    }

    if (ST_PAIRP(x))
    {
        StObject car = ST_CAR(x);

#define CASE(sym) if (car == I(#sym))

        CASE(quote) {
            return Nil;
        }

        CASE(lambda) {
            StObject vars = ST_CADR(x);
            StObject body = ST_CDDR(x);

            return find_sets(body, St_SetMinus(v, vars));
        }

        CASE(if) {
            StObject testc = ST_CADR(x);
            StObject thenc = ST_CADDR(x);
            StObject elsec = ST_CDR(ST_CDDR(x));

            return St_SetUnion(find_sets(testc, v),
                               St_SetUnion(find_sets(thenc, v),
                                           find_sets(elsec, v)));
        }

        CASE(set!) {
            StObject var = ST_CADR(x);
            StObject x2 = ST_CADDR(x);

            return St_SetUnion(St_SetMemberP(var, v) ? ST_LIST1(var) : Nil,
                               find_sets(x2, v));
        }

        CASE(call/cc) {
            StObject exp = ST_CADR(x);

            return find_sets(exp, v);
        }

        else {
            StObject r = Nil;

            ST_FOREACH(p, x) {
                r = St_SetUnion(find_sets(ST_CAR(p), v), r);
            }

            return r;
        }

#undef CASE

    }

    return Nil;
}

static StObject make_boxes(StObject sets, StObject vars, StObject next, int n)
{
    if (ST_NULLP(vars))
    {
        return next;
    }

    StObject next2 = make_boxes(sets, ST_CDR(vars), next, n + 1);

    return St_SetMemberP(ST_CAR(vars), sets)
        ? ST_LIST3(I("box"), St_Integer(n), next2)
        : next2;
}

static int macro_arity(StObject m)
{
    return ST_LAMBDA_ARITY(ST_MACRO_PROC(m));
}

static StObject macroexpand(StObject m, StObject x)
{
    if (ST_PAIRP(x))
    {
        StObject car = ST_CAR(x);

#define CASE(sym) if (car == I(#sym))

        CASE(quote) {
            return x;
        }

        if (ST_SYMBOLP(car))
        {
            StObject o = St_ModuleFind(m, car);
            if (ST_MACROP(o))
            {
                int arity = macro_arity(o);
                int len = St_Length(x) - 1;

                if (arity >= 0)
                {
                    if (arity != len)
                    {
                        St_Error("%s: wrong number of arguments: required %d but got %d", ST_SYMBOL_VALUE(ST_MACRO_SYMBOL(o)), arity, len);
                    }
                }
                else
                {
                    int required = -arity - 1;
                    if (required > len)
                    {
                        St_Error("%s: wrong number of arguments: required %d but got %d", ST_SYMBOL_VALUE(ST_MACRO_SYMBOL(o)), required, len);
                    }
                }

                StObject nx = St_Apply(ST_MACRO_PROC(o), ST_CDR(x));
                return macroexpand(m, nx);
            }
        }

        StObject p, h = Nil, t = Nil;

        for (p = x; ST_PAIRP(p); p = ST_CDR(p)) {
            StObject nx = macroexpand(m, ST_CAR(p));
            ST_APPEND1(h, t, nx);
        }

        if (!ST_NULLP(p))
        {
            StObject nx = macroexpand(m, p);
            ST_CDR_SET(t, nx);
        }

        return h;

#undef CASE

    }

    return x;
}

static StObject syntaxexpand(StObject m, StObject x)
{
    if (ST_PAIRP(x))
    {
        if (ST_SYMBOLP(ST_CAR(x)))
        {
            StObject o = St_ModuleFind(m, ST_CAR(x));

            if (ST_SYNTAXP(o))
            {
                return ST_SYNTAX_BODY(o)(m, x);
            }
        }

        StObject p, h = Nil, t = Nil;

        for (p = x; ST_PAIRP(p); p = ST_CDR(p)) {
            StObject nx = syntaxexpand(m, ST_CAR(p));
            ST_APPEND1(h, t, nx);
        }

        if (!ST_NULLP(p))
        {
            StObject nx = syntaxexpand(m, p);
            ST_CDR_SET(t, nx);
        }

        return h;
    }

    return x;
}

static StObject compile_and(StObject xs, StObject m, StObject e, StObject s, StObject next)
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

static StObject compile_or(StObject xs, StObject m, StObject e, StObject s, StObject next)
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

static StObject compile(StObject x, StObject m, StObject e, StObject s, StObject next)
{
    if (ST_SYMBOLP(x))
    {
        return compile_refer(m, e, x, St_SetMemberP(x, s) ? ST_LIST2(I("indirect"), next) : next);
    }

    if (ST_PAIRP(x))
    {
        StObject car = ST_CAR(x);

        if (car == I("quote"))
        {
            StObject obj = ST_CADR(x);

            return ST_LIST3(I("constant"), obj, next);
        }

        if (car == I("lambda"))
        {
            StObject params = ST_CADR(x);
            StObject body = ST_CDDR(x);
            StObject module_vars = St_ModuleSymbols(m);

            StObject p;
            int arity = 0;
            StObject vars = Nil;
            StObject tail = Nil;

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

            StObject defs = find_define(body);
            StObject extended_vars = St_SetAppend(defs, vars);
            StObject free = find_free(body, St_SetUnion(extended_vars, module_vars));
            StObject sets = find_sets(body, vars);

            int len_vars = St_Length(extended_vars);

            StObject nsets = St_SetUnion(sets,
                                         St_SetUnion(defs,
                                                     St_SetIntersect(s, free)));
            StObject body_c = compile_body(body, m, St_Cons(extended_vars, free), nsets,
                                           ST_LIST2(I("return"), St_Integer(len_vars)));

            if (abs(arity) != len_vars)
            {
                int to_extend = len_vars - abs(arity);
                body_c = ST_LIST3(I("extend"), St_Integer(to_extend), body_c);
            }

            return collect_free(m,
                                e,
                                free,
                                ST_LIST5(I("close"),
                                         St_Integer(arity),
                                         St_Integer(St_Length(free)),
                                         make_boxes(sets, vars, body_c, 0),
                                         next));
        }

        if (car == I("begin"))
        {
            return compile_body(ST_CDR(x), m, e, s, next);
        }

        if (car == I("if"))
        {
            StObject testE = ST_CADR(x);
            StObject thenE = ST_CADDR(x);
            StObject elseE = ST_CDR(ST_CDDR(x));

            StObject thenC = compile(thenE, m, e, s, next);
            StObject elseC = next;

            if (!ST_NULLP(elseE))
            {
                elseC = compile(ST_CAR(elseE), m, e, s, next);
            }

            return compile(testE, m, e, s, ST_LIST3(I("test"), thenC, elseC));
        }

        if (car == I("set!"))
        {
            StObject var = ST_CADR(x);
            StObject x2 = ST_CADDR(x);

            return compile(x2, m, e, s, compile_assign(m, e, var, next));
        }

        if (car == I("call/cc"))
        {
            StObject x2 = ST_CADR(x);
            return ST_LIST3(I("frame"),
                            next,
                            ST_LIST2(I("conti"),
                                     ST_LIST2(I("argument"),
                                              compile(x2, m, e, s, tailP(next)
                                                      ? ST_LIST4(I("shift"), St_Integer(1), ST_CADR(next), ST_LIST1(I("apply")))
                                                      : ST_LIST1(I("apply"))))));
        }

        if (car == I("define"))
        {
            StObject var = ST_CADR(x);
            StObject v = ST_CADDR(x);

            return compile(v, m, e, s, compile_assign(m, e, var, next));
        }

        if (car == I("define-macro"))
        {
            StObject var = ST_CADR(x);
            StObject v = ST_CADDR(x);

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

        // else clause
        for (StObject args = ST_CDR(x), c = compile(ST_CAR(x), m, e, s, tailP(next) ? ST_LIST4(I("shift"), St_Integer(St_Length(ST_CDR(x))), ST_CADR(next), ST_LIST1(I("apply"))) : ST_LIST1(I("apply")));
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

StObject St_Compile(StObject expr, StObject module, StObject next)
{
    return compile(syntaxexpand(module, macroexpand(module, expr)), module, St_Cons(Nil, Nil), Nil, next);
}

StObject St_MacroExpand(StObject module, StObject expr)
{
    return macroexpand(module, expr);
}

StObject St_SyntaxExpand(StObject module, StObject expr)
{
    return syntaxexpand(module, expr);
}
