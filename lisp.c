#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "lisp.h"

static Object _Nil = { TNIL };
Object *Nil = &_Nil;
Object *True = &(Object) { TTRUE };
Object *False = &(Object) { TFALSE };
Object *Unbound = &(Object) { TUNBOUND };
Object *GlobalModule = &_Nil;

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
    Object *obj = (Object *)St_Malloc(offsetof(Object, integer.value) + size);

    obj->type = type;

    return obj;
}

Object *St_Cons(Object *car, Object *cdr)
{
    Object *cell = St_Alloc(TCELL, sizeof(void*) * 2);

    ST_CAR_SET(cell, car);
    ST_CDR_SET(cell, cdr);

    return cell;
}

Object *St_Acons(Object *key, Object *val, Object *cdr)
{
    return St_Cons(St_Cons(key, val), cdr);
}

int St_Length(Object *list)
{
    int length = 0;

    ST_FOREACH(p, list) {
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
    Object *r = St_Cons(ST_CAR(list), Nil);

    while (ST_PAIRP(ST_CDR(p))) {
        p = ST_CDR(p);
        r = St_Cons(ST_CAR(p), r);
    }

    return r;
}

bool St_ListP(Object *maybe_list)
{
    if (ST_NULLP(maybe_list))
    {
        return true;
    }

    if (!ST_PAIRP(maybe_list))
    {
        return false;
    }

    Object *p = maybe_list;

    while (ST_PAIRP(ST_CDR(p)))
    {
        p = ST_CDR(p);
    }

    return ST_NULLP(ST_CDR(p));
}

bool St_DottedListP(Object *maybe_list)
{
    if (ST_NULLP(maybe_list))
    {
        return false;
    }

    if (!ST_PAIRP(maybe_list))
    {
        return true;
    }

    Object *p = maybe_list;

    while (ST_PAIRP(ST_CDR(p))) {
        p = ST_CDR(p);
    }

    return !ST_NULLP(ST_CDR(p));
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
        return lhs->integer.value == rhs->integer.value;
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

    if (ST_STRINGP(lhs) && ST_STRINGP(rhs))
    {
        return St_StringEqualP(lhs, rhs);
    }

    return St_EqvP(lhs, rhs);
}

Object *St_Integer(int value)
{
    Object *o = St_Alloc(TINT, sizeof(int));
    o->integer.value = value;
    return o;
}

Object *St_MakeEmptyString(int len)
{
    Object *o = St_Alloc(TSTRING, offsetof(Object, string.value) - offsetof(Object, integer.value) + len);
    o->string.len = len;

    return o;
}

Object *St_MakeString(int len, char *buf)
{
    Object *o = St_MakeEmptyString(len);
    memcpy(o->string.value, buf, len);

    return o;
}

int St_StringLength(Object *s)
{
    return s->string.len;
}

bool St_StringEqualP(Object *s1, Object *s2)
{
    if (St_StringLength(s1) != St_StringLength(s2))
    {
        return false;
    }

    int len = St_StringLength(s1);

    return memcmp(s1->string.value, s2->string.value, len) == 0;
}

Object *St_StringAppend(Object *list)
{
    int newlen = 0;

    ST_FOREACH(p, list) {
        newlen += St_StringLength(ST_CAR(p));
    }

    Object *o = St_MakeEmptyString(newlen);
    int offset = 0;

    ST_FOREACH(p, list) {
        int len = St_StringLength(ST_CAR(p));
        memcpy(o->string.value + offset, ST_CAR(p)->string.value, len);
        offset += len;
    }

    return o;
}

Object *St_MakeVector(int size)
{
    if (size < 0)
    {
        St_Error("make-vector: size must be a positive integer");
    }

    Object *o = St_Alloc(TVECTOR, sizeof(void*) * (size + 1));
    o->vector.size = size;

    for (int i = 0; i < size; i++) {
        o->vector.data[i] = Nil;
    }

    return o;
}

void St_CopyVector(Object *dst, Object *src, int size)
{
    memcpy(dst->vector.data, src->vector.data, sizeof(Object*) * size);
}

Object *St_VectorRef(Object *v, int idx)
{
    if (idx < 0 || v->vector.size <= idx)
    {
        St_Error("vector-ref: index out of range: %d against %d", idx, v->vector.size);
    }

    return v->vector.data[idx];
}

void St_VectorSet(Object *v, int idx, Object *obj)
{
    if (idx < 0 || v->vector.size <= idx)
    {
        St_Error("vector-set: index out of range");
    }

    v->vector.data[idx] = obj;
}

int St_VectorLength(Object *v)
{
    return v->vector.size;
}

// dynamic array

Object *St_MakeDVector(int size, int capa)
{
    return St_Cons(St_Integer(size), St_MakeVector(capa));
}

Object *St_DVectorRef(Object *vector, int idx)
{
    int len = St_DVectorLength(vector);
    if (idx < 0 || len <= idx)
    {
        St_Error("dvector-ref: index out of range %d against %d", idx, len);
    }

    return St_VectorRef(St_DVectorData(vector), idx);
}

void St_DVectorSet(Object *vector, int idx, Object *obj)
{
    int len = St_DVectorLength(vector);
    if (idx < 0 || len <= idx)
    {
        St_Error("dvector-set: index out of range %d against %d", idx, len);
    }

    St_VectorSet(St_DVectorData(vector), idx, obj);
}

int St_DVectorLength(Object *vector)
{
    return ST_CAR(vector)->integer.value;
}

Object *St_DVectorData(Object *vector)
{
   return ST_CDR(vector);
}

int St_DVectorCapacity(Object *vector)
{
    return St_VectorLength(St_DVectorData(vector));
}

int St_DVectorPush(Object *vector, Object *obj)
{
    int len = St_DVectorLength(vector);
    int capa = St_DVectorCapacity(vector);

    if (len == capa)
    {
        Object *data = St_MakeVector(capa * 1.4);
        St_CopyVector(data, ST_CDR(vector), capa);
        ST_CDR_SET(vector, data);
    }

    St_VectorSet(St_DVectorData(vector), len, obj);
    ST_CAR_SET(vector, St_Integer(len + 1));

    return len;
}

Object *St_MakeModule(Object *alist)
{
    int size = St_Length(alist);
    Object *v = St_MakeDVector(size, size);

    int i = 0;
    ST_FOREACH(p, alist) {
        St_DVectorSet(v, i++, ST_CAR(p));
    }

    return v;
}

#define NOT_FOUND (-1)

static int module_contains(Object *m, Object *sym)
{
    int size = St_DVectorLength(m);
    for (int i = 0; i < size; i++) {
        Object *pair = St_DVectorRef(m, i);
        if (ST_CAR(pair) == sym)
        {
            return i;
        }
    }
    return NOT_FOUND;
}

Object *St_ModuleFind(Object *m, Object *sym)
{
    int i = module_contains(m, sym);
    return i == NOT_FOUND
        ? Unbound
        : ST_CDR(St_DVectorRef(m, i));
}

int St_ModuleFindOrInitialize(Object *m, Object *sym, Object *init)
{
    int i = module_contains(m, sym);
    return i == NOT_FOUND
        ? St_DVectorPush(m, St_Cons(sym, init))
        : i;
}

void St_ModuleSet(Object *m, int idx, Object *val)
{
    ST_CDR_SET(St_DVectorRef(m, idx), val);
}

Object *St_ModuleRef(Object *m, int i)
{
    return St_DVectorRef(m, i);
}

Object *St_ModuleSymbols(Object *m)
{
    int len = St_DVectorLength(m);
    Object *syms = Nil;

    for (int i = 0; i < len; i++) {
        syms = St_Cons(ST_CAR(St_DVectorRef(m, i)), syms);
    }

    return syms;
}

void St_InitModule(Object *env)
{
    Object *head = Nil;
    Object *tail = Nil;

    for (Object *p = env; !ST_NULLP(p); p = ST_CAR(p)) {
        ST_FOREACH(q, ST_CADR(p)) {
            ST_APPEND1(head, tail, ST_CAR(q));
        }
    }
    GlobalModule = St_MakeModule(head);
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
    s->syntax.body = syntax;
    s->syntax.name = key;

    St_AddVariable(env, St_Intern(key), s);
}

void St_AddSubr(Object *env, const char *key, SubrFunction *subr)
{
    Object *s = St_Alloc(TSUBR, sizeof(void*) * 2);
    s->subr.body = subr;
    s->subr.name = key;

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
        St_Error("unbound variable: %s", key->symbol.value);
    }

    return ST_CDR(pair);
}
