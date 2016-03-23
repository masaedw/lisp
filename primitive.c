#include <stdio.h>

#include "lisp.h"
#include "subr.h"

static StObject subr_plus(StCallInfo *cinfo)
{
    int value = 0;

    ST_ARG_FOREACH(i, 0) {
        ARG(o, i);

        if (!ST_INTP(o))
        {
            St_Error("+: invalid type");
        }

        value += ST_INT_VALUE(o);
    }

    return St_Integer(value);
}

static StObject subr_minus(StCallInfo *cinfo)
{
    switch (cinfo->count) {
    case 0: {
        St_Error("-: wrong number of arguments");
    }
    case 1: {
        ARG(operand, 0);

        if (!ST_INTP(operand))
        {
            St_Error("-: invalid type");
        }

        return St_Integer(-ST_INT_VALUE(operand));
    }
    default: {
        ARG(head, 0);

        if (!ST_INTP(head))
        {
            St_Error("-: invalid type");
        }

        int value = ST_INT_VALUE(head);

        ST_ARG_FOREACH(i, 1) {
            ARG(o, i);

            if (!ST_INTP(o))
            {
                St_Error("-: invalid type");
            }

            value -= ST_INT_VALUE(o);
        }

        return St_Integer(value);
    }
    }
}

static StObject subr_mul(StCallInfo *cinfo)
{
    int value = 1;

    ST_ARG_FOREACH(i, 0) {
        ARG(o, i);

        if (!ST_INTP(o))
        {
            St_Error("*: invalid type");
        }

        value *= ST_INT_VALUE(o);
    }

    return St_Integer(value);
}

static StObject subr_div(StCallInfo *cinfo)
{
    switch (cinfo->count) {
    case 0: {
        St_Error("/: wrong number of arguments");
    }
    case 1: {
        ARG(operand, 0);

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
    default: {
        ARG(head, 0);
        if (!ST_INTP(head))
        {
            St_Error("/: invalid type");
        }

        int value = ST_INT_VALUE(head);

        ST_ARG_FOREACH(i, 1) {
            ARG(o, i);
            if (!ST_INTP(o))
            {
                St_Error("-: invalid type");
            }

            if (ST_INT_VALUE(o) == 0)
            {
                St_Error("division by zero");
            }

            value /= ST_INT_VALUE(o);
        }
        return St_Integer(value);
    }
    }
}

#define DEFINE_NUMERIC_COMPARISON(fname, op, sym)                       \
    static StObject subr_##fname(StCallInfo *cinfo)                     \
    {                                                                   \
        if (cinfo->count < 2)                                           \
        {                                                               \
            St_Error(#sym ": wrong number of arguments");               \
        }                                                               \
                                                                        \
        ARG(fst, 0);                                                    \
        ARG(snd, 1);                                                    \
                                                                        \
        if (!ST_INTP(fst) || !ST_INTP(snd))                             \
        {                                                               \
            St_Error(#sym ": invalid type");                            \
        }                                                               \
                                                                        \
        bool r = ST_INT_VALUE(fst) op ST_INT_VALUE(snd);                \
        int last = ST_INT_VALUE(snd);                                   \
        ST_ARG_FOREACH(i, 2) {                                          \
            if (!r)                                                     \
            {                                                           \
                return False;                                           \
            }                                                           \
                                                                        \
            ARG(o, i);                                                  \
                                                                        \
            if (!ST_INTP(o))                                            \
            {                                                           \
                St_Error(#sym ": invalid type");                        \
            }                                                           \
                                                                        \
            r = last op ST_INT_VALUE(o);                                \
            last = ST_INT_VALUE(o);                                     \
        }                                                               \
                                                                        \
        return ST_BOOLEAN(r);                                           \
    }

DEFINE_NUMERIC_COMPARISON(lt, <, <)
DEFINE_NUMERIC_COMPARISON(le, <=, <=)
DEFINE_NUMERIC_COMPARISON(gt, >, >)
DEFINE_NUMERIC_COMPARISON(ge, >=, >=)
DEFINE_NUMERIC_COMPARISON(numeric_eq, ==, =)

static StObject subr_numberp(StCallInfo *cinfo)
{
    ST_ARGS1("number?", cinfo, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static StObject subr_integerp(StCallInfo *cinfo)
{
    ST_ARGS1("integer?", cinfo, o);

    return ST_BOOLEAN(ST_INTP(o));
}

static StObject subr_zerop(StCallInfo *cinfo)
{
    ST_ARGS1("zero?", cinfo, o);

    if (!ST_INTP(o))
    {
        St_Error("zero?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) == 0);
}

static StObject subr_positivep(StCallInfo *cinfo)
{
    ST_ARGS1("positive?", cinfo, o);

    if (!ST_INTP(o))
    {
        St_Error("positive?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) > 0);
}

static StObject subr_negativep(StCallInfo *cinfo)
{
    ST_ARGS1("negative?", cinfo, o);

    if (!ST_INTP(o))
    {
        St_Error("negative?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) < 0);
}

static StObject subr_oddp(StCallInfo *cinfo)
{
    ST_ARGS1("odd?", cinfo, o);

    if (!ST_INTP(o))
    {
        St_Error("odd?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) % 2 != 0);
}

static StObject subr_evenp(StCallInfo *cinfo)
{
    ST_ARGS1("even?", cinfo, o);

    if (!ST_INTP(o))
    {
        St_Error("even?: invalid type");
    }

    return ST_BOOLEAN(ST_INT_VALUE(o) % 2 == 0);
}

static StObject subr_print(StCallInfo *cinfo)
{
    ST_ARG_FOREACH(i, 0) {
        St_Display(St_Arg(cinfo, i), False);
    }

    St_Newline(False);

    return Nil;
}

static StObject subr_eqp(StCallInfo *cinfo)
{
    ST_ARGS2("eq?", cinfo, lhs, rhs);

    return ST_BOOLEAN(lhs == rhs);
}

static StObject subr_eqvp(StCallInfo *cinfo)
{
    ST_ARGS2("eqv?", cinfo, lhs, rhs);

    return ST_BOOLEAN(St_EqvP(lhs, rhs));
}

static StObject subr_equalp(StCallInfo *cinfo)
{
    ST_ARGS2("equal?", cinfo, lhs, rhs);

    return ST_BOOLEAN(St_EqualP(lhs, rhs));
}

static StObject subr_nullp(StCallInfo *cinfo)
{
    ST_ARGS1("null?", cinfo, o);

    return ST_BOOLEAN(ST_NULLP(o));
}

static StObject subr_pairp(StCallInfo *cinfo)
{
    ST_ARGS1("pair?", cinfo, o);

    return ST_BOOLEAN(ST_PAIRP(o));
}

static StObject subr_symbolp(StCallInfo *cinfo)
{
    ST_ARGS1("symbol?", cinfo, o);

    return ST_BOOLEAN(ST_SYMBOLP(o));
}

static StObject subr_symbol_string(StCallInfo *cinfo)
{
    ST_ARGS1("symbol->string", cinfo, s);

    if (!ST_SYMBOLP(s))
    {
        St_Error("symbol->string: symbol required");
    }

    return St_SymbolToString(s);
}

static StObject subr_string_symbol(StCallInfo *cinfo)
{
    ST_ARGS1("string->symbol", cinfo, s);

    if (!ST_STRINGP(s))
    {
        St_Error("string->symbol: string required");
    }

    return St_StringToSymbol(s);
}

static StObject subr_symbol_equalp(StCallInfo *cinfo)
{
    if (cinfo->count < 2)
    {
        St_Error("symbol=?: wrong number of arguments: at least 2 arguments required but got %d", cinfo->count);
    }

    ARG(a, 0);

    if (!ST_SYMBOLP(a))
    {
        St_Error("symbol=?: symbol required");
    }

    ST_ARG_FOREACH(i, 1)
    {
        ARG(o, i);
        if (!ST_SYMBOLP(o))
        {
            St_Error("symbol=?: symbol required");
        }
        if (a != o)
        {
            return False;
        }
    }

    return True;
}

static StObject subr_not(StCallInfo *cinfo)
{
    ST_ARGS1("not", cinfo, o);

    return ST_BOOLEAN(ST_FALSEP(o));
}

static StObject subr_cons(StCallInfo *cinfo)
{
    ST_ARGS2("cons", cinfo, car, cdr);

    return St_Cons(car, cdr);
}

static StObject subr_acons(StCallInfo *cinfo)
{
    ST_ARGS3("acons", cinfo, s, v, cdr);

    return St_Acons(s, v, cdr);
}

static StObject subr_append(StCallInfo *cinfo)
{
    if (cinfo->count == 0)
    {
        St_Error("append: wrong number of arguments: at least 1 argument required");
    }

    StObject r = St_Arg(cinfo, cinfo->count - 1);

    for (int i = cinfo->count - 2; i >= 0; i--) {
        ARG(o, i);
        if (!St_ListP(o))
        {
            St_Print(o, False);
            St_Error("append: list required");
        }
        r = St_Append(o, r);
    }

    return r;
}

static StObject subr_car(StCallInfo *cinfo)
{
    ST_ARGS1("car", cinfo, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("car: pair required");
    }

    return ST_CAR(cell);
}

static StObject subr_cdr(StCallInfo *cinfo)
{
    ST_ARGS1("cdr", cinfo, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("cdr: pair required");
    }

    return ST_CDR(cell);
}

static StObject subr_list(StCallInfo *cinfo)
{
    StObject x = Nil;
    for (int i = cinfo->count - 1; i >= 0; i--) {
        ARG(o, i);
        x = St_Cons(o, x);
    }
    return x;
}

static StObject subr_length(StCallInfo *cinfo)
{
    ST_ARGS1("length", cinfo, list);

    if (!St_ListP(list))
    {
        St_Error("length: must a list");
    }

    return St_Integer(St_Length(list));
}

static StObject subr_listp(StCallInfo *cinfo)
{
    ST_ARGS1("list?", cinfo, o);

    return ST_BOOLEAN(St_ListP(o));
}

static StObject subr_dotted_listp(StCallInfo *cinfo)
{
    ST_ARGS1("dotted-list?", cinfo, o);

    return ST_BOOLEAN(St_DottedListP(o));
}

static StObject subr_set_car(StCallInfo *cinfo)
{
    ST_ARGS2("set-car!", cinfo, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-car!: pair requied.");
    }

    ST_CAR_SET(pair, value);

    return value;
}

static StObject subr_set_cdr(StCallInfo *cinfo)
{
    ST_ARGS2("set-cdr!", cinfo, pair, value);

    if (!ST_PAIRP(pair))
    {
        St_Error("set-cdr!: pair requied.");
    }

    ST_CDR_SET(pair, value);

    return value;
}

static StObject subr_vectorp(StCallInfo *cinfo)
{
    ST_ARGS1("vector?", cinfo, v);

    return ST_BOOLEAN(ST_VECTORP(v));
}

static StObject subr_make_vector(StCallInfo *cinfo)
{
    StObject fill = Nil;
    StObject size = Unbound;

    switch (cinfo->count) {
    case 2:
        fill = St_Arg(cinfo, 1);
    case 1:
        size = St_Arg(cinfo, 0);

        if (!ST_INTP(size) || ST_INT_VALUE(size) < 0)
        {
            St_Error("make-vector: size must be a positive integer");
        }

        return St_MakeVectorWithInitValue(ST_INT_VALUE(size), fill);
    default:
        St_Error("make-vector: wrong number of arguments");
    }
}

static StObject subr_vector(StCallInfo *cinfo)
{
    ST_ARGS1("vector", cinfo, list);

    if (!St_ListP(list))
    {
        St_Error("vector: list required.");
    }

    return St_MakeVectorFromList(list);
}

static StObject subr_vector_ref(StCallInfo *cinfo)
{
    ST_ARGS2("vector-ref", cinfo, v, idx);

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

static StObject subr_vector_set(StCallInfo *cinfo)
{
    ST_ARGS3("vector-set!", cinfo, v, idx, obj);

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

static StObject subr_vector_length(StCallInfo *cinfo)
{
    ST_ARGS1("vector-length", cinfo, v);

    if (!ST_VECTORP(v))
    {
        St_Error("vector-length: vector required.");
    }

    return St_Integer(St_VectorLength(v));
}

static StObject subr_compile(StCallInfo *cinfo)
{
    ST_ARGS2("compile", cinfo, expr, next);

    return St_Compile(expr, GlobalModule, next);
}

static StObject subr_eval_vm(StCallInfo *cinfo)
{
    ST_ARGS1("eval-vm", cinfo, expr);

    return St_Eval_VM(GlobalModule, expr);
}

static StObject subr_set_memberp(StCallInfo *cinfo)
{
    ST_ARGS2("set-member?", cinfo, obj, set);

    return ST_BOOLEAN(St_SetMemberP(obj, set));
}

static StObject subr_set_cons(StCallInfo *cinfo)
{
    ST_ARGS2("set-cons", cinfo, obj, set);

    return St_SetCons(obj, set);
}

static StObject subr_set_union(StCallInfo *cinfo)
{
    ST_ARGS2("set-union", cinfo, s1, s2);

    return St_SetUnion(s1, s2);
}

static StObject subr_set_minus(StCallInfo *cinfo)
{
    ST_ARGS2("set-minus", cinfo, s1, s2);

    return St_SetMinus(s1, s2);
}

static StObject subr_set_intersect(StCallInfo *cinfo)
{
    ST_ARGS2("set-intersect", cinfo, s1, s2);

    return St_SetIntersect(s1, s2);
}

static StObject subr_stringp(StCallInfo *cinfo)
{
    ST_ARGS1("string?", cinfo, o);

    return ST_BOOLEAN(ST_STRINGP(o));
}

static StObject subr_make_string(StCallInfo *cinfo)
{
    ST_ARGS1("make-string", cinfo, len);

    if (!ST_INTP(len))
    {
        St_Error("integer required");
    }

    return St_MakeEmptyString(ST_INT_VALUE(len));
}

static StObject subr_string_length(StCallInfo *cinfo)
{
    ST_ARGS1("string-length", cinfo, s);

    if (!ST_STRINGP(s))
    {
        St_Error("string-length: string required");
    }

    return St_Integer(St_StringLength(s));
}

static StObject subr_string_append(StCallInfo *cinfo)
{
    StObject x = Nil;

    for (int i = cinfo->count - 1; i >= 0; i--) {
        ARG(o, i);
        if (!ST_STRINGP(o))
        {
            St_Error("string-append: string required");
        }
        x = St_Cons(o, x);
    }

    return St_StringAppend(x);
}

static StObject subr_string_equalp(StCallInfo *cinfo)
{
    if (cinfo->count < 2)
    {
        St_Error("string=?: wrong number of argumats, at least 2 argument required");
    }

    ST_ARG_FOREACH(i, 0) {
        ARG(o, i);
        if (!ST_STRINGP(o))
        {
            St_Error("string=?: string required");
        }
    }

    ARG(fst, 0);

    ST_ARG_FOREACH(i, 1) {
        ARG(o, i);
        if (!St_StringEqualP(fst, o))
        {
            return False;
        }
    }

    return True;
}

static StObject subr_apply(StCallInfo *cinfo)
{
    if (cinfo->count < 2)
    {
        St_Error("apply: wrong number of arguments");
    }

    ARG(proc, 0);

    if (!ST_PROCEDUREP(proc))
    {
        St_Error("required procedure");
    }

    ARG(x, cinfo->count - 1);

    if (!St_ListP(x))
    {
        St_Error("required proper list");
    }

    for (int i = cinfo->count - 2; i > 0; i--)
    {
        x = St_Cons(St_Arg(cinfo, i), x);
    }

    StObject v = St_MakeVectorFromList(x);

    return St_Apply(proc, &(StCallInfo){ ST_VECTOR(v), St_VectorLength(v), St_VectorLength(v) });
}

static StObject subr_macroexpand(StCallInfo *cinfo)
{
    ST_ARGS1("macroexpand", cinfo, expr);

    return St_MacroExpand(GlobalModule, expr);
}

static StObject subr_load(StCallInfo *cinfo)
{
    ST_ARGS1("load", cinfo, file);

    if (!ST_STRINGP(file))
    {
        St_Error("required string");
    }

    St_Load(St_StringGetCString(file));

    return True;
}

static StObject subr_eof_object(StCallInfo *cinfo)
{
    ST_ARGS0("eof-object", cinfo);

    return Eof;
}

static StObject subr_eof_objectp(StCallInfo *cinfo)
{
    ST_ARGS1("eof-object?", cinfo, o);

    return ST_BOOLEAN(ST_EOFP(o));
}

static StObject subr_assq(StCallInfo *cinfo)
{
    ST_ARGS2("assq", cinfo, obj, alist);

    return St_Assq(obj, alist);
}

static StObject subr_assv(StCallInfo *cinfo)
{
    ST_ARGS2("assv", cinfo, obj, alist);

    return St_Assv(obj, alist);
}

static StObject subr_memq(StCallInfo *cinfo)
{
    ST_ARGS2("memq", cinfo, obj, list);

    return St_Memq(obj, list);
}

static StObject subr_memv(StCallInfo *cinfo)
{
    ST_ARGS2("memv", cinfo, obj, list);

    return St_Memv(obj, list);
}

static StObject subr_bytevectorp(StCallInfo *cinfo)
{
    ST_ARGS1("bytevector?", cinfo, o);

    return ST_BOOLEAN(ST_BYTEVECTORP(o));
}

static StObject subr_make_bytevector(StCallInfo *cinfo)
{
    int byte = -1;
    switch (cinfo->count) {
    case 2: {
        ARG(b, 1);
        if (!ST_INTP(b))
        {
            St_Error("make-bytevector: integer requried");
        }
        byte = ST_INT_VALUE(b);
    }
    case 1: {
        ARG(k, 0);
        if (!ST_INTP(k))
        {
            St_Error("make-bytevector: integer requried");
        }
        return St_MakeBytevector(ST_INT_VALUE(k), byte);
    }
    default:
        St_Error("make-bytevector: wrong number of arguments");
    }
}

static StObject subr_bytevector(StCallInfo *cinfo)
{
    StObject b = St_MakeBytevector(cinfo->count, 0);

    ST_ARG_FOREACH(i, 0) {
        ARG(o, i);

        if (!ST_INTP(o))
        {
            St_Error("bytevector: integer required");
        }

        St_BytevectorU8Set(b, i, ST_INT_VALUE(o));
    }

    return b;
}

static StObject subr_bytevector_length(StCallInfo *cinfo)
{
    ST_ARGS1("bytevector-length", cinfo, o);

    if (!ST_BYTEVECTORP(o))
    {
        St_Error("bytevector-length: bytevector requried");
    }

    return St_Integer(St_BytevectorLength(o));
}

static StObject subr_bytevector_u8_ref(StCallInfo *cinfo)
{
    ST_ARGS2("bytevector-u8-ref", cinfo, o, k);

    if (!ST_BYTEVECTORP(o))
    {
        St_Error("bytevector-length: bytevector requried");
    }

    if (!ST_INTP(k))
    {
        St_Error("bytevector-length: integer required");
    }

    return St_Integer(St_BytevectorU8Ref(o, ST_INT_VALUE(k)));
}

static StObject subr_bytevector_u8_set(StCallInfo *cinfo)
{
    ST_ARGS3("bytevector-u8-set!", cinfo, o, k, v);

    if (!ST_BYTEVECTORP(o))
    {
        St_Error("bytevector-length: bytevector requried");
    }

    if (!ST_INTP(k))
    {
        St_Error("bytevector-length: integer required");
    }

    if (!ST_INTP(v))
    {
        St_Error("bytevector-length: integer required");
    }

    St_BytevectorU8Set(o, ST_INT_VALUE(k), ST_INT_VALUE(v));

    return Nil;
}

static StObject subr_bytevector_copy(StCallInfo *cinfo)
{
    int start = -1;
    int end = -1;

    switch (cinfo->count) {
    case 3: {
        ARG(oEnd, 2);
        if (!ST_INTP(oEnd))
        {
            St_Error("bytevector-copy: integer required");
        }
        end = ST_INT_VALUE(oEnd);
    }
    case 2: {
        ARG(oStart, 1);
        if (!ST_INTP(oStart))
        {
            St_Error("bytevector-copy: integer required");
        }
        start = ST_INT_VALUE(oStart);
    }
    case 1: {
        ARG(b, 0);
        if (!ST_BYTEVECTORP(b))
        {
            St_Error("bytevector-copy: bytevector required");
        }
        return St_MakeBytevectorFrom(b, start, end);
    }
    default:
        St_Error("bytevector-copy: wrong number of arguments");
    }
}

static StObject subr_bytevector_copyx(StCallInfo *cinfo)
{
    int start = -1;
    int end = -1;

    switch (cinfo->count) {
    case 5: {
        ARG(e, 4);
        if (!ST_INTP(e))
        {
            St_Error("bytevector-copy!: integer required 'end'");
        }
        end = ST_INT_VALUE(e);
    }
    case 4: {
        ARG(s, 3);
        if (!ST_INTP(s))
        {
            St_Error("bytevector-copy!: integer required 'start'");
        }
        start = ST_INT_VALUE(s);
    }
    case 3: {
        ARG(from, 2);
        if (!ST_BYTEVECTORP(from))
        {
            St_Error("bytevector-copy!: bytevector required 'from'");
        }
        ARG(a, 1);
        if (!ST_INTP(a))
        {
            St_Error("bytevector-copy!: integer required 'at'");
        }
        int at = ST_INT_VALUE(a);
        ARG(to, 0);
        if (!ST_BYTEVECTORP(to))
        {
            St_Error("bytevector-copy!: bytevector required 'to'");
        }
        St_BytevectorCopy(to, at, from, start, end);
        return Nil;
    }
    default:
        St_Error("bytevector-copy!: wrong number of arguments");
    }
}

static StObject subr_bytevector_append(StCallInfo *cinfo)
{
    StObject x = Nil;
    for (int i = cinfo->count - 1; i >= 0; i--) {
        ARG(o, i);
        if (!ST_BYTEVECTORP(o))
        {
            St_Error("bytevector-append: bytevector required");
        }
        x = St_Cons(o, x);
    }
    return St_BytevectorAppend(x);
}

static StObject current_x_port_impl(StCallInfo *cinfo, const char* name, StObject *port)
{
    switch (cinfo->count) {
    case 1: {
        ARG(np, 0);
        if (!ST_FDPORTP(np))
        {
            St_Error("%s: port required", name);
        }
        StObject tmp = *port;
        *port = np;
        return tmp;
    }
    case 0:
        return *port;
    default:
        St_Error("%s: wrong number of arguments", name);
    }
}

static StObject subr_current_input_port(StCallInfo *cinfo)
{
    return current_x_port_impl(cinfo, "current-input-port", &St_CurrentInputPort);
}

static StObject subr_current_output_port(StCallInfo *cinfo)
{
    return current_x_port_impl(cinfo, "current-output-port", &St_CurrentOutputPort);
}

static StObject subr_current_error_port(StCallInfo *cinfo)
{
    return current_x_port_impl(cinfo, "current-error-port", &St_CurrentErrorPort);
}

void St_InitPrimitives(void)
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
    St_AddSubr(m, "print", subr_print);
    St_AddSubr(m, "eq?", subr_eqp);
    St_AddSubr(m, "eqv?", subr_eqvp);
    St_AddSubr(m, "equal?", subr_equalp);
    St_AddSubr(m, "null?", subr_nullp);
    St_AddSubr(m, "pair?", subr_pairp);
    St_AddSubr(m, "symbol?", subr_symbolp);
    St_AddSubr(m, "symbol=?", subr_symbol_equalp);
    St_AddSubr(m, "symbol->string", subr_symbol_string);
    St_AddSubr(m, "string->symbol", subr_string_symbol);
    St_AddSubr(m, "not", subr_not);
    St_AddSubr(m, "cons", subr_cons);
    St_AddSubr(m, "acons", subr_acons);
    St_AddSubr(m, "append", subr_append);
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
    St_AddSubr(m, "vector", subr_vector);
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
    St_AddSubr(m, "assq", subr_assq);
    St_AddSubr(m, "assv", subr_assv);
    St_AddSubr(m, "memq", subr_memq);
    St_AddSubr(m, "memv", subr_memv);
    St_AddSubr(m, "bytevector?", subr_bytevectorp);
    St_AddSubr(m, "make-bytevector", subr_make_bytevector);
    St_AddSubr(m, "bytevector", subr_bytevector);
    St_AddSubr(m, "bytevector-length", subr_bytevector_length);
    St_AddSubr(m, "bytevector-u8-ref", subr_bytevector_u8_ref);
    St_AddSubr(m, "bytevector-u8-set!", subr_bytevector_u8_set);
    St_AddSubr(m, "bytevector-copy", subr_bytevector_copy);
    St_AddSubr(m, "bytevector-copy!", subr_bytevector_copyx);
    St_AddSubr(m, "bytevector-append", subr_bytevector_append);
    St_AddSubr(m, "current-input-port", subr_current_input_port);
    St_AddSubr(m, "current-output-port", subr_current_output_port);
    St_AddSubr(m, "current-error-port", subr_current_error_port);
}
