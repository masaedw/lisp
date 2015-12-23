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

    // insns
#define INSN(x) Object *x = St_Intern(#x)
    INSN(halt);
    INSN(refer);
    INSN(constant);
    INSN(close);
    INSN(test);
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
            ST_ARGS2("refer", var, x2);
            a = St_LookupVariable(e, var);
            x = x2;
            continue;
        }

        CASE(x, constant) {
            ST_ARGS2("constant", obj, x2);
            a = obj;
            x = x2;
            continue;
        }

        CASE(x, close) {
            ST_ARGS3("close", vars, body, x2);
            a = make_closure(body, e, vars);
            x = x2;
            continue;
        }

        CASE(x, test) {
            ST_ARGS2("test", thenc, elsec);
            x = a != False ? thenc : elsec;
            continue;
        }

        CASE(x, assign) {
            ST_ARGS2("assign", var, x2);
            ST_CAR_SET(St_LookupVariablePair(e, var), a);
            x = x2;
            continue;
        }

        CASE(x, conti) {
            ST_ARG1("conti", x2);
            a = make_continuation(s);
            x = x2;
            continue;
        }

        CASE(x, nuate) {
            ST_ARG2("nuate", s2, var);
            a = St_LookupVariable(e, var);
            x = ST_LIST1(rtn);
            s = s2;
            continue;
        }

        CASE(x, frame) {
            ST_ARG2("frame", ret, x2);
            r = Nil;
            s = make_call_frame(ret, e, r, s);
            x = x2;
            continue;
        }

        CASE(x, argument) {
            ST_ARG1("argument", x2);
            r = St_Cons(a, r);
            x = x2;
            continue;
        }

        CASE(x, apply) {
            
        }

        CASE(x, rtn) {
        }
    }
}
