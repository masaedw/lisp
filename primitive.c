#include "lisp.h"

static Object *syntax_if(Object *env, Object *form)
{
    // TODO: form validation

    // (if cond then else)

    Object *cond = form->cdr->car;
    Object *then_block = form->cdr->cdr->car;
    Object *else_block = Nil;

    if (ST_PAIRP(form->cdr->cdr->cdr))
    {
        else_block = form->cdr->cdr->cdr->car;
    }

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
            r = r && ST_SYMBOLP(p->car);
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

static Object *syntax_define_macro(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 2)
    {
        St_Error("define-macro: malformed define-macro");
    }

    Object *sym = form->cdr->car;
    Object *proc = St_Eval(env, form->cdr->cdr->car);

    if (!ST_SYMBOLP(sym))
    {
        St_Error("define-macro: needs a symbol");
    }

    if (!ST_LAMBDAP(proc))
    {
        St_Error("define-macro: needs a lambda");
    }

    Object *macro = St_Alloc(TMACRO);
    macro->proc = proc;
    macro->macro_symbol = sym;

    St_AddVariable(env, sym, macro);

    return sym;
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

    Object *o = St_Alloc(TINT);
    o->int_value = lhs->int_value + rhs->int_value;

    return o;
}

static Object *subr_minus(Object *env, Object *args)
{
    int len = St_Length(args);

    if (len < 1 || 2 < len)
    {
        St_Error("-: wrong number of arguments");
    }

    if (len == 1)
    {
        Object *operand = args->car;
        if (!ST_INTP(operand))
        {
            St_Error("-: invalid type");
        }

        Object *o = St_Alloc(TINT);
        o->int_value = -operand->int_value;

        return o;
    }

    // len == 2

    Object *lhs = args->car;
    Object *rhs = args->cdr->car;

    if (!ST_INTP(lhs) || !ST_INTP(rhs))
    {
        St_Error("-: invalid type");
    }

    Object *o = St_Alloc(TINT);
    o->int_value = lhs->int_value - rhs->int_value;

    return o;
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

static Object *subr_eqp(Object *env, Object *args)
{
    ST_ARGS2("eq?", args, lhs, rhs);

    return ST_BOOLEAN(lhs == rhs);
}

static Object *subr_eqvp(Object *env, Object *args)
{
    ST_ARGS2("eqv?", args, lhs, rhs);

    if (ST_INTP(lhs) && ST_INTP(rhs))
    {
        return ST_BOOLEAN(lhs->int_value == rhs->int_value);
    }

    return ST_BOOLEAN(lhs == rhs);
}

static Object *subr_nullp(Object *env, Object *args)
{
    ST_ARGS1("null?", args, o);

    return ST_BOOLEAN(ST_NULLP(o));
}

static Object *subr_pairp(Object *env, Object *args)
{
    ST_ARGS1("pair?", args, o);

    return ST_BOOLEAN(ST_PAIRP(o));
}

static Object *subr_symbolp(Object *env, Object *args)
{
    ST_ARGS1("symbol?", args, o);

    return ST_BOOLEAN(ST_SYMBOLP(o));
}

static Object *subr_cons(Object *env, Object *args)
{
    ST_ARGS2("cons", args, car, cdr);

    return St_Cons(car, cdr);
}

static Object *subr_car(Object *env, Object *args)
{
    ST_ARGS1("car", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("car: pair required");
    }

    return cell->car;
}

static Object *subr_cdr(Object *env, Object *args)
{
    ST_ARGS1("cdr", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("cdr: pair required");
    }

    return cell->cdr;
}

static Object *subr_list(Object *env, Object *args)
{
    return args;
}

static Object *subr_length(Object *env, Object *args)
{
    ST_ARGS1("length", args, list);

    if (!St_Listp(list))
    {
        St_Error("length: must a list");
    }

    Object *o = St_Alloc(TINT);
    o->int_value = St_Length(list);
    return o;
}

static Object *subr_listp(Object *env, Object *args)
{
    ST_ARGS1("list?", args, o);

    return ST_BOOLEAN(St_Listp(o));
}

void St_InitPrimitives(Object *env)
{
    St_AddSyntax(env, "if", syntax_if);
    St_AddSyntax(env, "define", syntax_define);
    St_AddSyntax(env, "quote", syntax_quote);
    St_AddSyntax(env, "set!", syntax_set);
    St_AddSyntax(env, "lambda", syntax_lambda);
    St_AddSyntax(env, "call/cc", syntax_call_cc);
    St_AddSyntax(env, "define-macro", syntax_define_macro);
    St_AddSubr(env, "+", subr_plus);
    St_AddSubr(env, "-", subr_minus);
    St_AddSubr(env, "*", subr_mul);
    St_AddSubr(env, "/", subr_div);
    St_AddSubr(env, "<", subr_lt);
    St_AddSubr(env, "<=", subr_le);
    St_AddSubr(env, ">", subr_gt);
    St_AddSubr(env, ">=", subr_ge);
    St_AddSubr(env, "=", subr_numeric_eq);
    //St_AddSubr(env, "number?", subr_numberp);
    //St_AddSubr(env, "integer?", subr_integerp);
    //St_AddSubr(env, "zero?", subr_zerop);
    //St_AddSubr(env, "positive?", subr_positivep);
    //St_AddSubr(env, "negative?", subr_negativep);
    //St_AddSubr(env, "odd?", subr_oddp);
    //St_AddSubr(env, "even?", subr_evenp);
    St_AddSubr(env, "print", subr_print);
    St_AddSubr(env, "newline", subr_newline);
    St_AddSubr(env, "eq?", subr_eqp);
    St_AddSubr(env, "eqv?", subr_eqvp);
    //St_AddSubr(env, "equal?", subr_equalp);
    St_AddSubr(env, "null?", subr_nullp);
    St_AddSubr(env, "pair?", subr_pairp);
    St_AddSubr(env, "symbol?", subr_symbolp);
    St_AddSubr(env, "cons", subr_cons);
    St_AddSubr(env, "car", subr_car);
    St_AddSubr(env, "cdr", subr_cdr);
    St_AddSubr(env, "list", subr_list);
    St_AddSubr(env, "length", subr_length);
    St_AddSubr(env, "list?", subr_listp);
}
