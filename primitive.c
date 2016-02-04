#include "lisp.h"

static Object *subr_plus(Object *env, Object *args)
{
    int value = 0;

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("+: invalid type");
        }

        value += ST_CAR(p)->integer.value;
    }

    return St_Integer(value);
}

static Object *subr_minus(Object *env, Object *args)
{
    int len = St_Length(args);

    if (len < 1)
    {
        St_Error("-: wrong number of arguments");
    }

    if (len == 1)
    {
        Object *operand = ST_CAR(args);
        if (!ST_INTP(operand))
        {
            St_Error("-: invalid type");
        }

        return St_Integer(-operand->integer.value);
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("-: invalid type");
    }

    int value = ST_CAR(args)->integer.value;

    ST_FOREACH(p, ST_CDR(args)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        value -= ST_CAR(p)->integer.value;
    }

    return St_Integer(value);
}

static Object *subr_mul(Object *env, Object *args)
{
    int value = 1;

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("*: invalid type");
        }

        value *= ST_CAR(p)->integer.value;
    }

    return St_Integer(value);
}

static Object *subr_div(Object *env, Object *args)
{
    int len = St_Length(args);

    if (len < 1)
    {
        St_Error("/: wrong number of arguments");
    }

    if (len == 1)
    {
        Object *operand = ST_CAR(args);
        if (!ST_INTP(operand))
        {
            St_Error("/: invalid type");
        }

        if (operand->integer.value == 0)
        {
            St_Error("division by zero");
        }

        return St_Integer(1 / operand->integer.value);
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("/: invalid type");
    }

    int value = ST_CAR(args)->integer.value;

    ST_FOREACH(p, ST_CDR(args)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        if (ST_CAR(p)->integer.value == 0)
        {
            St_Error("division by zero");
        }

        value /= ST_CAR(p)->integer.value;
    }

    return St_Integer(value);
}

#define DEFINE_NUMERIC_COMPARISON(fname, op, sym)                       \
    static Object *subr_##fname(Object *env, Object *args)              \
    {                                                                   \
        if (St_Length(args) < 2)                                        \
        {                                                               \
            St_Error(#sym ": wrong number of arguments");               \
        }                                                               \
                                                                        \
        Object *fst = ST_CAR(args);                                     \
        Object *snd = ST_CADR(args);                                    \
                                                                        \
        if (!ST_INTP(fst) || !ST_INTP(snd))                             \
        {                                                               \
            St_Error(#sym ": invalid type");                            \
        }                                                               \
                                                                        \
        bool r = fst->integer.value op snd->integer.value;              \
        int last;                                                       \
        Object *p;                                                      \
                                                                        \
        for (p = ST_CDDR(args), last = snd->integer.value; r && !ST_NULLP(p); last = ST_CAR(p)->integer.value, p = ST_CDR(p)) { \
            if (!ST_INTP(ST_CAR(p)))                                    \
            {                                                           \
                St_Error(#sym ": invalid type");                        \
            }                                                           \
                                                                        \
            r = last op ST_CAR(p)->integer.value;                       \
        }                                                               \
                                                                        \
        return ST_BOOLEAN(r);                                           \
    }

DEFINE_NUMERIC_COMPARISON(lt, <, <)
DEFINE_NUMERIC_COMPARISON(le, <=, <=)
DEFINE_NUMERIC_COMPARISON(gt, >, >)
DEFINE_NUMERIC_COMPARISON(ge, >=, >=)
DEFINE_NUMERIC_COMPARISON(numeric_eq, ==, =)

static Object *subr_numberp(Object *env, Object *args)
{
    ST_ARGS1("number?", args, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static Object *subr_integerp(Object *env, Object *args)
{
    ST_ARGS1("integer?", args, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static Object *subr_zerop(Object *env, Object *args)
{
    ST_ARGS1("zero?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("zero?: invalid type");
    }

    return ST_BOOLEAN(o->integer.value == 0);
}

static Object *subr_positivep(Object *env, Object *args)
{
    ST_ARGS1("positive?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("positive?: invalid type");
    }

    return ST_BOOLEAN(o->integer.value > 0); 
}

static Object *subr_negativep(Object *env, Object *args)
{
    ST_ARGS1("negative?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("negative?: invalid type");
    }

    return ST_BOOLEAN(o->integer.value < 0);
}

static Object *subr_oddp(Object *env, Object *args)
{
    ST_ARGS1("odd?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("odd?: invalid type");
    }

    return ST_BOOLEAN(o->integer.value % 2 != 0);
}

static Object *subr_evenp(Object *env, Object *args)
{
    ST_ARGS1("even?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("even?: invalid type");
    }

    return ST_BOOLEAN(o->integer.value % 2 == 0);
}

static Object *subr_display(Object *env, Object *args)
{
    ST_FOREACH(p, args) {
        St_Display(ST_CAR(p));
    }

    return Nil;
}

static Object *subr_print(Object *env, Object *args)
{
    ST_FOREACH(p, args) {
        St_Print(ST_CAR(p));
    }

    return Nil;
}

static Object *subr_newline(Object *env, Object *args)
{
    fprintf(stdout, "\n");
    return Nil;
}

static Object *subr_eqp(Object *env, Object *args)
{
    ST_ARGS2("eq?", args, lhs, rhs);

    return ST_BOOLEAN(lhs == rhs);
}

static Object *subr_eqvp(Object *env, Object *args)
{
    ST_ARGS2("eqv?", args, lhs, rhs);

    return ST_BOOLEAN(St_EqvP(lhs, rhs));
}

static Object *subr_equalp(Object *env, Object *args)
{
    ST_ARGS2("equal?", args, lhs, rhs);

    return ST_BOOLEAN(St_EqualP(lhs, rhs));
}

static Object *subr_nullp(Object *env, Object *args)
{
    ST_ARGS1("null?", args, o);

    return ST_BOOLEAN(ST_NULLP(o));
}

static Object *subr_pairp(Object *env, Object *args)
{
    ST_ARGS1("pair?", args, o);

    return ST_BOOLEAN(ST_PAIRP(o));
}

static Object *subr_symbolp(Object *env, Object *args)
{
    ST_ARGS1("symbol?", args, o);

    return ST_BOOLEAN(ST_SYMBOLP(o));
}

static Object *subr_not(Object *env, Object *args)
{
    ST_ARGS1("not", args, o);

    return ST_BOOLEAN(ST_FALSEP(o));
}

static Object *subr_cons(Object *env, Object *args)
{
    ST_ARGS2("cons", args, car, cdr);

    return St_Cons(car, cdr);
}

static Object *subr_car(Object *env, Object *args)
{
    ST_ARGS1("car", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("car: pair required");
    }

    return ST_CAR(cell);
}

static Object *subr_cdr(Object *env, Object *args)
{
    ST_ARGS1("cdr", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("cdr: pair required");
    }

    return ST_CDR(cell);
}

static Object *subr_list(Object *env, Object *args)
{
    return args;
}

static Object *subr_length(Object *env, Object *args)
{
    ST_ARGS1("length", args, list);

    if (!St_ListP(list))
    {
        St_Error("length: must a list");
    }

    return St_Integer(St_Length(list));
}

static Object *subr_listp(Object *env, Object *args)
{
    ST_ARGS1("list?", args, o);

    return ST_BOOLEAN(St_ListP(o));
}

static Object *subr_dotted_listp(Object *env, Object *args)
{
    ST_ARGS1("dotted-list?", args, o);

    return ST_BOOLEAN(St_DottedListP(o));
}

static Object *subr_set_car(Object *env, Object *args)
{
    ST_ARGS2("set-car!", args, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-car!: pair requied.");
    }

    ST_CAR_SET(pair, value);

    return value;
}

static Object *subr_set_cdr(Object *env, Object *args)
{
    ST_ARGS2("set-cdr!", args, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-cdr!: pair requied.");
    }

    ST_CDR_SET(pair, value);

    return value;
}

static Object *subr_vectorp(Object *env, Object *args)
{
    ST_ARGS1("vector?", args, v);

    return ST_BOOLEAN(ST_VECTORP(v));
}

static Object *subr_make_vector(Object *env, Object *args)
{
    ST_ARGS1("make-vector", args, size);

    if (!ST_INTP(size) || size->integer.value < 0) {
        St_Error("make-vector: size must be a positive integer");
    }

    return St_MakeVector(size->integer.value);
}

static Object *subr_vector_ref(Object *env, Object *args)
{
    ST_ARGS2("vector-ref", args, v, idx);

    if (!ST_VECTORP(v))
    {
        St_Error("vector-ref: vector required.");
    }

    if (!ST_INTP(idx))
    {
        St_Error("vector-ref: integer required.");
    }

    return St_VectorRef(v, idx->integer.value);
}

static Object *subr_vector_set(Object *env, Object *args)
{
    ST_ARGS3("vector-set!", args, v, idx, obj);

    if (!ST_VECTORP(v))
    {
        St_Error("vector-set!: vector required.");
    }

    if (!ST_INTP(idx))
    {
        St_Error("vector-set!: integer required.");
    }

    St_VectorSet(v, idx->integer.value, obj);

    return obj;
}

static Object *subr_vector_length(Object *env, Object *args)
{
    ST_ARGS1("vector-length", args, v);

    if (!ST_VECTORP(v))
    {
        St_Error("vector-length: vector required.");
    }

    return St_Integer(St_VectorLength(v));
}

static Object *subr_compile(Object *env, Object *args)
{
    ST_ARGS2("compile", args, expr, next);

    return St_Compile(expr, GlobalModule, env, next);
}

static Object *subr_eval_vm(Object *env, Object *args)
{
    ST_ARGS1("eval-vm", args, expr);

    return St_Eval_VM(GlobalModule, env, expr);
}

static Object *subr_set_memberp(Object *env, Object *args)
{
    ST_ARGS2("set-member?", args, obj, set);

    return ST_BOOLEAN(St_SetMemberP(obj, set));
}

static Object *subr_set_cons(Object *env, Object *args)
{
    ST_ARGS2("set-cons", args, obj, set);

    return St_SetCons(obj, set);
}

static Object *subr_set_union(Object *env, Object *args)
{
    ST_ARGS2("set-union", args, s1, s2);

    return St_SetUnion(s1, s2);
}

static Object *subr_set_minus(Object *env, Object *args)
{
    ST_ARGS2("set-minus", args, s1, s2);

    return St_SetMinus(s1, s2);
}

static Object *subr_set_intersect(Object *env, Object *args)
{
    ST_ARGS2("set-intersect", args, s1, s2);

    return St_SetIntersect(s1, s2);
}

static Object *subr_stringp(Object *env, Object *args)
{
    ST_ARGS1("string?", args, o);

    return ST_BOOLEAN(ST_STRINGP(o));
}

static Object *subr_make_string(Object *env, Object *args)
{
    ST_ARGS1("make-string", args, len);

    if (!ST_INTP(len))
    {
        St_Error("integer required");
    }

    return St_MakeEmptyString(len->integer.value);
}

static Object *subr_string_length(Object *env, Object *args)
{
    ST_ARGS1("string-length", args, s);

    if (!ST_STRINGP(s))
    {
        St_Error("string-length: string required");
    }

    return St_Integer(St_StringLength(s));
}

static Object *subr_string_append(Object *env, Object *args)
{
    ST_FOREACH(p, args) {
        if (!ST_STRINGP(ST_CAR(p)))
        {
            St_Error("string-append: string required");
        }
    }

    return St_StringAppend(args);
}

static Object *subr_string_equalp(Object *env, Object *args)
{
    int len = 0;

    ST_FOREACH(p, args) {
        len++;
        if (!ST_STRINGP(ST_CAR(p)))
        {
            St_Error("string=?: string required");
        }
    }

    if (len < 2)
    {
        St_Error("string=?: wrong number of argumats, at least 2 argument required");
    }

    Object *fst = ST_CAR(args);

    ST_FOREACH(p, ST_CDR(args)) {
        if (!St_StringEqualP(fst, ST_CAR(p)))
        {
            return False;
        }
    }

    return True;
}

static Object *subr_apply(Object *env, Object *args)
{
    int len = St_Length(args);
    if (len < 2)
    {
        St_Error("apply: wrong number of arguments");
    }

    Object *proc = ST_CAR(args);

    if (!ST_PROCEDUREP(proc))
    {
        St_Error("required procedure");
    }

    args = St_Reverse(ST_CDR(args));

    if (!St_ListP(ST_CAR(args)))
    {
        St_Error("required proper list");
    }

    Object *x = Nil;
    if (!ST_NULLP(args)) {
        x = ST_CAR(args);
        ST_FOREACH(p, ST_CDR(args)) {
            x = St_Cons(ST_CAR(p), x);
        }
    }

    return St_Apply(proc, x);
}

static Object *subr_macroexpand(Object *env, Object *args)
{
    ST_ARGS1("macroexpand", args, expr);

    return St_MacroExpand(GlobalModule, expr);
}

void St_InitPrimitives(Object *env)
{
    St_AddSubr(env, "+", subr_plus);
    St_AddSubr(env, "-", subr_minus);
    St_AddSubr(env, "*", subr_mul);
    St_AddSubr(env, "/", subr_div);
    St_AddSubr(env, "<", subr_lt);
    St_AddSubr(env, "<=", subr_le);
    St_AddSubr(env, ">", subr_gt);
    St_AddSubr(env, ">=", subr_ge);
    St_AddSubr(env, "=", subr_numeric_eq);
    St_AddSubr(env, "number?", subr_numberp);
    St_AddSubr(env, "integer?", subr_integerp);
    St_AddSubr(env, "zero?", subr_zerop);
    St_AddSubr(env, "positive?", subr_positivep);
    St_AddSubr(env, "negative?", subr_negativep);
    St_AddSubr(env, "odd?", subr_oddp);
    St_AddSubr(env, "even?", subr_evenp);
    St_AddSubr(env, "display", subr_display);
    St_AddSubr(env, "print", subr_print);
    St_AddSubr(env, "newline", subr_newline);
    St_AddSubr(env, "eq?", subr_eqp);
    St_AddSubr(env, "eqv?", subr_eqvp);
    St_AddSubr(env, "equal?", subr_equalp);
    St_AddSubr(env, "null?", subr_nullp);
    St_AddSubr(env, "pair?", subr_pairp);
    St_AddSubr(env, "symbol?", subr_symbolp);
    St_AddSubr(env, "not", subr_not);
    St_AddSubr(env, "cons", subr_cons);
    St_AddSubr(env, "car", subr_car);
    St_AddSubr(env, "cdr", subr_cdr);
    St_AddSubr(env, "list", subr_list);
    St_AddSubr(env, "length", subr_length);
    St_AddSubr(env, "list?", subr_listp);
    St_AddSubr(env, "dotted-list?", subr_dotted_listp);
    St_AddSubr(env, "set-car!", subr_set_car);
    St_AddSubr(env, "set-cdr!", subr_set_cdr);
    St_AddSubr(env, "vector?", subr_vectorp);
    St_AddSubr(env, "make-vector", subr_make_vector);
    St_AddSubr(env, "vector-ref", subr_vector_ref);
    St_AddSubr(env, "vector-set!", subr_vector_set);
    St_AddSubr(env, "vector-length", subr_vector_length);
    St_AddSubr(env, "compile", subr_compile);
    St_AddSubr(env, "eval-vm", subr_eval_vm);
    St_AddSubr(env, "set-member?", subr_set_memberp);
    St_AddSubr(env, "set-cons", subr_set_cons);
    St_AddSubr(env, "set-union", subr_set_union);
    St_AddSubr(env, "set-minus", subr_set_minus);
    St_AddSubr(env, "set-intersect", subr_set_intersect);
    St_AddSubr(env, "string?", subr_stringp);
    St_AddSubr(env, "make-string", subr_make_string);
    St_AddSubr(env, "string-length", subr_string_length);
    St_AddSubr(env, "string-append", subr_string_append);
    St_AddSubr(env, "string=?", subr_string_equalp);
    St_AddSubr(env, "apply", subr_apply);
    St_AddSubr(env, "macroexpand", subr_macroexpand);
}
