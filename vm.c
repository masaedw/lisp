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
    Object *v = St_MakeVector(n + 1);

    St_VectorSet(v, 0, body);

    for (int i = 0; i < n; i++) {
        St_VectorSet(v, i + 1, index(s, i));
    }

    return v;
}

static Object *closure_body(Object *c)
{
    return St_VectorRef(c, 0);
}

static Object *index_closure(Object *c, int n)
{
    return St_VectorRef(c, n + 1);
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
    return make_closure(ST_LIST4(St_Intern("refer"),
                                 St_Integer(0),
                                 St_Integer(0),
                                 ST_LIST3(St_Intern("nuate"),
                                          save_stack(s),
                                          ST_LIST2(St_Intern("return"), St_Integer(0)))),
                        0,
                        0);
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

static Object *vm(Object *env, Object *insn)
{
    // registers
    Object *a; // Accumulator
    Object *x; // Next expression
    int f;     // Current frame
    Object *c; // Current closure
    int s;     // Current stack

    stack = St_MakeVector(1000);

    a = Nil;
    x = insn;
    c = Nil;
    s = prepare_stack(env, 0) + 1;
    f = s;

    // first argument               pushed by `argument`
    // ...                          ...
    // last argument                pushed by `argument`
    // next expression              pushed by `frame`
    // current frame                pushed by `frame`
    // dynamic link (pushed first)  pushed by `frame`

    // dynamic link
    // The dynamic link always points to the caller’s frame.

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    Object *refer_local = St_Intern("refer-local");
    Object *refer_free = St_Intern("refer-free");
    INSN(constant);
    INSN(close);
    INSN(test);
    INSN(conti);
    INSN(nuate);
    INSN(frame);
    INSN(argument);
    INSN(apply);
    Object *rtn = St_Intern("return");
#undef INSN

#define CASE(x, insn) if (ST_CAR(x) == insn)

    while (true) {

        //*
        printf("%s (f:%d s:%d) ", ST_CAR(x)->symbol_value, f, s);
        St_Print(a);
        printf("\n");
        //*/

        CASE(x, halt) {
            return a;
        }

        CASE(x, refer_local) {
            ST_ARGS2("refer-local", ST_CDR(x), n, x2);
            a = index(f, n->int_value);
            x = x2;
            continue;
        }

        CASE(x, refer_free) {
            ST_ARGS2("refer-free", ST_CDR(x), n, x2);
            a = index_closure(c, n->int_value);
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
            a = make_closure(body, n->int_value, s);
            x = x2;
            s = s - n->int_value;
            continue;
        }

        CASE(x, test) {
            ST_ARGS2("test", ST_CDR(x), thenc, elsec);
            x = a != False ? thenc : elsec;
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
            s = push(ret, push(St_Integer(f), push(c, s)));
            f = s;
            continue;
        }

        CASE(x, argument) {
            ST_ARGS1("argument", ST_CDR(x), x2);
            x = x2;
            s = push(a, s);
            continue;
        }

        CASE(x, apply) {
            if (ST_SUBRP(a))
            {
                // Global variable reference is currently not supported. So can't refer subr for now.

                // not supported higher order functions
                int len = s - f;
                Object *head = Nil;
                Object *tail = Nil;

                for (int i = 0; i < len; i++) {
                    ST_APPEND1(head, tail, index(s, i));
                }

                a = St_Apply(env, a, head);

                // return
                x = index(s, len + 0);
                f = index(s, len + 1)->int_value;
                c = index(s, len + 2);
                s = s - len - 3; 
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
            int s2 = s - n->int_value;
            x = index(s2, 0);
            f = index(s2, 1)->int_value;
            c = index(s2, 2);
            s = s2 - 3;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *env, Object *obj)
{
    return vm(env, St_Compile(obj, env, ST_LIST1(St_Intern("halt"))));
}
