#include <string.h>

#include "lisp.h"

void test_print()
{
    St_Print(Nil);
    printf("\n");

    St_Print(True);
    printf("\n");

    St_Print(False);
    printf("\n");

    Object *one = St_Integer(1);

    Object *two = St_Integer(2);

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

    Object *str = St_Alloc(TSTRING);
    str->string_value = "hoge";
    St_Print(str);
    printf("\n");
}

int main(int argc, char** argv)
{
    GC_INIT();

    Object *env = St_InitEnv();
    St_InitPrimitives(env);

    Object *expr;

    bool interactive_mode = false;

    if (argc >= 2 && strcmp(argv[1], "-i") == 0)
    {
        interactive_mode = true;
    }

    if (argc >=2 && strcmp(argv[1], "-p") == 0)
    {
        test_print();
        return 0;
    }

    while (true) {
        expr = St_Read(stdin);
        if (!expr)
        {
            return 0;
        }

        Object *value = St_Eval(env, expr);

        if (interactive_mode)
        {
            St_Print(value);
        }
    };


    return 0;
}
