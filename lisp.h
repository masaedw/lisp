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
};

typedef struct Object Object;

struct Object
{
    int type;

    union {
        int int_value;

        struct {
            Object *car;
            Object *cdr;
        };

        char *symbol_value;
    };
};

extern static Object *Nil;
extern static Object *True;
extern static Object *False;

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
#define ST_SYBOLP(obj) ((obj)->type == TSYMBOL)

Object *St_Cons(Object *car, Object *cdr);
Object *St_Reverse(Object *list);

// Parser

Object *St_Read(FILE *stream);

// Printer

void St_Print(Object *obj);

#endif
