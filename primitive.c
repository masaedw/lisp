#include "lisp.h"

static Object *syntax_if(Object *env, Object *form)
{
    // TODO: form validation

    // (if cond then else)

    Object *cond = form->cdr->car;
    Object *then_block = form->cdr->cdr->car;
    Object *else_block = form->cdr->cdr->cdr->car;

    Object *result = St_Eval(env, cond);

    if (!ST_FALSEP(result))
    {
        return St_Eval(env, then_block);
    }
    else
    {
        return St_Eval(env, else_block);
    }
}

static Object *syntax_define(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 2)
    {
        St_Error("define: malformed define");
    }

    Object *sym = form->cdr->car;
    Object *body = form->cdr->cdr->car;

    if (!ST_SYMBOLP(sym))
    {
        St_Error("define: symbol required");
    }

    Object *value = St_Eval(env, body);

    St_AddVariable(env, sym, value);

    return sym;
}

static Object *syntax_quote(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 1)
    {
        St_Error("quote: malformed quote");
    }

    return form->cdr->car;
}

static Object *syntax_set(Object *env, Object *form)
{
    return Nil; // TODO
}

static bool nil_or_symbol_list(Object *obj)
{
    if (ST_NULLP(obj))
    {
        return true;
    }

    if (ST_PAIRP(obj))
    {
        bool r = true;
        for (Object *p = obj; r && !ST_NULLP(p); p = p->cdr) {
            r = r && ST_SYMBOLP(p);
        }
        return r;
    }

    return false;
}

static Object *syntax_lambda(Object *env, Object *form)
{
    // (lambda <params> <body>)
    // <params> ::= (<symbol> *)
    // <body> ::<define> * <expr> +

    int len = St_Length(form->cdr);

    if (len < 2)
    {
        St_Error("lambda: malformed lambda");
    }

    Object *params = form->cdr->car;
    Object *body = form->cdr->cdr;

    if (!nil_or_symbol_list(params))
    {
        St_Error("lambda: params needs to be () or a list of symbols");
    }

    // TODO: validate body

    Object *lambda = St_Alloc(TLAMBDA);
    lambda->params = params;
    lambda->env = env;
    lambda->body = body;

    return lambda;
}

static Object *syntax_call_cc(Object *env, Object *form)
{
    return Nil; // TODO
}

static Object *subr_plus(Object *env, Object *args)
{
    int len = St_Length(args);

    if (len != 2)
    {
        St_Error("+: wrong number of arguments");
    }

    Object *lhs = args->car;
    Object *rhs = args->cdr->car;

    if (!ST_INTP(lhs) || !ST_INTP(rhs))
    {
        St_Error("+: invalid type");
    }

    if (!ST_INTP(lhs))
    {
        St_Error("+: invalid argument");
    }

    Object *o = St_Alloc(TINT);
    o->int_value = lhs->int_value + rhs->int_value;

    return o;
}

static Object *subr_minus(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_mul(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_div(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_lt(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_le(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_gt(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_ge(Object *env, Object *args)
{
    return Nil; // TODO
}

static Object *subr_numeric_eq(Object *env, Object *args)
{
    return Nil; // TOO
}

static Object *subr_print(Object *env, Object *args)
{
    for (Object *p = args; !ST_NULLP(p); p = p->cdr) {
        St_Print(p->car);
    }

    return Nil;
}

static Object *subr_newline(Object *env, Object *args)
{
    fprintf(stdout, "\n");
    return Nil;
}

void St_InitPrimitives(Object *env)
{
    St_AddSyntax(env, "if", syntax_if);
    St_AddSyntax(env, "define", syntax_define);
    St_AddSyntax(env, "quote", syntax_quote);
    St_AddSyntax(env, "set!", syntax_set);
    St_AddSyntax(env, "lambda", syntax_lambda);
    St_AddSyntax(env, "call/cc", syntax_call_cc);
    St_AddSubr(env, "+", subr_plus);
    St_AddSubr(env, "-", subr_minus);
    St_AddSubr(env, "*", subr_mul);
    St_AddSubr(env, "/", subr_div);
    St_AddSubr(env, "<", subr_lt);
    St_AddSubr(env, "<=", subr_le);
    St_AddSubr(env, ">", subr_gt);
    St_AddSubr(env, ">=", subr_ge);
    St_AddSubr(env, "=", subr_numeric_eq);
    St_AddSubr(env, "print", subr_print);
    St_AddSubr(env, "newline", subr_newline);
}
