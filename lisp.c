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

bool St_Listp(Object *maybe_list)
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

static Object *zip(Object *a, Object *b)
{
    Object *head = Nil;
    Object *tail = Nil;

    for (Object *p = a, *q = b; !ST_NULLP(p) && !ST_NULLP(q); p = p->cdr, q = q->cdr) {
        ST_APPEND1(head, tail, St_Cons(p->car, q->car));
    }

    return head;
}

Object *St_PushEnv(Object *env, Object *keys, Object *values)
{
    Object *new_env = St_Alloc(TCELL);
    new_env->car = env;
    new_env->cdr = zip(keys, values);

    return new_env;
}

Object *St_LookupVariable(Object *env, Object *key)
{
    if (ST_NULLP(env))
    {
        St_Error("unbound variable");
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
