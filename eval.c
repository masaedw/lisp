#include "lisp.h"

static Object *eval(Object *env, Object *obj);

static Object *map_eval(Object *env, Object *args)
{
    return args; // TODO
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
        return obj;

    case TSYMBOL: // variable
        return St_LookupVariable(env, obj);
        
    case TCELL: { // function application or syntax
        Object *fst = eval(env, obj->car);
        //printf(": %s %d\n", obj->car, fst->type);
        if (ST_SYNTAXP(fst))
        {
            return fst->syntax(env, obj);
        }
        if (ST_SUBRP(fst))
        {
            Object *args = map_eval(env, obj->cdr);
            return fst->subr(env, args);
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
