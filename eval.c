#include "lisp.h"

static Object *eval(Object *env, Object *obj);

static Object *map_eval(Object *env, Object *args)
{
    Object *head = Nil;
    Object *tail = Nil;

    ST_FOREACH(p, args) {
        Object *val = St_Eval(env, p->car);

        ST_APPEND1(head, tail, val);
    }

    return head;
}

static Object *apply(Object *env, Object *proc, Object *args)
{
    Object *internal_env = St_PushEnv(proc->env, proc->params, args);
    Object *value = Nil;

    ST_FOREACH(p, proc->body) {
        value = eval(internal_env, p->car);
    }

    return value;
}

Object *St_Apply(Object *env, Object *proc, Object *args)
{
    if (ST_LAMBDAP(proc))
    {
        return apply(env, proc, args);
    }
    if (ST_SUBRP(proc))
    {
        return proc->subr(env, args);
    }

    St_Error("apply: lambda or subr requried");
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
        if (!St_ListP(obj))
        {
            St_Error("eval: invalid application");
        }

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
            return apply(env, fst, args);
        }
        if (ST_MACROP(fst))
        {
            Object *proc = fst->proc;
            return St_Eval(env, apply(env, proc, ST_CDR(obj)));
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
