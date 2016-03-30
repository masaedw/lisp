#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "lisp.h"

void test_print()
{
    St_Print(Nil, False);

    St_Print(True, False);

    St_Print(False, False);

    StObject one = St_Integer(1);

    StObject two = St_Integer(2);

    St_Print(St_Cons(one, two), False);

    St_Print(St_Cons(one, St_Cons(two, Nil)), False);

    StObject sym1 = St_Intern("sym1");

    St_Print(sym1, False);

    StObject sym2 = St_Intern("sym2");

    St_Print(St_Cons(sym1, sym2), False);


    St_Print(St_Cons(St_Cons(one, St_Cons(two, Nil)),
                     St_Cons(sym1, St_Cons(sym2, St_Cons(Nil, sym1)))),
             False);

    StObject str = St_MakeString(4, "hoge");
    St_Print(str, False);

    StObject v = St_MakeVector(3);
    St_VectorSet(v, 0, one);
    St_VectorSet(v, 1, sym1);
    St_VectorSet(v, 2, True);
    St_Print(v, False);
}

void test_port()
{
    StObject in = St_MakeFdPort(0, false);

    while (true) {
        printf("input: ");
        fflush(stdout);
        do {
            StObject c = St_ReadChar(in);
            if (ST_EOFP(c))
            {
                return;
            }
            printf("%c", (char)ST_INT_VALUE(c));
        } while (St_CharReadyP(in));
    }
}

static StObject input;

void finalizer()
{
    //fclose(input);
}

static StObject MakeListFromArgv(char **argv)
{
    StObject h = Nil, t = Nil;

    for (char **p = argv; *p != NULL; p++) {
        ST_APPEND1(h, t, St_MakeStringFromCString(*p));
    }

    return h;
}

int main(int argc, char** argv)
{
    GC_INIT();

    St_InitModule();
    St_InitPort();
    St_InitSystem(argc, argv);
    St_InitPrimitives();
    St_InitSyntax();
    St_InitVm();

    St_InitSrfi60();

    StObject expr;

    bool interactive_mode = false;
    int pargs = 1;

    StObject args = MakeListFromArgv(argv);

    if (ST_TRUTHYP(St_Member(St_MakeStringFromCString("-i"), args)))
    {
        interactive_mode = true;
        pargs++;
    }

    if (ST_TRUTHYP(St_Member(St_MakeStringFromCString("-p"), args)))
    {
        test_print();
        return 0;
    }

    if (ST_TRUTHYP(St_Member(St_MakeStringFromCString("-t"), args)))
    {
        test_port();
        return 0;
    }

    if (ST_TRUTHYP(St_Member(St_MakeStringFromCString("-d"), args)))
    {
        St_DebugVM = True;
        pargs++;
    }

    atexit(finalizer);

    if (argc >= pargs + 1)
    {
        int f = open(argv[pargs], O_RDONLY);
        if (f == -1)
        {
            St_Error("can't open: %s", argv[pargs]);
        }

        input = St_MakeFdPort(f, true);
        St_CurrentExecScriptName = St_MakeStringFromCString(argv[pargs]);
    }
    else
    {
        input = St_StandardInputPort;
        St_CurrentExecScriptName = St_MakeStringFromCString("-");
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
            St_Print(value, False);
        }
    };


    return 0;
}
