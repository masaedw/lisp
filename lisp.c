#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// type tag
enum {
    TINTEGER = 1,
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

        int symbol_value;
    };
};

static Object *Nil = &(Object) { TNIL };
static Object *True = &(Object) { TTRUE };
static Object *False = &(Object) { TFALSE };

// 構文
// <expr>    ::= <term> | <list>
// <term>    ::= <integer> | <symbol> | '\'' <expr> | #t | #f
// <list>    ::= '(' <listsub>
// <listsub> ::= ')' | <expr> + ['.' <expr>] ')'
// <integer> ::= <数字> +
// <symbol>  ::= <見える文字> +
