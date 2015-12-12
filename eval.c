#include "lisp.h"

static Object *eval(Object *env, Object *obj);

static Object *map_eval(Object *env, Object *args)
{
    Object *head = Nil;
    Object *tail = Nil;

    for (Object *p = args; !ST_NULLP(p); p = p->cdr) {
        Object *val = St_Eval(env, p->car);

        ST_APPEND1(head, tail, val);
    }

    return head;
}

static Object *apply(Object *env, Object *proc, Object *args)
{
    Object *internal_env = St_PushEnv(proc->env, proc->params, args);
    Object *value = Nil;

    for (Object *p = proc->body; !ST_NULLP(p); p = p->cdr) {
        value = eval(internal_env, p->car);
    }

    return value;
}

static Object *eval(Object *env, Object *obj)
{
    switch (obj->type) {
    case TINT:
    case TNIL:
    case TTRUE:
    case TFALSE:
    case TSYNTAX:
    case TSUBR:
    case TLAMBDA:
        return obj;

    case TSYMBOL: // variable
        return St_LookupVariable(env, obj);
        
    case TCELL: { // function application or syntax
        Object *fst = eval(env, obj->car);
        if (ST_SYNTAXP(fst))
        {
            return fst->syntax(env, obj);
        }
        if (ST_SUBRP(fst))
        {
            Object *args = map_eval(env, obj->cdr);
            return fst->subr(env, args);
        }
        if (ST_LAMBDAP(fst))
        {
            Object *args = map_eval(env, obj->cdr);
            Object *internal_env = St_PushEnv(fst->env, fst->params, args);
            Object *value = Nil;

            for (Object *p = fst->body; !ST_NULLP(p); p = p->cdr) {
                value = eval(internal_env, p->car);
            }

            return value;
        }
        if (ST_MACROP(fst))
        {
            Object *proc = fst->proc;

            if (St_Length(proc->params) != 1)
            {
                St_Error("macro proc must have just one parameter.");
            }

            return St_Eval(env, apply(env, proc, obj));
        }
    }
    default:
        return Nil; // not reached
    }
}

Object *St_Eval(Object *env, Object *obj)
{
    return eval(env, obj);
}
