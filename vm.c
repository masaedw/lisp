#include "lisp.h"

static Object *make_closure(Object *body, Object *e)
{
    return ST_LIST2(body, e);
}

static Object *make_continuation(Object *s)
{
    return make_closure(ST_LIST3(St_Intern("nuate"), s, St_Cons(St_Integer(0), St_Integer(0))), Nil);
}

static Object *make_call_frame(Object *x, Object *e, Object *r, Object *s)
{
    return ST_LIST4(x, e, r, s);
}

static Object *lookup(Object *env, Object *access)
{
    int rib = ST_CAR(access)->int_value;
    int elt = ST_CDR(access)->int_value;

    Object *e = env;

    while (rib-- > 0) {
        e = ST_CAR(e);
    }

    Object *p = ST_CADR(e);

    while (elt-- > 0) {
        p = ST_CDR(p);
    }

    return ST_CAR(p);
}

static Object *extend(Object *env, Object *vals)
{
    Object *syms = Nil;

    for (Object *p = vals; !ST_NULLP(p); p = ST_CDR(p)) {
        syms = St_Cons(Nil, syms);
    }

    return St_PushEnv(env, syms, vals);
}

static Object *vm(Object *env, Object *insn)
{
    // registers
    Object *a; // Accumulator
    Object *x; // Next expression
    Object *e; // Current environment
    Object *r; // Current value rib
    Object *s; // Current stack

    a = Nil;
    x = insn;
    e = env;
    r = Nil;
    s = Nil;

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    INSN(refer);
    INSN(constant);
    INSN(close);
    INSN(test);
    INSN(define);
    INSN(assign);
    INSN(conti);
    INSN(nuate);
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
            ST_ARGS2("refer", ST_CDR(x), var, x2);
            a = ST_CDR(lookup(e, var));
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
            a = make_closure(body, e);
            x = x2;
            continue;
        }

        CASE(x, test) {
            ST_ARGS2("test", ST_CDR(x), thenc, elsec);
            x = a != False ? thenc : elsec;
            continue;
        }

        CASE(x, define) {
            ST_ARGS2("define", ST_CDR(x), var, x2);

            St_AddVariable(e, var, a);

            x = x2;
            continue;
        }

        CASE(x, assign) {
            ST_ARGS2("assign", ST_CDR(x), var, x2);

            Object *pair = lookup(e, var);

            if (pair == Nil)
            {
                St_Error("set!: unbound variable");
            }

            ST_CDR_SET(pair, a);
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
            ST_ARGS2("nuate", ST_CDR(x), s2, var);
            a = ST_CDR(lookup(e, var));
            x = ST_LIST1(rtn);
            s = s2;
            continue;
        }

        CASE(x, frame) {
            ST_ARGS2("frame", ST_CDR(x), ret, x2);
            s = make_call_frame(ret, e, r, s);
            r = Nil;
            x = x2;
            continue;
        }

        CASE(x, argument) {
            ST_ARGS1("argument", ST_CDR(x), x2);
            r = St_Cons(a, r);
            x = x2;
            continue;
        }

        CASE(x, apply) {
            if (ST_LAMBDAP(a) || ST_SUBRP(a)) // a lambda created in interpreter or subr
            {
                a = St_Apply(env, a, r);
                goto label_rtn;
            }
            else
            {
                ST_ARGS2("apply", a, body, e2);
                x = body;
                e = extend(e2, r);
                r = Nil;
                continue;
            }
        }

        CASE(x, rtn) {
        label_rtn:
            ST_ARGS4("return", s, x2, e2, r2, s2);
            x = x2;
            e = e2;
            r = r2;
            s = s2;
            continue;
        }
    }
}

Object *St_Eval_VM(Object *env, Object *obj)
{
    return vm(env, St_Compile(obj, env, ST_LIST1(St_Intern("halt"))));
}
