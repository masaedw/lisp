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

void St_InitPrimitives(Object *env)
{
    St_AddSyntax(env, "if", syntax_if);
    St_AddSubr(env, "+", subr_plus);
}
