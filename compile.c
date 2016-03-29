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

static StObject primitiveSyntaxes()
{
    static char *syntaxes[] = {
        "or",
        "and",
        "define-macro",
        NULL,
    };

    static StObject ss = Nil;

    if (!ST_NULLP(ss))
    {
        return ss;
    }

    StObject x = Nil;
    for (char **p = syntaxes; *p != NULL; p++) {
        x = St_Cons(I(*p), x);
    }

    ss = x;

    return ss;
}

typedef struct
{
    StObject module;
    StObject env;
    StObject sets;
    bool toplevel;
    int stackoffset;
} StCompileContext;


static StObject compile(StCompileContext *ctx, StObject expr, StObject next);

static StObject compile_body(StCompileContext *ctx, StObject body, StObject next)
{
    if (ST_NULLP(body))
    {
        return next;
    }

    return compile(ctx, ST_CAR(body), compile_body(ctx, ST_CDR(body), next));
}

static StObject compile_lookup(StCompileContext *ctx, StObject x, StObject next, const char* insn)
{
    char buf[strlen(insn) + 9];
    strcpy(buf, insn);
    char *bp = buf + strlen(insn);

    int nl = 0;
    ST_FOREACH(locals, ST_CAR(ctx->env)) {
        if (ST_CAR(locals) == x)
        {
            strcpy(bp, "-local");
            return ST_LIST3(I(buf), St_Integer(nl - ctx->stackoffset), next);
        }
        nl++;
    }

    int nf = 0;
    ST_FOREACH(free, ST_CDR(ctx->env)) {
        if (ST_CAR(free) == x)
        {
            strcpy(bp, "-free");
            return ST_LIST3(I(buf), St_Integer(nf), next);
        }
        nf++;
    }

    strcpy(bp, "-module");
    int nm = module_add(ctx->module, x);
    return ST_LIST3(I(buf), St_Integer(nm), next);
}

static StObject compile_refer(StCompileContext *ctx, StObject x, StObject next)
{
    return compile_lookup(ctx, x, next, "refer");
}

static StObject compile_assign(StCompileContext *ctx, StObject x, StObject next)
{
    return compile_lookup(ctx, x, next, "assign");
}

static StObject collect_free(StCompileContext *ctx, StObject vars, StObject next)
{
    ST_FOREACH(p, vars) {
        next = compile_refer(ctx, ST_CAR(p), ST_LIST2(I("argument"), next));
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

            if (ST_SYMBOLP(vars))
            {
                b = St_SetCons(vars, b);
            }
            if (ST_PAIRP(vars))
            {
                StObject p;
                for (p = vars; ST_PAIRP(p); p = ST_CDR(p)) {
                    b = St_SetCons(ST_CAR(p), b);
                }
                if (!ST_NULLP(p))
                {
                    b = St_SetCons(p, b);
                }
            }
            return find_free(body, b);
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

        // TODO: let
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

                StObject v = St_MakeVectorFromList(ST_CDR(x));
                StObject nx = St_Apply(ST_MACRO_PROC(o), &(StCallInfo){ ST_VECTOR(v), St_VectorLength(v), St_VectorLength(v) });
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

static StObject compile_and(StCompileContext *ctx, StObject xs, StObject next)
{
    if (ST_NULLP(xs))
    {
        return ST_LIST3(I("constant"), True, next);
    }

    if (ST_NULLP(ST_CDR(xs)))
    {
        return compile(ctx, ST_CAR(xs), next);
    }

    return compile(ctx, ST_CAR(xs), ST_LIST3(I("test"), compile_and(ctx, ST_CDR(xs), next), next));
}

static StObject compile_or(StCompileContext *ctx, StObject xs, StObject next)
{
    if (ST_NULLP(xs))
    {
        return ST_LIST3(I("constant"), False, next);
    }

    if (ST_NULLP(ST_CDR(xs)))
    {
        return compile(ctx, ST_CAR(xs), next);
    }

    return compile(ctx, ST_CAR(xs), ST_LIST3(I("test"), next, compile_or(ctx, ST_CDR(xs), next)));
}

static StObject compile(StCompileContext *ctx, StObject x, StObject next)
{
    if (ST_SYMBOLP(x))
    {
        return compile_refer(ctx, x, St_SetMemberP(x, ctx->sets) ? ST_LIST2(I("indirect"), next) : next);
    }

    if (ST_PAIRP(x))
    {
        StObject car = ST_CAR(x);

        if (car == I("quote"))
        {
            StObject obj = ST_CADR(x);

            return ST_LIST3(I("constant"), obj, next);
        }

        if (car == I("---let---") ||
            car == I("---let*---"))
        {
            int len = St_Length(x);
            if (len < 2)
            {
                St_Error("compile: malformed let");
            }

            // Design decision: Set refer-local's offset negative values to point variables made by let.
            // Because f register keeps its value when executing let body and binding expressions.
            //
            // (let ((s1 x1) (s2 x2)) body)
            // => (x1 (argument (x2 (argument (body (shift 0 2 next))))))
            //
            //  s2 <-    refer-local -2
            //  s1 <- f  refer-local -1
            //  a1 <-    refer-local 0
            //  a2 <-    refer-local 1
            //  a3 <-    refer-local 2
            //  frame
            //  ------

            StObject bindings = ST_CADR(x);
            StObject body = ST_CDDR(x);

            StObject vars = Nil, vt = Nil;
            StObject exprs = Nil;

            ST_FOREACH(p, bindings) {
                ST_BIND2("let binding", ST_CAR(p), s, exp);

                if (!ST_SYMBOLP(s))
                {
                    St_Error("let bining: symbol reuqired");
                }

                if (St_SetMemberP(s, vars))
                {
                    St_Error("let binding: multiple symbol: %s", ST_SYMBOL_VALUE(s));
                }

                ST_APPEND1(vars, vt, s);
                exprs = St_Cons(exp, exprs);
            }

            StCompileContext nctx = *ctx;
            nctx.env = St_Cons(St_SetAppend(vars, ST_CAR(ctx->env)), ST_CDR(ctx->env));
            nctx.sets = find_sets(body, vars);
            nctx.stackoffset = St_Length(vars);
            StObject nnext = ST_LIST4(I("shift"), St_Integer(0), St_Integer(St_Length(vars)), next);
            StObject c = make_boxes(nctx.sets, vars, compile_body(&nctx, body, nnext), 0);

            ST_FOREACH(p, exprs) {
                c = compile(ctx, ST_CAR(p), ST_LIST2(I("argument"), c));
            }

            return c;
        }

        if (car == I("lambda"))
        {
            StObject params = ST_CADR(x);
            StObject body = ST_CDDR(x);
            StObject known_vars = St_SetUnion(primitiveSyntaxes(), St_ModuleSymbols(ctx->module));

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
            StObject free = find_free(body, St_SetUnion(extended_vars, known_vars));

            // Top-level defined functions must have no free variables because
            // free variables point to stack allocated variable.
            // Otherwise lambdas capture module functions.
            if (ctx->toplevel)
            {
                ST_FOREACH(p, free) {
                    module_add(ctx->module, ST_CAR(p));
                }
                free = Nil;
            }

            StObject sets = find_sets(body, vars);

            int len_vars = St_Length(extended_vars);

            StCompileContext nctx = *ctx;

            nctx.env = St_Cons(extended_vars, free);
            nctx.sets = St_SetUnion(sets,
                                    St_SetUnion(defs,
                                                St_SetIntersect(ctx->sets, free)));
            nctx.toplevel = false;
            nctx.stackoffset = 0;

            StObject body_c = compile_body(&nctx, body, ST_LIST2(I("return"), St_Integer(len_vars)));

            if (abs(arity) != len_vars)
            {
                int to_extend = len_vars - abs(arity);
                body_c = ST_LIST3(I("extend"), St_Integer(to_extend), body_c);
            }

            return collect_free(ctx, free,
                                ST_LIST5(I("close"),
                                         St_Integer(arity),
                                         St_Integer(St_Length(free)),
                                         make_boxes(sets, vars, body_c, 0),
                                         next));
        }

        if (car == I("begin"))
        {
            return compile_body(ctx, ST_CDR(x), next);
        }

        if (car == I("if"))
        {
            StObject testE = ST_CADR(x);
            StObject thenE = ST_CADDR(x);
            StObject elseE = ST_CDR(ST_CDDR(x));

            StObject thenC = compile(ctx, thenE, next);
            StObject elseC = next;

            if (!ST_NULLP(elseE))
            {
                elseC = compile(ctx, ST_CAR(elseE), next);
            }

            return compile(ctx, testE, ST_LIST3(I("test"), thenC, elseC));
        }

        if (car == I("set!"))
        {
            StObject var = ST_CADR(x);
            StObject x2 = ST_CADDR(x);

            return compile(ctx, x2, compile_assign(ctx, var, next));
        }

        if (car == I("call/cc"))
        {
            StObject x2 = ST_CADR(x);
            return ST_LIST3(I("frame"),
                            next,
                            ST_LIST2(I("conti"),
                                     ST_LIST2(I("argument"),
                                              compile(ctx, x2, tailP(next)
                                                      ? ST_LIST4(I("shift"), St_Integer(1), ST_CADR(next), ST_LIST1(I("apply")))
                                                      : ST_LIST1(I("apply"))))));
        }

        if (car == I("define"))
        {
            int len = St_Length(x);
            if (len < 3)
            {
                St_Error("define: malformed define");
            }

            StObject var = ST_CADR(x);
            StObject v = ST_CADDR(x);

            return compile(ctx, v, compile_assign(ctx, var, next));
        }

        if (car == I("define-macro"))
        {
            StObject var = ST_CADR(x);
            StObject v = ST_CADDR(x);

            return compile(ctx, v, ST_LIST3(I("macro"), var, compile_assign(ctx, var, next)));
        }

        if (car == I("and"))
        {
            return compile_and(ctx, ST_CDR(x), next);
        }

        if (car == I("or"))
        {
            return compile_or(ctx, ST_CDR(x), next);
        }

        // else clause
        for (StObject args = ST_CDR(x), c = compile(ctx, ST_CAR(x), tailP(next) ? ST_LIST4(I("shift"), St_Integer(St_Length(ST_CDR(x))), ST_CADR(next), ST_LIST1(I("apply"))) : ST_LIST1(I("apply")));
             ;
             c = compile(ctx, ST_CAR(args), ST_LIST2(I("argument"), c)), args = ST_CDR(args))
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
    return compile(&(StCompileContext){ module, St_Cons(Nil, Nil), Nil, true, 0 }, syntaxexpand(module, macroexpand(module, expr)), next);
}

StObject St_MacroExpand(StObject module, StObject expr)
{
    return macroexpand(module, expr);
}

StObject St_SyntaxExpand(StObject module, StObject expr)
{
    return syntaxexpand(module, expr);
}
