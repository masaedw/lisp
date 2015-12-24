#include "lisp.h"

static Object *syntax_if(Object *env, Object *form)
{
    // TODO: form validation

    // (if cond then else)

    Object *cond = form->cdr->car;
    Object *then_block = form->cdr->cdr->car;
    Object *else_block = Nil;

    if (ST_PAIRP(form->cdr->cdr->cdr))
    {
        else_block = form->cdr->cdr->cdr->car;
    }

    Object *result = St_Eval(env, cond);

    if (!ST_FALSEP(result))
    {
        return St_Eval(env, then_block);
    }
    else
    {
        return St_Eval(env, else_block);
    }
}

static Object *syntax_define(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 2)
    {
        St_Error("define: malformed define");
    }

    Object *sym = form->cdr->car;
    Object *body = form->cdr->cdr->car;

    if (!ST_SYMBOLP(sym))
    {
        St_Error("define: symbol required");
    }

    Object *value = St_Eval(env, body);

    St_AddVariable(env, sym, value);

    return sym;
}

static Object *syntax_quote(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 1)
    {
        St_Error("quote: malformed quote");
    }

    return form->cdr->car;
}

static Object *syntax_set(Object *env, Object *form)
{
    if (St_Length(ST_CDR(form)) != 2 || !ST_SYMBOLP(ST_CADR(form)))
    {
        St_Error("set!: malformed set!");
    }

    Object *symbol = ST_CADR(form);
    Object *value = St_Eval(env, ST_CAR(ST_CDDR(form)));

    Object *pair = St_LookupVariablePair(env, symbol);

    if (pair == Nil)
    {
        St_Error("set!: unbound variable");
    }

    ST_CDR_SET(pair, value);

    return value;
}

static bool nil_or_dotted_list_of_sybol(Object *obj)
{
    while (true) {
        if (ST_NULLP(obj))
        {
            return true;
        }

        if (ST_SYMBOLP(obj))
        {
            return true;
        }

        if (ST_PAIRP(obj))
        {
            if (!ST_SYMBOLP(ST_CAR(obj)))
            {
                return false;
            }
            obj = obj->cdr;
            continue;
        }

        return false;
    }
}

static Object *syntax_lambda(Object *env, Object *form)
{
    // (lambda <params> <body>)
    // <params> ::= (<symbol> *)
    // <body> ::<define> * <expr> +

    int len = St_Length(form->cdr);

    if (len < 2)
    {
        St_Error("lambda: malformed lambda");
    }

    Object *params = form->cdr->car;
    Object *body = form->cdr->cdr;

    if (!nil_or_dotted_list_of_sybol(params))
    {
        St_Error("lambda: params needs to be () or a list of symbols");
    }

    // TODO: validate body

    Object *lambda = St_Alloc(TLAMBDA);
    lambda->params = params;
    lambda->env = env;
    lambda->body = body;

    return lambda;
}

static Object *syntax_call_cc(Object *env, Object *form)
{
    return Nil; // TODO
}

static Object *syntax_define_macro(Object *env, Object *form)
{
    int len = St_Length(form->cdr);

    if (len != 2)
    {
        St_Error("define-macro: malformed define-macro");
    }

    Object *sym = form->cdr->car;
    Object *proc = St_Eval(env, form->cdr->cdr->car);

    if (!ST_SYMBOLP(sym))
    {
        St_Error("define-macro: needs a symbol");
    }

    if (!ST_LAMBDAP(proc))
    {
        St_Error("define-macro: needs a lambda");
    }

    Object *macro = St_Alloc(TMACRO);
    macro->proc = proc;
    macro->macro_symbol = sym;

    St_AddVariable(env, sym, macro);

    return sym;
}

static Object *syntax_begin(Object *env, Object *args)
{
    Object *value = Nil;

    for (Object *p = ST_CDR(args); !ST_NULLP(p); p = ST_CDR(p)) {
        value = St_Eval(env, ST_CAR(p));
    }

    return value;
}

static Object *syntax_and(Object *env, Object *args)
{
    Object *value = True;

    for (Object *p = ST_CDR(args); !ST_NULLP(p); p = ST_CDR(p)) {
        value = St_Eval(env, ST_CAR(p));

        if (ST_FALSEP(value))
        {
            break;
        }
    }

    return value;
}

static Object *syntax_or(Object *env, Object *args)
{
    Object *value = False;

    for (Object *p = ST_CDR(args); !ST_NULLP(p); p = ST_CDR(p)) {
        value = St_Eval(env, ST_CAR(p));

        if (!ST_FALSEP(value))
        {
            break;
        }
    }

    return value;
}

void validate_bindings(Object *args)
{
    if (ST_NULLP(args))
    {
        return;
    }

    if (!St_ListP(args))
    {
        St_Error("let: malformed bindings");
    }

    for (Object *p = args; !ST_NULLP(p); p = ST_CDR(p)) {
        Object *b = ST_CAR(p);
        if (!ST_PAIRP(b) || St_Length(b) != 2 || !ST_SYMBOLP(ST_CAR(b)))
        {
            St_Error("let: malformed binding");
        }
    }
}

static Object *syntax_let(Object *env, Object *args)
{
    // (let <bindings> <body>)
    // <bindings> ::= ((sym <expr>)*)

    if (St_Length(args) < 2)
    {
        St_Error("let: malformed let");
    }

    Object *bindings = ST_CADR(args);
    Object *body = ST_CDDR(args);

    validate_bindings(bindings);

    Object *pshead = Nil;
    Object *pstail = Nil;

    for (Object *p = bindings; !ST_NULLP(p); p = ST_CDR(p)) {
        ST_APPEND1(pshead, pstail, ST_CAAR(p));
    }

    Object *ashead = Nil;
    Object *astail = Nil;

    for (Object *p = bindings; !ST_NULLP(p); p = ST_CDR(p)) {
        ST_APPEND1(ashead, astail, St_Eval(env, ST_CAR(ST_CDAR(p))));
    }

    Object *internal_env = St_PushEnv(env, pshead, ashead);
    Object *value = Nil;

    for (Object *p = body; !ST_NULLP(p); p = ST_CDR(p)) {
        value = St_Eval(internal_env, ST_CAR(p));
    }

    return value;
}

static Object *subr_plus(Object *env, Object *args)
{
    int value = 0;

    for (Object *p = args; !ST_NULLP(p); p = ST_CDR(p)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("+: invalid type");
        }

        value += ST_CAR(p)->int_value;
    }

    Object *o = St_Alloc(TINT);
    o->int_value = value;

    return o;
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

        Object *o = St_Alloc(TINT);
        o->int_value = -operand->int_value;

        return o;
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("-: invalid type");
    }

    int value = ST_CAR(args)->int_value;

    for (Object *p = ST_CDR(args); !ST_NULLP(p); p = ST_CDR(p))
    {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        value -= ST_CAR(p)->int_value;
    }

    Object *o = St_Alloc(TINT);
    o->int_value = value;

    return o;
}

static Object *subr_mul(Object *env, Object *args)
{
    int value = 1;

    for (Object *p = args; !ST_NULLP(p); p = ST_CDR(p)) {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("*: invalid type");
        }

        value *= ST_CAR(p)->int_value;
    }

    Object *o = St_Alloc(TINT);
    o->int_value = value;

    return o;
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

        if (operand->int_value == 0)
        {
            St_Error("division by zero");
        }

        Object *o = St_Alloc(TINT);
        o->int_value = 1 / operand->int_value;

        return o;
    }

    // 2 <= len
    if (!ST_INTP(ST_CAR(args)))
    {
        St_Error("/: invalid type");
    }

    int value = ST_CAR(args)->int_value;

    for (Object *p = ST_CDR(args); !ST_NULLP(p); p = ST_CDR(p))
    {
        if (!ST_INTP(ST_CAR(p)))
        {
            St_Error("-: invalid type");
        }

        if (ST_CAR(p)->int_value == 0)
        {
            St_Error("division by zero");
        }

        value /= ST_CAR(p)->int_value;
    }

    Object *o = St_Alloc(TINT);
    o->int_value = value;

    return o;
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
        bool r = fst->int_value op snd->int_value;                      \
        int last;                                                       \
        Object *p;                                                      \
                                                                        \
        for (p = ST_CDDR(args), last = snd->int_value; r && !ST_NULLP(p); last = ST_CAR(p)->int_value, p = ST_CDR(p)) { \
            if (!ST_INTP(ST_CAR(p)))                                    \
            {                                                           \
                St_Error(#sym ": invalid type");                        \
            }                                                           \
                                                                        \
            r = last op ST_CAR(p)->int_value;                           \
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

    return ST_BOOLEAN(o->int_value == 0);
}

static Object *subr_positivep(Object *env, Object *args)
{
    ST_ARGS1("positive?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("positive?: invalid type");
    }

    return ST_BOOLEAN(o->int_value > 0);
}

static Object *subr_negativep(Object *env, Object *args)
{
    ST_ARGS1("negative?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("negative?: invalid type");
    }

    return ST_BOOLEAN(o->int_value < 0);
}

static Object *subr_oddp(Object *env, Object *args)
{
    ST_ARGS1("odd?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("odd?: invalid type");
    }

    return ST_BOOLEAN(o->int_value % 2 != 0);
}

static Object *subr_evenp(Object *env, Object *args)
{
    ST_ARGS1("even?", args, o);

    if (!ST_INTP(o))
    {
        St_Error("even?: invalid type");
    }

    return ST_BOOLEAN(o->int_value % 2 == 0);
}

static Object *subr_print(Object *env, Object *args)
{
    for (Object *p = args; !ST_NULLP(p); p = p->cdr) {
        St_Print(p->car);
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

    return cell->car;
}

static Object *subr_cdr(Object *env, Object *args)
{
    ST_ARGS1("cdr", args, cell);

    if (!ST_PAIRP(cell))
    {
        St_Error("cdr: pair required");
    }

    return cell->cdr;
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

    Object *o = St_Alloc(TINT);
    o->int_value = St_Length(list);
    return o;
}

static Object *subr_listp(Object *env, Object *args)
{
    ST_ARGS1("list?", args, o);

    return ST_BOOLEAN(St_ListP(o));
}

static Object *append_argument(Object *args)
{
    if (ST_NULLP(ST_CDR(args)))
    {
        if (!St_ListP(ST_CAR(args)))
        {
            St_Error("apply: proper list required.");
        }
        return ST_CAR(args);
    }

    return St_Cons(ST_CAR(args), append_argument(ST_CDR(args)));
}

static Object *subr_apply(Object *env, Object *args)
{
    int len = St_Length(args);

    if (len < 2)
    {
        St_Error("apply: wrong number of arguments");
    }

    Object *proc = ST_CAR(args);
    Object *real_args = append_argument(ST_CDR(args));

    return St_Apply(env, proc, real_args);
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

static Object *subr_compile(Object *env, Object *args)
{
    ST_ARGS2("compile", args, expr, next);

    return St_Compile(expr, next);
}

static Object *subr_eval_vm(Object *env, Object *args)
{
    ST_ARGS1("eval-vm", args, expr);

    return St_Eval_VM(env, expr);
}

void St_InitPrimitives(Object *env)
{
    St_AddSyntax(env, "if", syntax_if);
    St_AddSyntax(env, "define", syntax_define);
    St_AddSyntax(env, "quote", syntax_quote);
    St_AddSyntax(env, "set!", syntax_set);
    St_AddSyntax(env, "lambda", syntax_lambda);
    St_AddSyntax(env, "call/cc", syntax_call_cc);
    St_AddSyntax(env, "define-macro", syntax_define_macro);
    St_AddSyntax(env, "begin", syntax_begin);
    St_AddSyntax(env, "and", syntax_and);
    St_AddSyntax(env, "or", syntax_or);
    St_AddSyntax(env, "let", syntax_let);
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
    St_AddSubr(env, "print", subr_print);
    St_AddSubr(env, "newline", subr_newline);
    St_AddSubr(env, "eq?", subr_eqp);
    St_AddSubr(env, "eqv?", subr_eqvp);
    St_AddSubr(env, "equal?", subr_equalp);
    St_AddSubr(env, "null?", subr_nullp);
    St_AddSubr(env, "pair?", subr_pairp);
    St_AddSubr(env, "symbol?", subr_symbolp);
    St_AddSubr(env, "cons", subr_cons);
    St_AddSubr(env, "car", subr_car);
    St_AddSubr(env, "cdr", subr_cdr);
    St_AddSubr(env, "list", subr_list);
    St_AddSubr(env, "length", subr_length);
    St_AddSubr(env, "list?", subr_listp);
    St_AddSubr(env, "apply", subr_apply);
    St_AddSubr(env, "set-car!", subr_set_car);
    St_AddSubr(env, "set-cdr!", subr_set_cdr);
    St_AddSubr(env, "compile", subr_compile);
    St_AddSubr(env, "eval-vm", subr_eval_vm);
}
