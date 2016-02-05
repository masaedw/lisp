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
    TMACRO,
};

typedef struct Object Object;
typedef Object * StObject;
typedef StObject SyntaxFunction(StObject form);
typedef StObject SubrFunction(StObject env, StObject args);

struct Object
{
    int type;

    union {
        struct {
            int value;
        } integer;

        // cell
        struct {
            StObject car;
            StObject cdr;
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
            StObject data[1];
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
            StObject body;
            StObject free;
            int arity;
        } lambda;

        // macro
        struct {
            StObject proc;
            StObject symbol;
        } macro;
    };
};

extern StObject Nil;
extern StObject True;
extern StObject False;
extern StObject Unbound;

void St_Error(const char *fmt, ...) __attribute__((noreturn));

#define St_Malloc GC_MALLOC

StObject St_Alloc(int type, size_t size);

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
#define ST_LAMBDAP(obj) ((obj)->type == TLAMBDA)
#define ST_MACROP(obj) ((obj)->type == TMACRO)
#define ST_PROCEDUREP(obj) (ST_SUBRP(obj) || ST_LAMBDAP(obj))

// List and Pair

StObject St_Cons(StObject car, StObject cdr);
StObject St_Acons(StObject key, StObject val, StObject cdr);
StObject St_Reverse(StObject list);
int St_Length(StObject list);
bool St_ListP(StObject maybe_list);
bool St_DottedListP(StObject maybe_list);

#define ST_LIST1(a0)                 St_Cons((a0), Nil)
#define ST_LIST2(a0, a1)             St_Cons((a0), ST_LIST1((a1)))
#define ST_LIST3(a0, a1, a2)         St_Cons((a0), ST_LIST2((a1), (a2)))
#define ST_LIST4(a0, a1, a2, a3)     St_Cons((a0), ST_LIST3((a1), (a2), (a3)))
#define ST_LIST5(a0, a1, a2, a3, a4) St_Cons((a0), ST_LIST4((a1), (a2), (a3), (a4)))

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

#define ST_FOREACH(p, list) for (StObject p = (list); !ST_NULLP(p); p = ST_CDR(p))

// List as Set

bool St_SetMemberP(StObject obj, StObject s);
StObject St_SetCons(StObject obj, StObject s);
StObject St_SetUnion(StObject s1, StObject s2);
StObject St_SetMinus(StObject s1, StObject s2);
StObject St_SetIntersect(StObject s1, StObject s2);

// Symbol

extern StObject Symbols;
StObject St_Intern(const char *symbol_string);

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
    StObject (a1) = ST_CAR(args)

#define ST_ARGS2(name, args, a1, a2)                    \
    ST_CHECK_ARG_LEN(name, args, 2);                    \
    StObject (a1) = ST_CAR(args);                        \
    StObject (a2) = ST_CADR(args)

#define ST_ARGS3(name, args, a1, a2, a3)                \
    ST_CHECK_ARG_LEN(name, args, 3);                    \
    StObject (a1) = ST_CAR(args);                        \
    StObject (a2) = ST_CADR(args);                       \
    StObject (a3) = ST_CADDR(args)

#define ST_ARGS4(name, args, a1, a2, a3, a4)            \
    ST_CHECK_ARG_LEN(name, args, 4);                    \
    StObject (a1) = ST_CAR(args);                        \
    StObject (a2) = ST_CADR(args);                       \
    StObject (a3) = ST_CADDR(args);                      \
    StObject (a4) = ST_CADDDR(args)

// Coercers

#define ST_BOOLEAN(b) ((b) ? True : False)
StObject St_Integer(int value);
#define ST_INT_VALUE(x) (x)->integer.value
#define ST_OBJECT(x) ((StObject)(x))

// String

StObject St_MakeEmptyString(int len);
StObject St_MakeString(int len, char* buf);
int St_StringLength(StObject s);
bool St_StringEqualP(StObject s1, StObject s2);
StObject St_StringAppend(StObject list);

// Vector

StObject St_MakeVector(int size);
StObject St_VectorRef(StObject vector, int idx);
void St_CopyVector(StObject dst, StObject src, int size);
void St_VectorSet(StObject vector, int idx, StObject obj);
int St_VectorLength(StObject vector);

// Dynamic Vector (complex type)

StObject St_MakeDVector(int size, int capa);
StObject St_DVectorRef(StObject vector, int idx);
void St_DVectorSet(StObject vector, int idx, StObject obj);
int St_DVectorLength(StObject vector);
StObject St_DVectorData(StObject vector);
int St_DVectorCapacity(StObject vector);
int St_DVectorPush(StObject vector, StObject obj);

// Module

extern StObject GlobalModule;
StObject St_MakeModule(StObject alist);
StObject St_ModuleFind(StObject module, StObject sym);
int St_ModuleFindOrInitialize(StObject module, StObject sym, StObject init);
void St_ModuleSet(StObject module, int idx, StObject val);
StObject St_ModuleRef(StObject module, int idx);
StObject St_ModuleSymbols(StObject module);
void St_InitModule(StObject env);

// Basic functions

bool St_EqvP(StObject lhs, StObject rhs);
bool St_EqualP(StObject lhs, StObject rhs);
StObject St_Gensym();
StObject St_Apply(StObject proc, StObject args);

// Environment

StObject St_InitEnv();
void St_AddVariable(StObject env, StObject key, StObject value);
void St_AddSyntax(StObject env, const char *key, SyntaxFunction *syntax);
void St_AddSubr(StObject env, const char *key, SubrFunction *subr);
StObject St_PushEnv(StObject env, StObject keys, StObject values);
StObject St_LookupVariablePair(StObject env, StObject key);
StObject St_LookupVariable(StObject env, StObject key);

void St_InitPrimitives(StObject env);
void St_InitSyntax(StObject env);
void St_InitVm();

// Parser

StObject St_Read(FILE *stream);

// Printer

void St_Display(StObject obj);
void St_Print(StObject obj);

// Evaluator

StObject St_Eval_VM(StObject module, StObject env, StObject obj);
StObject St__Eval_INSN(StObject module, StObject env, StObject insn);

// Compiler

StObject St_MacroExpand(StObject module, StObject expr);
StObject St_Compile(StObject expr, StObject module, StObject env, StObject next);

#endif
