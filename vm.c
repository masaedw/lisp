#include "lisp.h"

static Object *stack;

static int push(Object *x, int s)
{
    St_VectorSet(stack, s, x);
    return s + 1;
}

static Object *index(int s, int i)
{
    return St_VectorRef(stack, s - i - 1);
}

static void index_set(int s, int i, Object *v)
{
    St_VectorSet(stack, s - i - 1, v);
}

static Object *make_closure(Object *body, int n, int s)
{
    Object *c = St_Alloc(TLAMBDAVM, sizeof(Object *) * 2);
    Object *f = n == 0 ? Nil : St_MakeVector(n);

    c->lambda_vm.body = body;
    c->lambda_vm.free = f;

    for (int i = 0; i < n; i++) {
        St_VectorSet(f, i, index(s, i));
    }

    return c;
}

static Object *closure_body(Object *c)
{
    return c->lambda_vm.body;
}

static Object *index_closure(Object *c, int n)
{
    return St_VectorRef(c->lambda_vm.free, n);
}

static Object *save_stack(int s)
{
    Object *v = St_MakeVector(s);
    St_CopyVector(v, stack, s);
    return v;
}

static int restore_stack(Object *v)
{
    int s = St_VectorLength(v);
    St_CopyVector(stack, v, s);
    return s;
}

static Object *make_continuation(int s)
{
    return make_closure(ST_LIST3(St_Intern("refer-local"),
                                 St_Integer(0),
                                 ST_LIST3(St_Intern("nuate"),
                                          save_stack(s),
                                          ST_LIST2(St_Intern("return"), St_Integer(0)))),
                        0,
                        s);
}

static int prepare_stack(Object *env, int e)
{
    if (ST_CAR(env) != Nil)
    {
        e += prepare_stack(ST_CAR(env), e) + 1;
    }

    int ne = e + St_Length(ST_CADR(env));
    int i = 0;

    ST_FOREACH(p, ST_CADR(env)) {
        Object *v = ST_CDAR(p);

        index_set(ne, i++, v);
    }

    index_set(ne, -1, St_Integer(e));

    return ne;
}

static Object *make_box(Object *obj)
{
    Object *v = St_MakeVector(1);

    St_VectorSet(v, 0, obj);

    return obj;
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
    for (int i = n - 1; i >= 0; i--) {
        index_set(s, i + m, index(s, i));
    }
    return s - m;
}

static Object *vm(Object *env, Object *insn)
{
    // registers
    Object *a; // Accumulator
    Object *x; // Next expression
    int f;     // Current frame
    int fp;    // Most inner frame
    Object *c; // Current closure
    int s;     // Current stack
    int g;     // Global variables

    stack = St_MakeVector(1000);

    a = Nil;
    x = insn;
    c = Nil;
    fp = f = g = s = prepare_stack(env, 0);

    // first argument               pushed by `argument`
    // ...                          ...
    // last argument                pushed by `argument`
    // next expression              pushed by `frame`
    // current frame                pushed by `frame`
    // most inner frame             pushed by `frame`
    // current closuer              pushed by `frame`

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    Object *refer_local = St_Intern("refer-local");
    Object *refer_free = St_Intern("refer-free");
    INSN(indirect);
    INSN(constant);
    INSN(close);
    INSN(box);
    INSN(test);
    Object *assign_local = St_Intern("assign-local");
    Object *assign_free = St_Intern("assign-free");
    INSN(conti);
    INSN(nuate);
    INSN(frame);
    INSN(argument);
    INSN(shift);
    INSN(apply);
    Object *rtn = St_Intern("return");
#undef INSN

#define CASE(x, insn) if (ST_CAR(x) == insn)

    while (true) {

        /*
        St_Print(ST_CAR(x));
        if (St_Length(x) > 1)
        {
            printf(" [");
            St_Print(ST_CADR(x));
            printf("]");
        }
        else
        {
            printf(" []");
        }
        printf(" (f:%d s:%d) ", f, s);
        St_Print(a);
        printf("\n");
        //*/

        CASE(x, halt) {
            return a;
        }

        CASE(x, refer_local) {
            ST_ARGS2("refer-local", ST_CDR(x), n, x2);
            a = index(f, n->integer.value);
            x = x2;
            continue;
        }

        CASE(x, refer_free) {
            ST_ARGS2("refer-free", ST_CDR(x), n, x2);
            if (ST_NULLP(c))
            {
                a = index(g, n->integer.value);
            }
            else
            {
                a = index_closure(c, n->integer.value);
            }
            x = x2;
            continue;
        }

        CASE(x, indirect) {
            ST_ARGS1("indirect", ST_CDR(x), x2);
            a = unbox(a);
            x = x2;
            continue;
        }

        CASE(x, constant) {
            ST_ARGS2("constant", ST_CDR(x), obj, x2);
            a = obj;
            x = x2;
            continue;
        }

        CASE(x, close) {
            ST_ARGS3("close", ST_CDR(x), n, body, x2);
            a = make_closure(body, n->integer.value, s);
            x = x2;
            s = s - n->integer.value;
            continue;
        }

        CASE(x, box) {
            ST_ARGS2("box", ST_CDR(x), n, x2);
            index_set(s, n->integer.value, make_box(index(s, n->integer.value)));
            x = x2;
            continue;
        }

        CASE(x, test) {
            ST_ARGS2("test", ST_CDR(x), thenc, elsec);
            x = a != False ? thenc : elsec;
            continue;
        }

        CASE(x, assign_local) {
            ST_ARGS2("assign-local", ST_CDR(x), n, x2);
            set_box(index(f, n->integer.value), a);
            x = x2;
            continue;
        }

        CASE(x, assign_free) {
            ST_ARGS2("assign-free", ST_CDR(x), n, x2);
            set_box(index_closure(c, n->integer.value), a);
            x = x2;
            continue;
        }

        CASE(x, conti) {
            ST_ARGS1("conti", ST_CDR(x), x2);
            a = make_continuation(s);
            x = x2;
            continue;
        }

        CASE(x, nuate) {
            ST_ARGS2("nuate", ST_CDR(x), st, x2);
            x = x2;
            s = restore_stack(st);
            continue;
        }

        CASE(x, frame) {
            ST_ARGS2("frame", ST_CDR(x), ret, x2);
            x = x2;
            s = push(ret, push(St_Integer(f), push(St_Integer(fp), push(c, s))));
            fp = s;
            continue;
        }

        CASE(x, argument) {
            ST_ARGS1("argument", ST_CDR(x), x2);
            x = x2;
            s = push(a, s);
            continue;
        }

        CASE(x, shift) {
            ST_ARGS3("shift", ST_CDR(x), n, m, x2);
            x = x2;
            s = shift_args(n->integer.value, m->integer.value, s);
        }

        CASE(x, apply) {
            if (ST_SUBRP(a))
            {
                // not supported higher order functions
                int len = s - fp;
                Object *head = Nil;
                Object *tail = Nil;

                for (int i = 0; i < len; i++) {
                    ST_APPEND1(head, tail, index(s, i));
                }

                a = St_Apply(env, a, head);

                // return
                x = index(s, len + 0);
                f = index(s, len + 1)->integer.value;
                fp = index(s, len + 2)->integer.value;
                c = index(s, len + 3);
                s = s - len - 4;
            }
            else
            {
                x = closure_body(a);
                f = s;
                c = a;
            }
            continue;
        }

        CASE(x, rtn) {
            ST_ARGS1("return", ST_CDR(x), n);
            int s2 = s - n->integer.value;
            x = index(s2, 0);
            f = index(s2, 1)->integer.value;
            fp = index(s2, 2)->integer.value;
            c = index(s2, 3);
            s = s2 - 4;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *env, Object *obj)
{
    return vm(env, St_Compile(obj, env, ST_LIST1(St_Intern("halt"))));
}
