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

static Object *make_functional(Object *body, int e)
{
    return ST_LIST2(body, St_Integer(e));
}

static int find_link(int e, int n)
{
    for (; n != 0; n--, index(e, -1))
    {
    }
    return e;
}

static Object *vm(Object *env, Object *insn)
{
    // registers
    Object *a; // Accumulator
    Object *x; // Next expression
    int e; // Current environment
    int s; // Current stack

    a = Nil;
    x = insn;
    e = 0;
    s = 0;

    stack = St_MakeVector(1000);

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    INSN(refer);
    INSN(constant);
    INSN(close);
    INSN(test);
    INSN(define);
    INSN(assign);
    INSN(frame);
    INSN(argument);
    INSN(apply);
    Object *rtn = St_Intern("return");
#undef INSN
    
#define CASE(x, insn) if (ST_CAR(x) == insn)

    while (true) {

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
            a = make_functional(body, e);
            x = x2;
            continue;
        }

        CASE(x, test) {
            ST_ARGS2("test", ST_CDR(x), thenc, elsec);
            x = a != False ? thenc : elsec;
            continue;
        }

        CASE(x, define) {
            ST_ARGS3("define", ST_CDR(x), n, m, x2);
            index_set(find_link(n->int_value, e), m->int_value, a);
            x = x2; 
            continue;
        }

        CASE(x, assign) {
            ST_ARGS3("assign", ST_CDR(x), n, m, x2);
            index_set(find_link(n->int_value, e), m->int_value, a);
            x = x2;
            continue;
        }

        CASE(x, frame) {
            ST_ARGS2("frame", ST_CDR(x), ret, x2);
            x = x2;
            s = push(ret, push(St_Integer(e), s));
            continue;
        }

        CASE(x, argument) {
            ST_ARGS1("argument", ST_CDR(x), x2);
            x = x2;
            s = push(a, s);
            continue;
        }

        CASE(x, apply) {
            ST_ARGS2("apply", a, body, link);
            x = body;
            e = s;
            s = push(link, s);
            continue;
        }

        CASE(x, rtn) {
            ST_ARGS1("return", ST_CDR(x), n);
            int s2 = s - n->int_value;
            x = index(s, 0);
            e = index(s, 1)->int_value;
            s = s2 - 2;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *env, Object *obj)
{
    return vm(env, St_Compile(obj, env, ST_LIST1(St_Intern("halt"))));
}
