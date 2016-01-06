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

static Object *make_closure(Object *body, Object *e)
{
    return ST_LIST2(body, e);
}

static int find_link(int n, int e)
{
    for (; n != 0; n--, e = index(e, -1)->int_value)
    {
    }
    return e;
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
                        Nil);
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
    int e; // Current environment
    int s; // Current stack
    int f; // Current frame bottom

    stack = St_MakeVector(1000);

    a = Nil;
    x = insn;
    e = prepare_stack(env, 0);
    s = e + 1;
    f = s;

    // static link (pushed last)    pushed by `apply`
    // first argument               pushed by `argument`
    // ...                          ...
    // last argument                pushed by `argument`
    // current frame                pushed by `frame` to return from subr
    // next expression              pushed by `frame`
    // dynamic link (pushed first)  pushed by `frame`

    // static link
    // The static link, on the other hand, always points to the frame of the closest enclosing function definition of the called function.

    // dynamic link
    // The dynamic link always points to the caller’s frame.

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    INSN(refer);
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

        /*
        printf("%s (e:%d s:%d f:%d) ", ST_CAR(x)->symbol_value, e, s, f);
        St_Print(a);
        printf("\n");
        //*/

        CASE(x, halt) {
            return a;
        }

        CASE(x, refer) {
            ST_ARGS3("refer", ST_CDR(x), n, m, x2);
            a = index(find_link(n->int_value, e), m->int_value);
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
            ST_ARGS2("close", ST_CDR(x), body, x2);
            a = make_closure(body, St_Integer(e));
            x = x2;
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
        }

        CASE(x, frame) {
            ST_ARGS2("frame", ST_CDR(x), ret, x2);
            x = x2;
            s = push(St_Integer(f), push(ret, push(St_Integer(e), s)));
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
                // not supported higher order functions
                int len = s - f;
                Object *head = Nil;
                Object *tail = Nil;

                for (int i = 0; i < len; i++) {
                    ST_APPEND1(head, tail, index(s, i));
                }

                a = St_Apply(env, a, head);

                // return
                f = index(s, len + 0)->int_value;
                x = index(s, len + 1);
                s = s - len - 3;
            }
            else
            {
                ST_ARGS2("apply", a, body, link);
                x = body;
                e = s;
                s = push(link, s);
            }
            continue;
        }

        CASE(x, rtn) {
            ST_ARGS1("return", ST_CDR(x), n);
            int s2 = s - n->int_value;
            f = index(s2, 0)->int_value;
            x = index(s2, 1);
            e = index(s2, 2)->int_value;
            s = s2 - 3;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *env, Object *obj)
{
    return vm(env, St_Compile(obj, env, ST_LIST1(St_Intern("halt"))));
}
