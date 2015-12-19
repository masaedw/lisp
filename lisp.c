#include "lisp.h"

Object *Nil = &(Object) { TNIL };
Object *True = &(Object) { TTRUE };
Object *False = &(Object) { TFALSE };

Object *St_Alloc(int type)
{
    Object *obj = (Object *)St_Malloc(sizeof(Object));

    obj->type = type;

    return obj;
}

Object *St_Cons(Object *car, Object *cdr)
{
    Object *cell = St_Alloc(TCELL);

    cell->car = car;
    cell->cdr = cdr;

    return cell;
}

Object *St_Acons(Object *key, Object *val, Object *cdr)
{
    return St_Cons(St_Cons(key, val), cdr);
}

int St_Length(Object *list)
{
    int length = 0;

    for (; !ST_NULLP(list); list = list->cdr)
    {
        length++;
    }

    return length;
}

Object *St_Reverse(Object *list)
{
    if (!ST_PAIRP(list))
    {
        return Nil;
    }

    Object *p = list;
    Object *r = St_Cons(list->car, Nil);

    while (ST_PAIRP(p->cdr)) {
        p = p->cdr;
        r = St_Cons(p->car, r);
    }

    return r;
}

bool St_ListP(Object *maybe_list)
{
    if (!ST_PAIRP(maybe_list))
    {
        return false;
    }

    Object *p = maybe_list;

    while (ST_PAIRP(ST_CDR(p)))
    {
        p = p->cdr;
    }

    return ST_NULLP(ST_CDR(p));
}

bool St_EqvP(Object *lhs, Object *rhs)
{
    if (ST_INTP(lhs) && ST_INTP(rhs))
    {
        return lhs->int_value == rhs->int_value;
    }

    return lhs == rhs;
}

bool St_EqualP(Object *lhs, Object *rhs)
{
    if (ST_PAIRP(lhs) && ST_PAIRP(rhs))
    {
        return St_EqualP(ST_CAR(lhs), ST_CAR(rhs)) &&
            St_EqualP(ST_CDR(lhs), ST_CDR(rhs));
    }

    return St_EqvP(lhs, rhs);
}

// Environment structure
// (<upper level env> . <variable alist>)

Object *St_InitEnv()
{
    return St_Cons(Nil, Nil);
}

void St_AddVariable(Object *env, Object *key, Object *value)
{
    env->cdr = St_Acons(key, value, env->cdr);
}

void St_AddSyntax(Object *env, const char *key, SyntaxFunction *syntax)
{
    Object *s = St_Alloc(TSYNTAX);
    s->syntax = syntax;
    s->syntax_name = key;

    St_AddVariable(env, St_Intern(key), s);
}

void St_AddSubr(Object *env, const char *key, SubrFunction *subr)
{
    Object *s = St_Alloc(TSUBR);
    s->subr = subr;
    s->subr_name = key;

    St_AddVariable(env, St_Intern(key), s);
}

Object *St_PushEnv(Object *env, Object *keys, Object *values)
{
    Object *new_env = St_Alloc(TCELL);
    new_env->car = env;

    Object *p = keys;
    Object *v = values;
    Object *vars = Nil;
    Object *tail = Nil;

    if (ST_SYMBOLP(p))
    {
        new_env->cdr = St_Acons(p, values, Nil);
        return new_env;
    }

    for (; !ST_NULLP(p) && !ST_NULLP(v); p = p->cdr, v = v->cdr)
    {
        ST_APPEND1(vars, tail, St_Cons(p->car, v->car));

        if (!ST_PAIRP(p->cdr) && !ST_NULLP(p->cdr))
        {
            ST_APPEND1(vars, tail, St_Cons(p->cdr, v->cdr));
            break;
        }
    }

    new_env->cdr = vars;

    return new_env;
}

Object *St_LookupVariablePair(Object *env, Object *key)
{
    if (ST_NULLP(env))
    {
        return Nil;
    }

    for (Object *p = ST_CDR(env); !ST_NULLP(p); p = ST_CDR(p)) {
        Object *symbol = ST_CAAR(p);

        if (symbol == key)
        {
            return ST_CAR(p);
        }
    }

    // go to upper level
    return St_LookupVariablePair(ST_CAR(env), key);
}

Object *St_LookupVariable(Object *env, Object *key)
{
    Object *pair = St_LookupVariablePair(env, key);

    if (ST_NULLP(pair))
    {
        St_Error("unbound variable");
    }

    return ST_CDR(pair);
}
