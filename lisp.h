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
    TSYMBOL,
    TSTRING,
    TSYNTAX,
    TSUBR,
    TLAMBDA,
    TMACRO,
};

typedef struct Object Object;
typedef Object *SyntaxFunction(Object *env, Object *form);
typedef Object *SubrFunction(Object *env, Object *args);

struct Object
{
    int type;

    union {
        int int_value;

        // cell
        struct {
            Object *car;
            Object *cdr;
        };

        char *symbol_value;

        char *string_value;

        // syntax
        struct {
            SyntaxFunction *syntax;
            const char *syntax_name;
        };

        // subr
        struct {
            SubrFunction *subr;
            const char *subr_name;
        };

        // lambda
        struct {
            Object *params;
            Object *body;
            Object *env;
        };

        // macro
        struct {
            Object *proc;
            Object *macro_symbol;
        };
    };
};

extern Object *Nil;
extern Object *True;
extern Object *False;

#define St_Error(message)                       \
    do {                                        \
        fprintf(stderr, message);               \
        exit(1);                                \
    } while (0)

#define St_Malloc GC_MALLOC

Object *St_Alloc(int type);

#define ST_NULLP(obj) ((obj) == Nil)
#define ST_TRUEP(obj) ((obj) == True)
#define ST_FALSEP(obj) ((obj) == False)
#define ST_PAIRP(obj) ((obj)->type == TCELL)
#define ST_INTP(obj) ((obj)->type == TINT)
#define ST_SYMBOLP(obj) ((obj)->type == TSYMBOL)
#define ST_STRINGP(obj) ((obj)->type == TSTRING)
#define ST_SYNTAXP(obj) ((obj)->type == TSYNTAX)
#define ST_SUBRP(obj) ((obj)->type == TSUBR)
#define ST_LAMBDAP(obj) ((obj)->type == TLAMBDA)
#define ST_MACROP(obj) ((obj)->type == TMACRO)

Object *St_Cons(Object *car, Object *cdr);
Object *St_Acons(Object *key, Object *val, Object *cdr);
Object *St_Reverse(Object *list);
int St_Length(Object *list);
bool St_ListP(Object *maybe_list);

#define ST_LIST1(a0)             St_Cons((a0), Nil)
#define ST_LIST2(a0, a1)         St_Cons((a0), ST_LIST1((a1)))
#define ST_LIST3(a0, a1, a2)     St_Cons((a0), ST_LIST2((a1), (a2)))
#define ST_LIST4(a0, a1, a2, a3) St_Cons((a0), ST_LIST3((a1), (a2), (a3)))

#define ST_CAR(pair) (pair)->car
#define ST_CDR(pair) (pair)->cdr
#define ST_CDDR(list) ST_CDR(ST_CDR(list))
#define ST_CDAR(list) ST_CDR(ST_CAR(list))
#define ST_CADR(list) ST_CAR(ST_CDR(list))
#define ST_CAAR(list) ST_CAR(ST_CAR(list))
#define ST_CADDR(list) ST_CAR(ST_CDR(ST_CDR(list)))
#define ST_CADDDR(list) ST_CAR(ST_CDR(ST_CDR(ST_CDR(list))))

#define ST_CAR_SET(pair, value) ((pair)->car = (value))
#define ST_CDR_SET(pair, value) ((pair)->cdr = (value))

#define ST_APPEND1(head, tail, value)                   \
    do {                                                \
        if (ST_NULLP(head))                             \
        {                                               \
            (head) = (tail) = St_Cons((value), Nil);    \
        }                                               \
        else                                            \
        {                                               \
            tail->cdr = St_Cons((value), Nil);          \
            tail = tail->cdr;                           \
        }                                               \
    } while (0)


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

#define ST_BOOLEAN(b) ((b) ? True : False)
Object *St_Integer(int value);

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

Object *St_Compile(Object *expr, Object *next);

#endif
