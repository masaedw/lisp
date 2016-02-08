#include <string.h>

#include "lisp.h"

void test_print()
{
    St_Print(Nil);

    St_Print(True);

    St_Print(False);

    StObject one = St_Integer(1);

    StObject two = St_Integer(2);

    St_Print(St_Cons(one, two));

    St_Print(St_Cons(one, St_Cons(two, Nil)));

    StObject sym1 = St_Intern("sym1");

    St_Print(sym1);

    StObject sym2 = St_Intern("sym2");

    St_Print(St_Cons(sym1, sym2));


    St_Print(St_Cons(St_Cons(one, St_Cons(two, Nil)),
                     St_Cons(sym1, St_Cons(sym2, St_Cons(Nil, sym1)))));

    StObject str = St_MakeString(4, "hoge");
    St_Print(str);

    StObject v = St_MakeVector(3);
    St_VectorSet(v, 0, one);
    St_VectorSet(v, 1, sym1);
    St_VectorSet(v, 2, True);
    St_Print(v);
}

static FILE *input;

void finalizer()
{
    if (input != stdin)
    {
        fclose(input);
    }
}

int main(int argc, char** argv)
{
    GC_INIT();

    St_InitModule();
    St_InitPrimitives();
    St_InitSyntax();
    St_InitVm();

    StObject expr;

    bool interactive_mode = false;
    int pargs = 1;

    if (argc >= 2 && strcmp(argv[1], "-i") == 0)
    {
        interactive_mode = true;
        pargs++;
    }

    if (argc >=2 && strcmp(argv[1], "-p") == 0)
    {
        test_print();
        return 0;
    }

    input = stdin;
    atexit(finalizer);

    if (argc >= pargs + 1)
    {
        FILE *f = fopen(argv[pargs], "r");
        if (f == NULL)
        {
            St_Error("can't open: %s", argv[pargs]);
        }

        input = f;
    }

    while (true) {
        expr = St_Read(input);
        if (ST_EOFP(expr))
        {
            return 0;
        }

        StObject value = St_Eval_VM(GlobalModule, expr);

        if (interactive_mode)
        {
            St_Print(value);
        }
    };


    return 0;
}
