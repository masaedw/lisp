#include "lisp.h"

#define I(x) St_Intern(x)

static Object *syntax_and(Object *expr)
{
    if (ST_NULLP(ST_CDR(expr)))
    {
        return True;
    }

    Object *vals = St_Reverse(ST_CDR(expr));

    Object *ret = Nil;
    Object *slet = I("let");
    Object *sif = I("if");

    ST_FOREACH(p, vals) {
        Object *sym = St_Gensym();
        Object *next = ST_NULLP(ret) ? sym : ret;

        ret = ST_LIST3(slet, ST_LIST1(ST_LIST2(sym, ST_CAR(p))),
                       ST_LIST4(sif, sym, next, False));
    }

    return ret;
}

static void validate_bindings(Object *args)
{
    if (ST_NULLP(args))
    {
        return;
    }

    if (!St_ListP(args))
    {
        St_Error("let: malformed bindings");
    }

    ST_FOREACH(p, args) {
        Object *b = ST_CAR(p);
        if (!ST_PAIRP(b) || St_Length(b) != 2 || !ST_SYMBOLP(ST_CAR(b)))
        {
            St_Error("let: malformed binding");
        }
    }
}


static Object *syntax_let(Object *expr)
{
    // (let <bindings> <body>)
    // <bindings> ::= ((sym <expr>)*)
    // =>
    // ((lambda (s1 s2 s3 ...) body) e1 e2 e3 ...)

    if (St_Length(expr) < 2)
    {
        St_Error("let: malformed let");
    }

    Object *bindings = ST_CADR(expr);
    Object *body = ST_CDDR(expr);

    validate_bindings(bindings);

    Object *syms = Nil, *symst = Nil;
    Object *vals = Nil, *valst = Nil;

    ST_FOREACH(p, bindings) {
        ST_APPEND1(syms, symst, ST_CAAR(p));
        ST_APPEND1(vals, valst, ST_CADR(ST_CAR(p)));
    }

    Object *lambda = St_Cons(I("lambda"), St_Cons(syms, body));
    Object *ret = St_Cons(lambda, vals);

    return ret;
}

void St_InitSyntax(Object *env)
{
    St_AddSyntax(env, "and", syntax_and);
    St_AddSyntax(env, "let", syntax_let);
}
