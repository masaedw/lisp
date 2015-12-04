#include "lisp.h"

int main(int argc, char** argv)
{
    //Object *expr = St_Read(stdin);
    //St_Print(expr);

    St_Print(Nil);
    printf("\n");

    St_Print(True);
    printf("\n");

    St_Print(False);
    printf("\n");

    Object *one = St_Alloc(TINT);
    one->int_value = 1;

    Object *two = St_Alloc(TINT);
    two->int_value = 2;

    St_Print(St_Cons(one, two));
    printf("\n");

    St_Print(St_Cons(one, St_Cons(two, Nil)));
    printf("\n");

    Object *sym1 = St_Alloc(TSYMBOL);
    sym1->symbol_value = "sym1";

    St_Print(sym1);
    printf("\n");

    Object *sym2 = St_Alloc(TSYMBOL);
    sym2->symbol_value = "sym2";

    St_Print(St_Cons(sym1, sym2));
    printf("\n");


    St_Print(St_Cons(St_Cons(one, St_Cons(two, Nil)),
                     St_Cons(sym1, St_Cons(sym2, St_Cons(Nil, sym1)))));
    printf("\n");

    return 0;
}
