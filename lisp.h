#ifndef LISP_H
#define LISP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <gc.h>

// type tag
enum {
    TINT = 1,
    TCELL,
    TNIL,
    TTRUE,
    TFALSE,
    TUNBOUND,
    TSYMBOL,
    TCHAR,
    TSTRING,
    TVECTOR,
    TSYNTAX,
    TSUBR,
    TLAMBDA,
    TLAMBDAVM,
    TMACRO,
};

typedef struct Object Object;
typedef Object *SyntaxFunction(Object *env, Object *form);
typedef Object *SubrFunction(Object *env, Object *args);

struct Object
{
    int type;

    union {
        struct {
            int value;
        } integer;

        // cell
        struct {
            Object *car;
            Object *cdr;
        } cell;

        struct {
            char value[1];
        } symbol;

        struct {
            int len;
            char value[1];
        } string;

        struct {
            int value;
        } charcter;

        struct {
            int size;
            Object *data[1];
        } vector;

        // syntax
        struct {
            SyntaxFunction *body;
            const char *name;
        } syntax;

        // subr
        struct {
            SubrFunction *body;
            const char *name;
        } subr;

        // lambda
        struct {
            Object *params;
            Object *body;
            Object *env;
        } lambda;

        // lambda_vm
        struct {
            Object *body;
            Object *free;
        } lambda_vm;

        // macro
        struct {
            Object *proc;
            Object *symbol;
        } macro;
    };
};

extern Object *Nil;
extern Object *True;
extern Object *False;
extern Object *Unbound;

void St_Error(const char *fmt, ...) __attribute__((noreturn));

#define St_Malloc GC_MALLOC

Object *St_Alloc(int type, size_t size);

#define ST_NULLP(obj) ((obj) == Nil)
#define ST_TRUEP(obj) ((obj) == True)
#define ST_FALSEP(obj) ((obj) == False)
#define ST_UNBOUNDP(obj) ((obj) == Unbound)
#define ST_PAIRP(obj) ((obj)->type == TCELL)
#define ST_INTP(obj) ((obj)->type == TINT)
#define ST_SYMBOLP(obj) ((obj)->type == TSYMBOL)
#define ST_VECTORP(obj) ((obj)->type == TVECTOR)
#define ST_CHARP(obj) ((obj)->type == TCHAR)
#define ST_STRINGP(obj) ((obj)->type == TSTRING)
#define ST_SYNTAXP(obj) ((obj)->type == TSYNTAX)
#define ST_SUBRP(obj) ((obj)->type == TSUBR)
#define ST_LAMBDAP(obj) ((obj)->type == TLAMBDA || (obj)->type == TLAMBDAVM)
#define ST_MACROP(obj) ((obj)->type == TMACRO)
#define ST_PROCEDUREP(obj) (ST_SUBRP(obj) || ST_LAMBDAP(obj))

// List and Pair

Object *St_Cons(Object *car, Object *cdr);
Object *St_Acons(Object *key, Object *val, Object *cdr);
Object *St_Reverse(Object *list);
int St_Length(Object *list);
bool St_ListP(Object *maybe_list);

#define ST_LIST1(a0)             St_Cons((a0), Nil)
#define ST_LIST2(a0, a1)         St_Cons((a0), ST_LIST1((a1)))
#define ST_LIST3(a0, a1, a2)     St_Cons((a0), ST_LIST2((a1), (a2)))
#define ST_LIST4(a0, a1, a2, a3) St_Cons((a0), ST_LIST3((a1), (a2), (a3)))

#define ST_CAR(pair) (pair)->cell.car
#define ST_CDR(pair) (pair)->cell.cdr
#define ST_CDDR(list) ST_CDR(ST_CDR(list))
#define ST_CDAR(list) ST_CDR(ST_CAR(list))
#define ST_CADR(list) ST_CAR(ST_CDR(list))
#define ST_CAAR(list) ST_CAR(ST_CAR(list))
#define ST_CADDR(list) ST_CAR(ST_CDR(ST_CDR(list)))
#define ST_CADDDR(list) ST_CAR(ST_CDR(ST_CDR(ST_CDR(list))))

#define ST_CAR_SET(pair, value) ((pair)->cell.car = (value))
#define ST_CDR_SET(pair, value) ((pair)->cell.cdr = (value))

#define ST_APPEND1(head, tail, value)                   \
    do {                                                \
        if (ST_NULLP(head))                             \
        {                                               \
            (head) = (tail) = St_Cons((value), Nil);    \
        }                                               \
        else                                            \
        {                                               \
            ST_CDR_SET(tail, St_Cons((value), Nil));    \
            tail = ST_CDR(tail);                        \
        }                                               \
    } while (0)

#define ST_FOREACH(p, list) for (Object *p = (list); !ST_NULLP(p); p = ST_CDR(p))

// List as Set

bool St_SetMemberP(Object *obj, Object *s);
Object *St_SetCons(Object *obj, Object *s);
Object *St_SetUnion(Object *s1, Object *s2);
Object *St_SetMinus(Object *s1, Object *s2);
Object *St_SetIntersect(Object *s1, Object *s2);

// Symbol

extern Object *Symbols;
Object *St_Intern(const char *symbol_string);

// Primitive utilities

#define ST_CHECK_ARG_LEN(name, args, len)                       \
    do {                                                        \
        int _len = St_Length(args);                             \
                                                                \
        if (_len != (len))                                      \
        {                                                       \
            St_Error(name ": wrong number of arguments");       \
        }                                                       \
    } while (0)

#define ST_ARGS1(name, args, a1)                        \
    ST_CHECK_ARG_LEN(name, args, 1);                    \
    Object *(a1) = ST_CAR(args)

#define ST_ARGS2(name, args, a1, a2)                    \
    ST_CHECK_ARG_LEN(name, args, 2);                    \
    Object *(a1) = ST_CAR(args);                        \
    Object *(a2) = ST_CADR(args)

#define ST_ARGS3(name, args, a1, a2, a3)                \
    ST_CHECK_ARG_LEN(name, args, 3);                    \
    Object *(a1) = ST_CAR(args);                        \
    Object *(a2) = ST_CADR(args);                       \
    Object *(a3) = ST_CADDR(args)

#define ST_ARGS4(name, args, a1, a2, a3, a4)            \
    ST_CHECK_ARG_LEN(name, args, 4);                    \
    Object *(a1) = ST_CAR(args);                        \
    Object *(a2) = ST_CADR(args);                       \
    Object *(a3) = ST_CADDR(args);                      \
    Object *(a4) = ST_CADDDR(args)

// Constructors

#define ST_BOOLEAN(b) ((b) ? True : False)
Object *St_Integer(int value);

// String

Object *St_MakeEmptyString(int len);
Object *St_MakeString(int len, char* buf);
int St_StringLength(Object *s);
bool St_StringEqualP(Object *s1, Object *s2);
Object *St_StringAppend(Object *list);

// Vector

Object *St_MakeVector(int size);
Object *St_VectorRef(Object *vector, int idx);
void St_CopyVector(Object *dst, Object *src, int size);
void St_VectorSet(Object *vector, int idx, Object *obj);
int St_VectorLength(Object *vector);

// Dynamic Vector (complex type)

Object *St_MakeDVector(int size, int capa);
Object *St_DVectorRef(Object *vector, int idx);
void St_DVectorSet(Object *vector, int idx, Object *obj);
int St_DVectorLength(Object *vector);
Object *St_DVectorData(Object *vector);
int St_DVectorCapacity(Object *vector);
int St_DVectorPush(Object *vector, Object *obj);

// Module

Object *St_MakeModule(Object *alist);
int St_ModuleIndex(Object *module, Object *sym, Object *init);
void St_ModuleSet(Object *module, int idx, Object *val);
Object *St_ModuleRef(Object *module, int idx);

// Basic functions

bool St_EqvP(Object *lhs, Object *rhs);
bool St_EqualP(Object *lhs, Object *rhs);
Object *St_Apply(Object *env, Object *proc, Object *args);

// Environment

Object *St_InitEnv();
void St_AddVariable(Object *env, Object *key, Object *value);
void St_AddSyntax(Object *env, const char *key, SyntaxFunction *syntax);
void St_AddSubr(Object *env, const char *key, SubrFunction *subr);
Object *St_PushEnv(Object *env, Object *keys, Object *values);
Object *St_LookupVariablePair(Object *env, Object *key);
Object *St_LookupVariable(Object *env, Object *key);

void St_InitPrimitives(Object *env);

// Parser

Object *St_Read(FILE *stream);

// Printer

void St_Print(Object *obj);

// Evaluator

Object *St_Eval(Object *env, Object *obj);
Object *St_Eval_VM(Object *env, Object *obj);

// Compiler

Object *St_Compile(Object *expr, Object *env, Object *next);

#endif
