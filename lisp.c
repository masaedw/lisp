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

    St_AddVariable(env, St_Intern(key), s);
}

void St_AddSubr(Object *env, const char *key, SubrFunction *subr)
{
    Object *s = St_Alloc(TSUBR);
    s->subr = subr;

    St_AddVariable(env, St_Intern(key), s);
}

Object *St_PushEnv(Object *env, Object *keys, Object *values)
{
    return Nil; // TODO
}

Object *St_LookupVariable(Object *env, Object *key)
{
    if (ST_NULLP(env))
    {
        return Nil;
    }

    for (Object *p = env->cdr; !ST_NULLP(p); p = p->cdr) {
        Object *symbol = p->car->car;
        Object *value = p->car->cdr;

        if (symbol == key)
        {
            return value;
        }
    }

    // go to upper level
    return St_LookupVariable(env->car, key);
}
