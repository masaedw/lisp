#include <stdio.h>
#include "lisp.h"

typedef struct STVm
{
    StObject stack;
    StObject a; // Accumulator
    StObject x; // Next expression
    int f;     // Current frame
    int fp;    // Most inner frame
    StObject c; // Current closure
    int s;     // Current stack
    StObject m; // Current module

    // first argument               pushed by `argument`
    // ...                          ...
    // last argument                pushed by `argument`
    // next expression              pushed by `frame`
    // current frame                pushed by `frame`
    // most inner frame             pushed by `frame`
    // current closuer              pushed by `frame`

} STVm;

static STVm _Vm;
static STVm *Vm = &_Vm;

void St_InitVm()
{
    Vm->stack = St_MakeVector(1000);

    Vm->a = Nil;
    Vm->x = Nil;
    Vm->c = Nil;
    Vm->fp = Vm->f = Vm->s = 0;
}

static int push(StObject x, int s)
{
    St_VectorSet(Vm->stack, s, x);
    return s + 1;
}

static StObject index(int s, int i)
{
    return St_VectorRef(Vm->stack, s - i - 1);
}

static void index_set(int s, int i, StObject v)
{
    St_VectorSet(Vm->stack, s - i - 1, v);
}

static StObject make_closure(StObject body, int arity, int n, int s)
{
    StObject c = St_Alloc2(TLAMBDA, sizeof(struct StLambdaRec));
    StObject f = n == 0 ? Nil : St_MakeVector(n);

    ST_LAMBDA_BODY(c) = body;
    ST_LAMBDA_FREE(c) = f;
    ST_LAMBDA_ARITY(c) = arity;

    for (int i = 0; i < n; i++) {
        St_VectorSet(f, i, index(s, i));
    }

    return c;
}

static StObject index_closure(StObject c, int n)
{
    return St_VectorRef(ST_LAMBDA_FREE(c), n);
}

static StObject make_macro(StObject sym, StObject proc)
{
    StObject macro = St_Alloc2(TMACRO, sizeof(struct StMacroRec));
    ST_MACRO_PROC(macro) = proc;
    ST_MACRO_SYMBOL(macro) = sym;
    return macro;
}

static StObject save_stack(int s)
{
    StObject v = St_MakeVector(s);
    St_CopyVector(v, Vm->stack, s);
    return v;
}

static int restore_stack(StObject v)
{
    int s = St_VectorLength(v);
    St_CopyVector(Vm->stack, v, s);
    return s;
}

static StObject make_continuation(int s)
{
    return make_closure(ST_LIST3(St_Intern("refer-local"),
                                 St_Integer(0),
                                 ST_LIST3(St_Intern("nuate"),
                                          save_stack(s),
                                          ST_LIST2(St_Intern("return"), St_Integer(0)))),
                        1,
                        0,
                        s);
}

static StObject make_box(StObject obj)
{
    StObject v = St_MakeVector(1);

    St_VectorSet(v, 0, obj);

    return v;
}

static StObject unbox(StObject v)
{
    return St_VectorRef(v, 0);
}

static void set_box(StObject box, StObject obj)
{
    St_VectorSet(box, 0, obj);
}

static int shift_args(int n, int m, int s)
{
    if (m > 0)
    {
        for (int i = n - 1; i >= 0; i--) {
            index_set(s, i + m, index(s, i));
        }
    }
    else
    {
        for (int i = 0; i < n; i++) {
            index_set(s, i + m, index(s, i));
        }
    }
    return s - m;
}

static StObject vm(StObject m, StObject insn)
{
    // insns
#define INSN(x) StObject x = St_Intern(#x)
    INSN(halt);
    StObject refer_local  = St_Intern("refer-local");
    StObject refer_free   = St_Intern("refer-free");
    StObject refer_module = St_Intern("refer-module");
    INSN(indirect);
    INSN(constant);
    INSN(close);
    INSN(box);
    INSN(test);
    StObject assign_local  = St_Intern("assign-local");
    StObject assign_free   = St_Intern("assign-free");
    StObject assign_module = St_Intern("assign-module");
    INSN(conti);
    INSN(nuate);
    INSN(frame);
    INSN(argument);
    INSN(extend);
    INSN(shift);
    INSN(apply);
    INSN(macro);
    StObject rtn = St_Intern("return");
#undef INSN

#define CASE(insn) if (ST_CAR(Vm->x) == insn)

    StObject xo = Vm->x;
    Vm->x = insn;
    Vm->m = m;

    while (true) {

        /*
        St_Display(ST_CAR(Vm->x), False);
        if (St_Length(Vm->x) > 1)
        {
            printf(" [");
            St_Display(ST_CADR(Vm->x), False);
            printf("]");
        }
        else
        {
            printf(" []");
        }
        printf(" (f:%d fp:%d s:%d) ", Vm->f, Vm->fp, Vm->s);
        St_Print(Vm->a, False);
        //*/

        CASE(halt) {
            Vm->x = xo;
            return Vm->a;
        }

        CASE(refer_local) {
            ST_ARGS2("refer-local", ST_CDR(Vm->x), n, x);
            Vm->a = index(Vm->f, ST_INT_VALUE(n));
            Vm->x = x;
            continue;
        }

        CASE(refer_free) {
            ST_ARGS2("refer-free", ST_CDR(Vm->x), n, x);
            Vm->a = index_closure(Vm->c, ST_INT_VALUE(n));
            Vm->x = x;
            continue;
        }

        CASE(refer_module) {
            ST_ARGS2("refer-module", ST_CDR(Vm->x), n, x);
            StObject pair = St_ModuleRef(Vm->m, ST_INT_VALUE(n));
            if (ST_UNBOUNDP(ST_CDR(pair)))
            {
                St_Error("unbound variable %s", ST_SYMBOL_VALUE(ST_CAR(pair)));
            }
            Vm->a = ST_CDR(pair);
            Vm->x = x;
            continue;
        }

        CASE(indirect) {
            ST_ARGS1("indirect", ST_CDR(Vm->x), x);
            Vm->a = unbox(Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(constant) {
            ST_ARGS2("constant", ST_CDR(Vm->x), obj, x);
            Vm->a = obj;
            Vm->x = x;
            continue;
        }

        CASE(close) {
            ST_ARGS4("close", ST_CDR(Vm->x), arity, n, body, x);
            Vm->a = make_closure(body, ST_INT_VALUE(arity), ST_INT_VALUE(n), Vm->s);
            Vm->x = x;
            Vm->s = Vm->s - ST_INT_VALUE(n);
            continue;
        }

        CASE(box) {
            ST_ARGS2("box", ST_CDR(Vm->x), n, x);
            index_set(Vm->f, ST_INT_VALUE(n), make_box(index(Vm->f, ST_INT_VALUE(n))));
            Vm->x = x;
            continue;
        }

        CASE(test) {
            ST_ARGS2("test", ST_CDR(Vm->x), thenc, elsec);
            Vm->x = !ST_FALSEP(Vm->a) ? thenc : elsec;
            continue;
        }

        CASE(assign_local) {
            ST_ARGS2("assign-local", ST_CDR(Vm->x), n, x);
            set_box(index(Vm->f, ST_INT_VALUE(n)), Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(assign_free) {
            ST_ARGS2("assign-free", ST_CDR(Vm->x), n, x);
            set_box(index_closure(Vm->c, ST_INT_VALUE(n)), Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(assign_module) {
            ST_ARGS2("assign-module", ST_CDR(Vm->x), n, x);
            St_ModuleSet(Vm->m, ST_INT_VALUE(n), Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(conti) {
            ST_ARGS1("conti", ST_CDR(Vm->x), x);
            Vm->a = make_continuation(Vm->s);
            Vm->x = x;
            continue;
        }

        CASE(nuate) {
            ST_ARGS2("nuate", ST_CDR(Vm->x), st, x);
            Vm->x = x;
            Vm->s = restore_stack(st);
            continue;
        }

        CASE(frame) {
            ST_ARGS2("frame", ST_CDR(Vm->x), ret, x);
            Vm->x = x;
            Vm->s = push(ret, push(St_Integer(Vm->f), push(St_Integer(Vm->fp), push(Vm->c, Vm->s))));
            Vm->fp = Vm->s;
            continue;
        }

        CASE(argument) {
            ST_ARGS1("argument", ST_CDR(Vm->x), x);
            Vm->x = x;
            Vm->s = push(Vm->a, Vm->s);
            continue;
        }

        CASE(extend) {
            ST_ARGS2("extend", ST_CDR(Vm->x), n, x);
            Vm->x = x;
            Vm->f += ST_INT_VALUE(n);
            for (int i = 0; i < ST_INT_VALUE(n); i++) {
                Vm->s = push(make_box(Unbound), Vm->s);
            }
            continue;
        }

        CASE(shift) {
            ST_ARGS3("shift", ST_CDR(Vm->x), n, m, x);
            Vm->x = x;
            Vm->s = shift_args(ST_INT_VALUE(n), ST_INT_VALUE(m), Vm->s);
            continue;
        }

        CASE(apply) {
            if (ST_SUBRP(Vm->a))
            {
                // not supported higher order functions
                int len = Vm->s - Vm->fp;
                StObject head = Nil;
                StObject tail = Nil;

                for (int i = 0; i < len; i++) {
                    ST_APPEND1(head, tail, index(Vm->s, i));
                }

                Vm->a = ST_SUBR_BODY(Vm->a)(head);

                // return
                Vm->x = index(Vm->s, len + 0);
                Vm->f = ST_INT_VALUE(index(Vm->s, len + 1));
                Vm->fp = ST_INT_VALUE(index(Vm->s, len + 2));
                Vm->c = index(Vm->s, len + 3);
                Vm->s = Vm->s - len - 4;
            }
            else
            {
                int len = Vm->s - Vm->fp;
                int arity = ST_LAMBDA_ARITY(Vm->a);

                if (arity >= 0)
                {
                    if (arity != len)
                    {
                        St_Error("wrong number of arguments: required %d but got %d", arity, len);
                    }
                }
                else
                {
                    int required = -arity - 1;
                    if (required > len)
                    {
                        St_Error("wrong number of arguments: required %d but got %d", required, len);
                    }

                    int listed = len - required;

                    StObject head = Nil;
                    StObject tail = Nil;

                    for (int i = 0 + required; i < listed + required; i++) {
                        ST_APPEND1(head, tail, index(Vm->s, i));
                    }

                    shift_args(required, listed - 1, Vm->s);
                    index_set(Vm->s, listed + required - 1, head);
                    Vm->s -= listed - 1;
                }

                Vm->x = ST_LAMBDA_BODY(Vm->a);
                Vm->f = Vm->s;
                Vm->c = Vm->a;
            }
            continue;
        }

        CASE(macro) {
            ST_ARGS2("macro", ST_CDR(Vm->x), sym, x);
            Vm->a = make_macro(sym, Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(rtn) {
            ST_ARGS1("return", ST_CDR(Vm->x), n);
            int s2 = Vm->s - ST_INT_VALUE(n);
            Vm->x = index(s2, 0);
            Vm->f = ST_INT_VALUE(index(s2, 1));
            Vm->fp = ST_INT_VALUE(index(s2, 2));
            Vm->c = index(s2, 3);
            Vm->s = s2 - 4;
            continue;
        }
    }
}

StObject St_Eval_VM(StObject module, StObject obj)
{
    return vm(module, St_Compile(obj, module, ST_LIST1(St_Intern("halt"))));
}

StObject St__Eval_INSN(StObject module, StObject insn)
{
    return vm(module, insn);
}

#define I(x) St_Intern(x)

StObject St_Apply(StObject proc, StObject args)
{
    StObject i = ST_LIST3(I("constant"), proc, ST_LIST1(I("apply")));

    ST_FOREACH(p, args) {
        i = ST_LIST3(I("constant"), ST_CAR(p),
                     ST_LIST2(I("argument"),
                              i));
    }

    i = ST_LIST3(I("frame"), ST_LIST1(I("halt")), i);

    return St__Eval_INSN(Vm->m, i);
}
