#include "lisp.h"

typedef struct STVm
{
    Object *stack;
    Object *a; // Accumulator
    Object *x; // Next expression
    int f;     // Current frame
    int fp;    // Most inner frame
    Object *c; // Current closure
    int s;     // Current stack
    Object *m; // Current module

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

static int push(Object *x, int s)
{
    St_VectorSet(Vm->stack, s, x);
    return s + 1;
}

static Object *index(int s, int i)
{
    return St_VectorRef(Vm->stack, s - i - 1);
}

static void index_set(int s, int i, Object *v)
{
    St_VectorSet(Vm->stack, s - i - 1, v);
}

static Object *make_closure(Object *body, int arity, int n, int s)
{
    Object *c = St_Alloc(TLAMBDA, sizeof(Object *) * 2);
    Object *f = n == 0 ? Nil : St_MakeVector(n);

    c->lambda.body = body;
    c->lambda.free = f;
    c->lambda.arity = arity;

    for (int i = 0; i < n; i++) {
        St_VectorSet(f, i, index(s, i));
    }

    return c;
}

static Object *closure_body(Object *c)
{
    return c->lambda.body;
}

static int closure_arity(Object *c)
{
    return c->lambda.arity;
}

static Object *index_closure(Object *c, int n)
{
    return St_VectorRef(c->lambda.free, n);
}

static Object *make_macro(Object *sym, Object *proc)
{
    Object *macro = St_Alloc(TMACRO, sizeof(void*) * 2);
    macro->macro.proc = proc;
    macro->macro.symbol = sym;
    return macro;
}

static Object *save_stack(int s)
{
    Object *v = St_MakeVector(s);
    St_CopyVector(v, Vm->stack, s);
    return v;
}

static int restore_stack(Object *v)
{
    int s = St_VectorLength(v);
    St_CopyVector(Vm->stack, v, s);
    return s;
}

static Object *make_continuation(int s)
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

static Object *make_box(Object *obj)
{
    Object *v = St_MakeVector(1);

    St_VectorSet(v, 0, obj);

    return v;
}

static Object *unbox(Object *v)
{
    return St_VectorRef(v, 0);
}

static void set_box(Object *box, Object *obj)
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

static Object *vm(Object *m, Object *env, Object *insn)
{
    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    Object *refer_local  = St_Intern("refer-local");
    Object *refer_free   = St_Intern("refer-free");
    Object *refer_module = St_Intern("refer-module");
    INSN(indirect);
    INSN(constant);
    INSN(close);
    INSN(box);
    INSN(test);
    Object *assign_local  = St_Intern("assign-local");
    Object *assign_free   = St_Intern("assign-free");
    Object *assign_module = St_Intern("assign-module");
    INSN(conti);
    INSN(nuate);
    INSN(frame);
    INSN(argument);
    INSN(shift);
    INSN(apply);
    INSN(macro);
    Object *rtn = St_Intern("return");
#undef INSN

#define CASE(insn) if (ST_CAR(Vm->x) == insn)

    Object *xo = Vm->x;
    Vm->x = insn;
    Vm->m = m;

    while (true) {

        /*
        St_Display(ST_CAR(Vm->x));
        if (St_Length(Vm->x) > 1)
        {
            printf(" [");
            St_Display(ST_CADR(Vm->x));
            printf("]");
        }
        else
        {
            printf(" []");
        }
        printf(" (f:%d fp:%d s:%d) ", Vm->f, Vm->fp, Vm->s);
        St_Print(Vm->a);
        //*/

        CASE(halt) {
            Vm->x = xo;
            return Vm->a;
        }

        CASE(refer_local) {
            ST_ARGS2("refer-local", ST_CDR(Vm->x), n, x);
            Vm->a = index(Vm->f, n->integer.value);
            Vm->x = x;
            continue;
        }

        CASE(refer_free) {
            ST_ARGS2("refer-free", ST_CDR(Vm->x), n, x);
            Vm->a = index_closure(Vm->c, n->integer.value);
            Vm->x = x;
            continue;
        }

        CASE(refer_module) {
            ST_ARGS2("refer-module", ST_CDR(Vm->x), n, x);
            Object *pair = St_ModuleRef(Vm->m, n->integer.value);
            if (ST_UNBOUNDP(ST_CDR(pair)))
            {
                St_Error("unbound variable %s", ST_CAR(pair)->symbol.value);
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
            Vm->a = make_closure(body, arity->integer.value, n->integer.value, Vm->s);
            Vm->x = x;
            Vm->s = Vm->s - n->integer.value;
            continue;
        }

        CASE(box) {
            ST_ARGS2("box", ST_CDR(Vm->x), n, x);
            index_set(Vm->f, n->integer.value, make_box(index(Vm->f, n->integer.value)));
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
            set_box(index(Vm->f, n->integer.value), Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(assign_free) {
            ST_ARGS2("assign-free", ST_CDR(Vm->x), n, x);
            set_box(index_closure(Vm->c, n->integer.value), Vm->a);
            Vm->x = x;
            continue;
        }

        CASE(assign_module) {
            ST_ARGS2("assign-module", ST_CDR(Vm->x), n, x);
            St_ModuleSet(Vm->m, n->integer.value, Vm->a);
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

        CASE(shift) {
            ST_ARGS3("shift", ST_CDR(Vm->x), n, m, x);
            Vm->x = x;
            Vm->s = shift_args(n->integer.value, m->integer.value, Vm->s);
        }

        CASE(apply) {
            if (ST_SUBRP(Vm->a))
            {
                // not supported higher order functions
                int len = Vm->s - Vm->fp;
                Object *head = Nil;
                Object *tail = Nil;

                for (int i = 0; i < len; i++) {
                    ST_APPEND1(head, tail, index(Vm->s, i));
                }

                Vm->a = Vm->a->subr.body(env, head);

                // return
                Vm->x = index(Vm->s, len + 0);
                Vm->f = index(Vm->s, len + 1)->integer.value;
                Vm->fp = index(Vm->s, len + 2)->integer.value;
                Vm->c = index(Vm->s, len + 3);
                Vm->s = Vm->s - len - 4;
            }
            else
            {
                int len = Vm->s - Vm->fp;
                int arity = closure_arity(Vm->a);

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

                    Object *head = Nil;
                    Object *tail = Nil;

                    for (int i = 0 + required; i < listed + required; i++) {
                        ST_APPEND1(head, tail, index(Vm->s, i));
                    }

                    shift_args(required, listed - 1, Vm->s);
                    index_set(Vm->s, listed + required - 1, head);
                    Vm->s -= listed - 1;
                }

                Vm->x = closure_body(Vm->a);
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
            int s2 = Vm->s - n->integer.value;
            Vm->x = index(s2, 0);
            Vm->f = index(s2, 1)->integer.value;
            Vm->fp = index(s2, 2)->integer.value;
            Vm->c = index(s2, 3);
            Vm->s = s2 - 4;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *module, Object *env, Object *obj)
{
    return vm(module, env, St_Compile(obj, module, env, ST_LIST1(St_Intern("halt"))));
}

Object *St__Eval_INSN(Object *module, Object *env, Object *insn)
{
    return vm(module, env, insn);
}

#define I(x) St_Intern(x)

Object *St_Apply(Object *proc, Object *args)
{
    Object *i = ST_LIST3(I("constant"), proc, ST_LIST1(I("apply")));

    ST_FOREACH(p, args) {
        i = ST_LIST3(I("constant"), ST_CAR(p),
                     ST_LIST2(I("argument"),
                              i));
    }

    i = ST_LIST3(I("frame"), ST_LIST1(I("halt")), i);

    return St__Eval_INSN(Vm->m, Nil, i);
}
