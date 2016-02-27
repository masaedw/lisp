#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "lisp.h"

StObject GlobalModule = Nil;

void St_Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char *rptr;
    vasprintf(&rptr, fmt, args);

    va_end(args);

    St_WriteCString(rptr, St_CurrentErrorPort);

    exit(1);
}

void *St_Alloc2(int type, size_t size)
{
    struct StObjectHeader *obj = St_Malloc(size);
    obj->type = type;
    return obj;
}

StObject St_Cons(StObject car, StObject cdr)
{
    StObject cell = St_Alloc2(TCELL, sizeof(struct StCellRec));

    ST_CAR_SET(cell, car);
    ST_CDR_SET(cell, cdr);

    return cell;
}

StObject St_Acons(StObject key, StObject val, StObject cdr)
{
    return St_Cons(St_Cons(key, val), cdr);
}

int St_Length(StObject list)
{
    int length = 0;

    ST_FOREACH(p, list) {
        length++;
    }

    return length;
}

StObject St_Reverse(StObject list)
{
    if (!ST_PAIRP(list))
    {
        return Nil;
    }

    StObject p = list;
    StObject r = St_Cons(ST_CAR(list), Nil);

    while (ST_PAIRP(ST_CDR(p))) {
        p = ST_CDR(p);
        r = St_Cons(ST_CAR(p), r);
    }

    return r;
}

bool St_ListP(StObject maybe_list)
{
    if (ST_NULLP(maybe_list))
    {
        return true;
    }

    if (!ST_PAIRP(maybe_list))
    {
        return false;
    }

    StObject p = maybe_list;

    while (ST_PAIRP(ST_CDR(p)))
    {
        p = ST_CDR(p);
    }

    return ST_NULLP(ST_CDR(p));
}

bool St_DottedListP(StObject maybe_list)
{
    if (ST_NULLP(maybe_list))
    {
        return false;
    }

    if (!ST_PAIRP(maybe_list))
    {
        return true;
    }

    StObject p = maybe_list;

    while (ST_PAIRP(ST_CDR(p))) {
        p = ST_CDR(p);
    }

    return !ST_NULLP(ST_CDR(p));
}

StObject St_Assq(StObject obj, StObject alist)
{
    ST_FOREACH(p, alist) {
        StObject pair = ST_CAR(p);
        if (!ST_PAIRP(pair))
        {
            return False;
        }
        if (ST_CAR(pair) == obj)
        {
            return pair;
        }
    }
    return False;
}

StObject St_Assv(StObject obj, StObject alist)
{
    ST_FOREACH(p, alist) {
        StObject pair = ST_CAR(p);
        if (!ST_PAIRP(pair))
        {
            return False;
        }
        if (St_EqvP(ST_CAR(pair), obj))
        {
            return pair;
        }
    }
    return False;
}

bool St_SetMemberP(StObject obj, StObject s)
{
    ST_FOREACH(p, s) {
        if (obj == ST_CAR(p))
        {
            return true;
        }
    }
    return false;
}

StObject St_SetCons(StObject obj, StObject s)
{
    return St_SetMemberP(obj, s)
        ? s
        : St_Cons(obj, s);
}

StObject St_SetUnion(StObject s1, StObject s2)
{
    ST_FOREACH(p, s1) {
        s2 = St_SetCons(ST_CAR(p), s2);
    }
    return s2;
}

StObject St_SetMinus(StObject s1, StObject s2)
{
    ST_FOREACH(p, s1) {
        if (!St_SetMemberP(ST_CAR(p), s2))
        {
            return St_Cons(ST_CAR(p), St_SetMinus(ST_CDR(p), s2));
        }
    }
    return Nil;
}

StObject St_SetIntersect(StObject s1, StObject s2)
{
    ST_FOREACH(p, s1) {
        if (St_SetMemberP(ST_CAR(p), s2)) {
            return St_Cons(ST_CAR(p), St_SetIntersect(ST_CDR(p), s2));
        }
    }
    return Nil;
}

bool St_EqvP(StObject lhs, StObject rhs)
{
    if (ST_INTP(lhs) && ST_INTP(rhs))
    {
        return ST_INT_VALUE(lhs) == ST_INT_VALUE(rhs);
    }

    return lhs == rhs;
}

bool St_EqualP(StObject lhs, StObject rhs)
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

    if (ST_BYTEVECTORP(lhs) && ST_BYTEVECTORP(rhs))
    {
        return St_BytevectorEqualP(lhs, rhs);
    }

    return St_EqvP(lhs, rhs);
}

void St_Load(const char* filename)
{
    StObject expr = Nil;
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        St_Error("can't open: %s", filename);
    }

    StObject input = St_MakeFdPort(fd, true);

    while (true) {
        expr = St_Read(input);
        if (ST_EOFP(expr))
        {
            return;
        }
        St_Eval_VM(GlobalModule, expr);
    }
}

StObject St_MakeEmptyString(int len)
{
    StObject o = St_Alloc2(TSTRING, sizeof(struct StStringRec) + len);
    ST_STRING_LENGTH(o) = len;
    return o;
}

StObject St_MakeString(int len, char *buf)
{
    StObject o = St_MakeEmptyString(len);
    memcpy(ST_STRING_VALUE(o), buf, len);

    return o;
}

size_t St_StringLength(StObject s)
{
    return ST_STRING_LENGTH(s);
}

bool St_StringEqualP(StObject s1, StObject s2)
{
    if (ST_STRING_LENGTH(s1) != ST_STRING_LENGTH(s2))
    {
        return false;
    }

    size_t len = ST_STRING_LENGTH(s1);

    return memcmp(ST_STRING_VALUE(s1), ST_STRING_VALUE(s2), len) == 0;
}

StObject St_StringAppend(StObject list)
{
    size_t newlen = 0;

    ST_FOREACH(p, list) {
        newlen += ST_STRING_LENGTH(ST_CAR(p));
    }

    StObject o = St_MakeEmptyString(newlen);
    int offset = 0;

    ST_FOREACH(p, list) {
        int len = ST_STRING_LENGTH(ST_CAR(p));
        memcpy(ST_STRING_VALUE(o) + offset, ST_STRING_VALUE(ST_CAR(p)), len);
        offset += len;
    }

    return o;
}

char *St_StringGetCString(StObject string)
{
    size_t len = ST_STRING_LENGTH(string);
    char *buf = St_Malloc(len + 1);
    memcpy(buf, ST_STRING_VALUE(string), len);
    buf[len] = 0;
    return buf;
}

StObject St_MakeVector(int size)
{
    if (size < 0)
    {
        St_Error("make-vector: size must be a positive integer");
    }

    StObject o = St_Alloc2(TVECTOR, sizeof(struct StVectorRec) + (sizeof(StObject) * size));
    ST_VECTOR_LENGTH(o) = size;

    for (int i = 0; i < size; i++) {
        ST_VECTOR_DATA(o)[i] = Nil;
    }

    return o;
}

void St_CopyVector(StObject dst, StObject src, int size)
{
    memcpy(ST_VECTOR_DATA(dst), ST_VECTOR_DATA(src), sizeof(StObject) * size);
}

StObject St_VectorRef(StObject v, int idx)
{
    if (idx < 0 || ST_VECTOR_LENGTH(v) <= (unsigned int)idx)
    {
        St_Error("vector-ref: index out of range: %d against %d", idx, ST_VECTOR_LENGTH(v));
    }

    return ST_VECTOR_DATA(v)[idx];
}

void St_VectorSet(StObject v, int idx, StObject obj)
{
    if (idx < 0 || ST_VECTOR_LENGTH(v) <= (unsigned int)idx)
    {
        St_Error("vector-set: index out of range");
    }

    ST_VECTOR_DATA(v)[idx] = obj;
}

size_t St_VectorLength(StObject v)
{
    return ST_VECTOR_LENGTH(v);
}

// dynamic array

StObject St_MakeDVector(int size, int capa)
{
    if (capa <= 0)
    {
        capa = 4;
    }
    return St_Cons(St_Integer(size), St_MakeVector(capa));
}

StObject St_DVectorRef(StObject vector, int idx)
{
    int len = St_DVectorLength(vector);
    if (idx < 0 || len <= idx)
    {
        St_Error("dvector-ref: index out of range %d against %d", idx, len);
    }

    return St_VectorRef(St_DVectorData(vector), idx);
}

void St_DVectorSet(StObject vector, int idx, StObject obj)
{
    int len = St_DVectorLength(vector);
    if (idx < 0 || len <= idx)
    {
        St_Error("dvector-set: index out of range %d against %d", idx, len);
    }

    St_VectorSet(St_DVectorData(vector), idx, obj);
}

int St_DVectorLength(StObject vector)
{
    return ST_INT_VALUE(ST_CAR(vector));
}

StObject St_DVectorData(StObject vector)
{
   return ST_CDR(vector);
}

int St_DVectorCapacity(StObject vector)
{
    return St_VectorLength(St_DVectorData(vector));
}

int St_DVectorPush(StObject vector, StObject obj)
{
    int len = St_DVectorLength(vector);
    int capa = St_DVectorCapacity(vector);

    if (len == capa)
    {
        StObject data = St_MakeVector(capa * 1.4);
        St_CopyVector(data, ST_CDR(vector), capa);
        ST_CDR_SET(vector, data);
    }

    St_VectorSet(St_DVectorData(vector), len, obj);
    ST_CAR_SET(vector, St_Integer(len + 1));

    return len;
}

StObject St_MakeModule(StObject alist)
{
    int size = St_Length(alist);
    StObject v = St_MakeDVector(size, size);

    int i = 0;
    ST_FOREACH(p, alist) {
        St_DVectorSet(v, i++, ST_CAR(p));
    }

    return v;
}

#define NOT_FOUND (-1)

static int module_contains(StObject m, StObject sym)
{
    int size = St_DVectorLength(m);
    for (int i = 0; i < size; i++) {
        StObject pair = St_DVectorRef(m, i);
        if (ST_CAR(pair) == sym)
        {
            return i;
        }
    }
    return NOT_FOUND;
}

StObject St_ModuleFind(StObject m, StObject sym)
{
    int i = module_contains(m, sym);
    return i == NOT_FOUND
        ? Unbound
        : ST_CDR(St_DVectorRef(m, i));
}

int St_ModuleFindOrInitialize(StObject m, StObject sym, StObject init)
{
    int i = module_contains(m, sym);
    return i == NOT_FOUND
        ? St_DVectorPush(m, St_Cons(sym, init))
        : i;
}

void St_ModulePush(StObject m, StObject sym, StObject value)
{
    St_DVectorPush(m, St_Cons(sym, value));
}

void St_ModuleSet(StObject m, int idx, StObject val)
{
    ST_CDR_SET(St_DVectorRef(m, idx), val);
}

StObject St_ModuleRef(StObject m, int i)
{
    return St_DVectorRef(m, i);
}

StObject St_ModuleSymbols(StObject m)
{
    int len = St_DVectorLength(m);
    StObject syms = Nil;

    for (int i = 0; i < len; i++) {
        syms = St_Cons(ST_CAR(St_DVectorRef(m, i)), syms);
    }

    return syms;
}

void St_InitModule()
{
    GlobalModule = St_MakeModule(Nil);
}

void St_AddSyntax(StObject module, const char *key, StSyntaxFunction syntax)
{
    StObject s = St_Alloc2(TSYNTAX, sizeof(struct StSyntaxRec));
    ST_SYNTAX_BODY(s) = syntax;
    ST_SYNTAX_NAME(s) = key;

    St_ModulePush(module, St_Intern(key), s);
}

void St_AddSubr(StObject module, const char *key, StSubrFunction subr)
{
    StObject s = St_Alloc2(TSUBR, sizeof(struct StSubrRec));
    ST_SUBR_BODY(s) = subr;
    ST_SUBR_NAME(s) = key;

    St_ModulePush(module, St_Intern(key), s);
}
