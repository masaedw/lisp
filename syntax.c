#include "lisp.h"

#define I(x) St_Intern(x)

static void validate_bindings(StObject args)
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
        StObject b = ST_CAR(p);
        if (!ST_PAIRP(b) || St_Length(b) != 2 || !ST_SYMBOLP(ST_CAR(b)))
        {
            St_Error("let: malformed binding");
        }
    }
}

static StObject syntax_let(StObject module, StObject expr)
{
    // (let <bindings> <body>)
    // <bindings> ::= ((sym <expr>)*)
    // =>
    // ((lambda (s1 s2 s3 ...) body) e1 e2 e3 ...)

    if (St_Length(expr) < 2)
    {
        St_Error("let: malformed let");
    }

    StObject bindings = ST_CADR(expr);
    StObject body = ST_CDDR(expr);

    validate_bindings(bindings);

    StObject syms = Nil, symst = Nil;
    StObject vals = Nil, valst = Nil;

    ST_FOREACH(p, bindings) {
        ST_APPEND1(syms, symst, ST_CAAR(p));
        ST_APPEND1(vals, valst, ST_CADR(ST_CAR(p)));
    }

    StObject lambda = St_Cons(I("lambda"), St_Cons(syms, body));
    StObject ret = St_Cons(lambda, vals);

    return St_SyntaxExpand(module, ret);
}

static StObject syntax_let1(StObject module, StObject expr)
{
    if (St_Length(expr) < 3)
    {
        St_Error("let1: malformed let1");
    }

    if (!ST_SYMBOLP(ST_CADR(expr)))
    {
        St_Error("let1: malformed let1, symbol requrired");
    }

    StObject sym = ST_CADR(expr);
    StObject val = ST_CADDR(expr);
    StObject body = ST_CDR(ST_CDDR(expr));
    StObject ret = St_Cons(I("let"), St_Cons(ST_LIST1(ST_LIST2(sym, val)), body));

    return St_SyntaxExpand(module, ret);
}

static StObject syntax_define(StObject module, StObject expr)
{
    if (St_Length(expr) < 3)
    {
        St_Error("define: malformed define");
    }

    if (ST_PAIRP(ST_CADR(expr)))
    {
        StObject sym = ST_CAR(ST_CADR(expr));
        StObject vars = ST_CDR(ST_CADR(expr));
        StObject body = ST_CDDR(expr);
        StObject lambda = St_Cons(I("lambda"), St_Cons(vars, body));

        StObject ret = St_Cons(I("define"),
                               St_Cons(St_SyntaxExpand(module, sym),
                                       ST_LIST1(St_SyntaxExpand(module, lambda))));
        return ret;
    }

    return St_Cons(ST_CAR(expr), St_SyntaxExpand(module, ST_CDR(expr)));
}

static StObject cond_expand(StObject module, StObject expr)
{
    if (ST_NULLP(ST_CAR(expr)))
    {
        return Nil;
    }

    if (ST_PAIRP(ST_CAR(expr)))
    {
        StObject pred = ST_CAAR(expr);
        StObject body = ST_CDAR(expr);

        if (pred != I("else"))
        {
            return ST_LIST4(I("if"), St_SyntaxExpand(module, pred),
                            St_SyntaxExpand(module, St_Cons(I("begin"), body)),
                            cond_expand(module, ST_CDR(expr)));
        }
        else
        {
             return St_SyntaxExpand(module, St_Cons(I("begin"), body));
        }
    }

    return Nil;
}

static StObject syntax_cond(StObject module, StObject expr)
{
    return cond_expand(module, ST_CDR(expr));
}

void St_InitSyntax()
{
    StObject m = GlobalModule;

    St_AddSyntax(m, "let", syntax_let);
    St_AddSyntax(m, "let1", syntax_let1);
    St_AddSyntax(m, "define", syntax_define);
    St_AddSyntax(m, "cond", syntax_cond);
}
