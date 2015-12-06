#ifndef LISP_H
#define LISP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// type tag
enum {
    TINT = 1,
    TCELL,
    TNIL,
    TTRUE,
    TFALSE,
    TSYMBOL,
    TSYNTAX,
    TSUBR,
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

#define St_Malloc malloc

Object *St_Alloc(int type);

#define ST_NULLP(obj) ((obj) == Nil)
#define ST_TRUEP(obj) ((obj) == True)
#define ST_FALSEP(obj) ((obj) == False)
#define ST_PAIRP(obj) ((obj)->type == TCELL)
#define ST_INTP(obj) ((obj)->type == TINT)
#define ST_SYMBOLP(obj) ((obj)->type == TSYMBOL)
#define ST_SYNTAXP(obj) ((obj)->type == TSYNTAX)
#define ST_SUBRP(obj) ((obj)->type == TSUBR)

Object *St_Cons(Object *car, Object *cdr);
Object *St_Acons(Object *key, Object *val, Object *cdr);
Object *St_Reverse(Object *list);
int St_Length(Object *list);

extern Object *Symbols;
Object *St_Intern(const char *symbol_string);
Object *St_Find(Object *env, Object *symbol);

// Environment

Object *St_InitEnv();
void St_AddVariable(Object *env, Object *key, Object *value);
void St_AddSyntax(Object *env, const char *key, SyntaxFunction *syntax);
void St_AddSubr(Object *env, const char *key, SubrFunction *subr);
Object *St_PushEnv(Object *env, Object *keys, Object *values);
Object *St_LookupVariable(Object *env, Object *key);

void St_InitPrimitives(Object *env);

// Parser

Object *St_Read(FILE *stream);

// Printer

void St_Print(Object *obj);

// Evaluator

Object *St_Eval(Object *env, Object *obj);

#endif
