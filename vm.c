#include "lisp.h"

Object *make_closure(Object *body, Object *e, Object *vars)
{
    return ST_LIST3(body, e, vars);
}

Object *make_continuation(Object *s)
{
    return make_closure(ST_LIST3(St_Intern("nuate"), s, St_Intern("v")), Nil, ST_LIST1(St_Intern("v")));
}

Object *make_call_frame(Object *x, Object *e, Object *r, Object *s)
{
    return ST_LIST4(x, e, r, s);
}

Object *vm(Object *env, Object *insn)
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
            a = St_LookupVariable(e, var);
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
            ST_ARGS3("close", ST_CDR(x), vars, body, x2);
            a = make_closure(body, e, vars);
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

            Object *pair = St_LookupVariablePair(e, var);

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
            a = St_LookupVariable(e, var);
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
                ST_ARGS3("apply", a, body, e2, vars);
                x = body;
                e = St_PushEnv(e2, vars, r);
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
    return vm(env, St_Compile(obj, ST_LIST1(St_Intern("halt"))));
}
