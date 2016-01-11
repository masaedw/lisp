#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "lisp.h"

Object *Nil = &(Object) { TNIL };
Object *True = &(Object) { TTRUE };
Object *False = &(Object) { TFALSE };

void St_Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    exit(1);
}

Object *St_Alloc(int type, size_t size)
{
    Object *obj = (Object *)St_Malloc(offsetof(Object, int_value) + size);

    obj->type = type;

    return obj;
}

Object *St_Cons(Object *car, Object *cdr)
{
    Object *cell = St_Alloc(TCELL, sizeof(void*) * 2);

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

bool St_SetMemberP(Object *obj, Object *s)
{
    ST_FOREACH(p, s) {
        if (obj == ST_CAR(p))
        {
            return true;
        }
    }
    return false;
}

Object *St_SetCons(Object *obj, Object *s)
{
    return St_SetMemberP(obj, s)
        ? s
        : St_Cons(obj, s);
}

Object *St_SetUnion(Object *s1, Object *s2)
{
    ST_FOREACH(p, s1) {
        s2 = St_SetCons(ST_CAR(p), s2);
    }
    return s2;
}

Object *St_SetMinus(Object *s1, Object *s2)
{
    ST_FOREACH(p, s1) {
        if (!St_SetMemberP(ST_CAR(p), s2))
        {
            return St_Cons(ST_CAR(p), St_SetMinus(ST_CDR(p), s2));
        }
    }
    return Nil;
}

Object *St_SetIntersect(Object *s1, Object *s2)
{
    ST_FOREACH(p, s1) {
        if (St_SetMemberP(ST_CAR(p), s2)) {
            return St_Cons(ST_CAR(p), St_SetIntersect(ST_CDR(p), s2));
        }
    }
    return Nil;
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

Object *St_Integer(int value)
{
    Object *o = St_Alloc(TINT, sizeof(int));
    o->int_value = value;
    return o;
}

Object *St_MakeVector(int size)
{
    if (size < 0)
    {
        St_Error("make-vector: size must be a positive integer");
    }

    Object *o = St_Alloc(TVECTOR, sizeof(void*) * (size + 1));
    o->size = size;

    for (int i = 0; i < size; i++) {
        o->vector[i] = Nil;
    }

    return o;
}

void St_CopyVector(Object *dst, Object *src, int size)
{
    memcpy(dst->vector, src->vector, sizeof(Object*) * size);
}

Object *St_VectorRef(Object *v, int idx)
{
    if (idx < 0 || v->size <= idx)
    {
        St_Error("vector-ref: index out of range: %d against %d", idx, v->size);
    }

    return v->vector[idx];
}

void St_VectorSet(Object *v, int idx, Object *obj)
{
    if (idx < 0 || v->size <= idx)
    {
        St_Error("vector-ref: index out of range");
    }

    v->vector[idx] = obj;
}

int St_VectorLength(Object *v)
{
    return v->size;
}

// Environment structure
// (<upper level env> <variable alist> . <tail cell of variable alist>)

Object *St_InitEnv()
{
    return St_Cons(Nil, St_Cons(Nil, Nil));
}

void St_AddVariable(Object *env, Object *key, Object *value)
{
    Object *head = ST_CADR(env);
    Object *tail = ST_CDDR(env);

    ST_FOREACH(p, head) {
        if (ST_CAAR(p) == key)
        {
            ST_CDR_SET(ST_CAR(p), value);
            return;
        }
    }

    ST_APPEND1(head, tail, St_Cons(key, value));

    ST_CAR_SET(ST_CDR(env), head);
    ST_CDR_SET(ST_CDR(env), tail);
}

void St_AddSyntax(Object *env, const char *key, SyntaxFunction *syntax)
{
    Object *s = St_Alloc(TSYNTAX, sizeof(void*) * 2);
    s->syntax = syntax;
    s->syntax_name = key;

    St_AddVariable(env, St_Intern(key), s);
}

void St_AddSubr(Object *env, const char *key, SubrFunction *subr)
{
    Object *s = St_Alloc(TSUBR, sizeof(void*) * 2);
    s->subr = subr;
    s->subr_name = key;

    St_AddVariable(env, St_Intern(key), s);
}

Object *St_PushEnv(Object *env, Object *keys, Object *values)
{
    Object *new_env = St_Cons(env, St_Cons(Nil, Nil));

    Object *p = keys;
    Object *v = values;

    if (ST_SYMBOLP(p))
    {
        St_AddVariable(new_env, p, values);
        return new_env;
    }

    for (; !ST_NULLP(p) && !ST_NULLP(v); p = ST_CDR(p), v = ST_CDR(v))
    {
        St_AddVariable(new_env, ST_CAR(p), ST_CAR(v));

        if (!ST_PAIRP(ST_CDR(p)) && !ST_NULLP(ST_CDR(p)))
        {
            St_AddVariable(new_env, ST_CDR(p), ST_CDR(v));
            break;
        }
    }

    return new_env;
}

Object *St_LookupVariablePair(Object *env, Object *key)
{
    if (ST_NULLP(env))
    {
        return Nil;
    }

    ST_FOREACH(p, ST_CADR(env)) {
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
