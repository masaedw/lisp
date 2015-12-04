#include "lisp.h"

Object *Nil = &(Object) { TNIL };
Object *True = &(Object) { TTRUE };
Object *False = &(Object) { TFALSE };

Object *St_Alloc(int type)
{
    Object *obj = (Object *)St_Malloc(siseof(Object));

    obj->type = type;

    return obj;
}

Object *St_Cons(Object *car, Object *cdr)
{
    Object *cell = alloc(TCELL);

    cell->car = car;
    cell->cdr = cdr;

    return cell;
}

Object *St_Reverse(Object *list)
{
    if (!ST_PAIRP(list))
    {
        return Nil;
    }

    Object *p = list;
    Object *r = St_Cons(list->car, Nil);

    while (ST_PAIRP(p->cdr)) {
        p = p->cdr;
        r = St_Cons(p->car, r);
    }

    return r;
}
