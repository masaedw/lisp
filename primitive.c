#include "lisp.h"

static StObject subr_plus(StObject args)
{
    int value = 0;

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("+: invalid type");
        }

        value += ST_INT_VALUE(ST_CAR(p));
    }

    return St_Integer(value);
}

static StObject subr_minus(StObject args)
{
    int len = St_Length(args);

    if (len < 1)
    {
        St_Error("-: wrong number of arguments");
    }

    if (len == 1)
    {
        StObject operand = ST_CAR(args);
        if (!ST_INTP(operand))
        {
            St_Error("-: invalid type");
        }

        return St_Integer(-ST_INT_VALUE(operand));
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("-: invalid type");
    }

    int value = ST_INT_VALUE(ST_CAR(args));

    ST_FOREACH(p, ST_CDR(args)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        value -= ST_INT_VALUE(ST_CAR(p));
    }

    return St_Integer(value);
}

static StObject subr_mul(StObject args)
{
    int value = 1;

    ST_FOREACH(p, args) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("*: invalid type");
        }

        value *= ST_INT_VALUE(ST_CAR(p));
    }

    return St_Integer(value);
}

static StObject subr_div(StObject args)
{
    int len = St_Length(args);

    if (len < 1)
    {
        St_Error("/: wrong number of arguments");
    }

    if (len == 1)
    {
        StObject operand = ST_CAR(args);
        if (!ST_INTP(operand))
        {
            St_Error("/: invalid type");
        }

        if (ST_INT_VALUE(operand) == 0)
        {
            St_Error("division by zero");
        }

        return St_Integer(1 / ST_INT_VALUE(operand));
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("/: invalid type");
    }

    int value = ST_INT_VALUE(ST_CAR(args));

    ST_FOREACH(p, ST_CDR(args)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        if (ST_INT_VALUE(ST_CAR(p)) == 0)
        {
            St_Error("division by zero");
        }

        value /= ST_INT_VALUE(ST_CAR(p));
    }

    return St_Integer(value);
}

#define DEFINE_NUMERIC_COMPARISON(fname, op, sym)                       \
    static StObject subr_##fname(StObject args)                         \
    {                                                                   \
        if (St_Length(args) < 2)                                        \
        {                                                               \
            St_Error(#sym ": wrong number of arguments");               \
        }                                                               \
                                                                        \
        StObject fst = ST_CAR(args);                                    \
        StObject snd = ST_CADR(args);                                   \
                                                                        \
        if (!ST_INTP(fst) || !ST_INTP(snd))                             \
        {                                                               \
            St_Error(#sym ": invalid type");                            \
        }                                                               \
                                                                        \
        bool r = ST_INT_VALUE(fst) op ST_INT_VALUE(snd);                \
        int last;                                                       \
        StObject p;                                                     \
                                                                        \
        for (p = ST_CDDR(args), last = ST_INT_VALUE(snd); r && !ST_NULLP(p); last = ST_INT_VALUE(ST_CAR(p)), p = ST_CDR(p)) { \
            if (!ST_INTP(ST_CAR(p)))                                    \
            {                                                           \
                St_Error(#sym ": invalid type");                        \
            }                                                           \
                                                                        \
            r = last op ST_INT_VALUE(ST_CAR(p));                        \
        }                                                               \
                                                                        \
        return ST_BOOLEAN(r);                                           \
    }

DEFINE_NUMERIC_COMPARISON(lt, <, <)
DEFINE_NUMERIC_COMPARISON(le, <=, <=)
DEFINE_NUMERIC_COMPARISON(gt, >, >)
DEFINE_NUMERIC_COMPARISON(ge, >=, >=)
DEFINE_NUMERIC_COMPARISON(numeric_eq, ==, =)

static StObject subr_numberp(StObject args)
{
    ST_ARGS1("number?", args, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static StObject subr_integerp(StObject args)
{
    ST_ARGS1("integer?", args, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static StObject subr_zerop(StObject args)
{
    ST_ARGS1("zero?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("zero?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) == 0);
}

static StObject subr_positivep(StObject args)
{
    ST_ARGS1("positive?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("positive?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) > 0);
}

static StObject subr_negativep(StObject args)
{
    ST_ARGS1("negative?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("negative?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) < 0);
}

static StObject subr_oddp(StObject args)
{
    ST_ARGS1("odd?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("odd?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) % 2 != 0);
}

static StObject subr_evenp(StObject args)
{
    ST_ARGS1("even?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("even?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) % 2 == 0);
}

static StObject subr_display(StObject args)
{
    ST_FOREACH(p, args) {
        St_Display(ST_CAR(p));
    }

    return Nil;
}

static StObject subr_print(StObject args)
{
    ST_FOREACH(p, args) {
        St_Print(ST_CAR(p));
    }

    return Nil;
}

static StObject subr_newline(StObject args)
{
    fprintf(stdout, "\n");
    return Nil;
}

static StObject subr_eqp(StObject args)
{
    ST_ARGS2("eq?", args, lhs, rhs);

    return ST_BOOLEAN(lhs == rhs);
}

static StObject subr_eqvp(StObject args)
{
    ST_ARGS2("eqv?", args, lhs, rhs);

    return ST_BOOLEAN(St_EqvP(lhs, rhs));
}

static StObject subr_equalp(StObject args)
{
    ST_ARGS2("equal?", args, lhs, rhs);

    return ST_BOOLEAN(St_EqualP(lhs, rhs));
}

static StObject subr_nullp(StObject args)
{
    ST_ARGS1("null?", args, o);

    return ST_BOOLEAN(ST_NULLP(o));
}

static StObject subr_pairp(StObject args)
{
    ST_ARGS1("pair?", args, o);

    return ST_BOOLEAN(ST_PAIRP(o));
}

static StObject subr_symbolp(StObject args)
{
    ST_ARGS1("symbol?", args, o);

    return ST_BOOLEAN(ST_SYMBOLP(o));
}

static StObject subr_not(StObject args)
{
    ST_ARGS1("not", args, o);

    return ST_BOOLEAN(ST_FALSEP(o));
}

static StObject subr_cons(StObject args)
{
    ST_ARGS2("cons", args, car, cdr);

    return St_Cons(car, cdr);
}

static StObject subr_car(StObject args)
{
    ST_ARGS1("car", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("car: pair required");
    }

    return ST_CAR(cell);
}

static StObject subr_cdr(StObject args)
{
    ST_ARGS1("cdr", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("cdr: pair required");
    }

    return ST_CDR(cell);
}

static StObject subr_list(StObject args)
{
    return args;
}

static StObject subr_length(StObject args)
{
    ST_ARGS1("length", args, list);

    if (!St_ListP(list))
    {
        St_Error("length: must a list");
    }

    return St_Integer(St_Length(list));
}

static StObject subr_listp(StObject args)
{
    ST_ARGS1("list?", args, o);

    return ST_BOOLEAN(St_ListP(o));
}

static StObject subr_dotted_listp(StObject args)
{
    ST_ARGS1("dotted-list?", args, o);

    return ST_BOOLEAN(St_DottedListP(o));
}

static StObject subr_set_car(StObject args)
{
    ST_ARGS2("set-car!", args, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-car!: pair requied.");
    }

    ST_CAR_SET(pair, value);

    return value;
}

static StObject subr_set_cdr(StObject args)
{
    ST_ARGS2("set-cdr!", args, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-cdr!: pair requied.");
    }

    ST_CDR_SET(pair, value);

    return value;
}

static StObject subr_vectorp(StObject args)
{
    ST_ARGS1("vector?", args, v);

    return ST_BOOLEAN(ST_VECTORP(v));
}

static StObject subr_make_vector(StObject args)
{
    ST_ARGS1("make-vector", args, size);

    if (!ST_INTP(size) || ST_INT_VALUE(size) < 0) {
        St_Error("make-vector: size must be a positive integer");
    }

    return St_MakeVector(ST_INT_VALUE(size));
}

static StObject subr_vector_ref(StObject args)
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

    return St_VectorRef(v, ST_INT_VALUE(idx));
}

static StObject subr_vector_set(StObject args)
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

    St_VectorSet(v, ST_INT_VALUE(idx), obj);

    return obj;
}

static StObject subr_vector_length(StObject args)
{
    ST_ARGS1("vector-length", args, v);

    if (!ST_VECTORP(v))
    {
        St_Error("vector-length: vector required.");
    }

    return St_Integer(St_VectorLength(v));
}

static StObject subr_compile(StObject args)
{
    ST_ARGS2("compile", args, expr, next);

    return St_Compile(expr, GlobalModule, next);
}

static StObject subr_eval_vm(StObject args)
{
    ST_ARGS1("eval-vm", args, expr);

    return St_Eval_VM(GlobalModule, expr);
}

static StObject subr_set_memberp(StObject args)
{
    ST_ARGS2("set-member?", args, obj, set);

    return ST_BOOLEAN(St_SetMemberP(obj, set));
}

static StObject subr_set_cons(StObject args)
{
    ST_ARGS2("set-cons", args, obj, set);

    return St_SetCons(obj, set);
}

static StObject subr_set_union(StObject args)
{
    ST_ARGS2("set-union", args, s1, s2);

    return St_SetUnion(s1, s2);
}

static StObject subr_set_minus(StObject args)
{
    ST_ARGS2("set-minus", args, s1, s2);

    return St_SetMinus(s1, s2);
}

static StObject subr_set_intersect(StObject args)
{
    ST_ARGS2("set-intersect", args, s1, s2);

    return St_SetIntersect(s1, s2);
}

static StObject subr_stringp(StObject args)
{
    ST_ARGS1("string?", args, o);

    return ST_BOOLEAN(ST_STRINGP(o));
}

static StObject subr_make_string(StObject args)
{
    ST_ARGS1("make-string", args, len);

    if (!ST_INTP(len))
    {
        St_Error("integer required");
    }

    return St_MakeEmptyString(ST_INT_VALUE(len));
}

static StObject subr_string_length(StObject args)
{
    ST_ARGS1("string-length", args, s);

    if (!ST_STRINGP(s))
    {
        St_Error("string-length: string required");
    }

    return St_Integer(St_StringLength(s));
}

static StObject subr_string_append(StObject args)
{
    ST_FOREACH(p, args) {
        if (!ST_STRINGP(ST_CAR(p)))
        {
            St_Error("string-append: string required");
        }
    }

    return St_StringAppend(args);
}

static StObject subr_string_equalp(StObject args)
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

    StObject fst = ST_CAR(args);

    ST_FOREACH(p, ST_CDR(args)) {
        if (!St_StringEqualP(fst, ST_CAR(p)))
        {
            return False;
        }
    }

    return True;
}

static StObject subr_apply(StObject args)
{
    int len = St_Length(args);
    if (len < 2)
    {
        St_Error("apply: wrong number of arguments");
    }

    StObject proc = ST_CAR(args);

    if (!ST_PROCEDUREP(proc))
    {
        St_Error("required procedure");
    }

    args = St_Reverse(ST_CDR(args));

    if (!St_ListP(ST_CAR(args)))
    {
        St_Error("required proper list");
    }

    StObject x = Nil;
    if (!ST_NULLP(args)) {
        x = ST_CAR(args);
        ST_FOREACH(p, ST_CDR(args)) {
            x = St_Cons(ST_CAR(p), x);
        }
    }

    return St_Apply(proc, x);
}

static StObject subr_macroexpand(StObject args)
{
    ST_ARGS1("macroexpand", args, expr);

    return St_MacroExpand(GlobalModule, expr);
}

static StObject subr_load(StObject args)
{
    ST_ARGS1("load", args, file);

    if (!ST_STRINGP(file))
    {
        St_Error("required string");
    }

    St_Load(St_StringGetCString(file));

    return True;
}

static StObject subr_eof_object(StObject args)
{
    ST_ARGS0("eof-object", args);

    return Eof;
}

static StObject subr_eof_objectp(StObject args)
{
    ST_ARGS1("eof-object?", args, o);

    return ST_BOOLEAN(ST_EOFP(o));
}

void St_InitPrimitives()
{
    StObject m = GlobalModule;

    St_AddSubr(m, "+", subr_plus);
    St_AddSubr(m, "-", subr_minus);
    St_AddSubr(m, "*", subr_mul);
    St_AddSubr(m, "/", subr_div);
    St_AddSubr(m, "<", subr_lt);
    St_AddSubr(m, "<=", subr_le);
    St_AddSubr(m, ">", subr_gt);
    St_AddSubr(m, ">=", subr_ge);
    St_AddSubr(m, "=", subr_numeric_eq);
    St_AddSubr(m, "number?", subr_numberp);
    St_AddSubr(m, "integer?", subr_integerp);
    St_AddSubr(m, "zero?", subr_zerop);
    St_AddSubr(m, "positive?", subr_positivep);
    St_AddSubr(m, "negative?", subr_negativep);
    St_AddSubr(m, "odd?", subr_oddp);
    St_AddSubr(m, "even?", subr_evenp);
    St_AddSubr(m, "display", subr_display);
    St_AddSubr(m, "print", subr_print);
    St_AddSubr(m, "newline", subr_newline);
    St_AddSubr(m, "eq?", subr_eqp);
    St_AddSubr(m, "eqv?", subr_eqvp);
    St_AddSubr(m, "equal?", subr_equalp);
    St_AddSubr(m, "null?", subr_nullp);
    St_AddSubr(m, "pair?", subr_pairp);
    St_AddSubr(m, "symbol?", subr_symbolp);
    St_AddSubr(m, "not", subr_not);
    St_AddSubr(m, "cons", subr_cons);
    St_AddSubr(m, "car", subr_car);
    St_AddSubr(m, "cdr", subr_cdr);
    St_AddSubr(m, "list", subr_list);
    St_AddSubr(m, "length", subr_length);
    St_AddSubr(m, "list?", subr_listp);
    St_AddSubr(m, "dotted-list?", subr_dotted_listp);
    St_AddSubr(m, "set-car!", subr_set_car);
    St_AddSubr(m, "set-cdr!", subr_set_cdr);
    St_AddSubr(m, "vector?", subr_vectorp);
    St_AddSubr(m, "make-vector", subr_make_vector);
    St_AddSubr(m, "vector-ref", subr_vector_ref);
    St_AddSubr(m, "vector-set!", subr_vector_set);
    St_AddSubr(m, "vector-length", subr_vector_length);
    St_AddSubr(m, "compile", subr_compile);
    St_AddSubr(m, "eval-vm", subr_eval_vm);
    St_AddSubr(m, "set-member?", subr_set_memberp);
    St_AddSubr(m, "set-cons", subr_set_cons);
    St_AddSubr(m, "set-union", subr_set_union);
    St_AddSubr(m, "set-minus", subr_set_minus);
    St_AddSubr(m, "set-intersect", subr_set_intersect);
    St_AddSubr(m, "string?", subr_stringp);
    St_AddSubr(m, "make-string", subr_make_string);
    St_AddSubr(m, "string-length", subr_string_length);
    St_AddSubr(m, "string-append", subr_string_append);
    St_AddSubr(m, "string=?", subr_string_equalp);
    St_AddSubr(m, "apply", subr_apply);
    St_AddSubr(m, "macroexpand", subr_macroexpand);
    St_AddSubr(m, "load", subr_load);
    St_AddSubr(m, "eof-object", subr_eof_object);
    St_AddSubr(m, "eof-object?", subr_eof_objectp);
}
