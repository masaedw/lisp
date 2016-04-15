#include "expression.h"

static void display(StObject obj, StObject port);
static bool equalp(StObject lhs, StObject rhs);

StExternalTypeInfo StExpressionTypeInfo = (StExternalTypeInfo) { "<expression>", display, equalp };

// display

static void display_body(StObject vector, StObject port)
{
    int len = ST_VECTOR_LENGTH(vector);

    if (len == 0)
    {
        return;
    }

    for (int i = 0; i < len - 1; i++) {
        St_Display(ST_VECTOR_DATA(vector)[i], port);
        St_WriteCString(" ", port);
    }
    St_Display(ST_VECTOR_DATA(vector)[len], port);
}

static void display_list(StObject vector, StObject port)
{
    St_WriteCString("(", port);
    display_body(vector, port);
    St_WriteCString(")", port);
}

static void display_let(const char *sym, struct StXLet *let, StObject port)
{
    St_WriteCString("(", port);
    St_WriteCString(sym, port);
    St_WriteCString(" (", port);
    for (size_t i = 0; i < ST_VECTOR_LENGTH(let->bindings); i++) {
        St_WriteCString("(", port);
        St_Display(ST_CAR(ST_VECTOR_DATA(let->bindings)[i]), port);
        St_WriteCString(" ", port);
        St_Display(ST_CDR(ST_VECTOR_DATA(let->bindings)[i]), port);
        St_WriteCString(")", port);
    }
    St_WriteCString(")", port);

    size_t len = ST_VECTOR_LENGTH(let->defvars);
    if (len != 0)
    {
        St_WriteCString(" ", port);
    }

    for (size_t i = 0; i < len; i++) {
        St_WriteCString("(define ", port);
        St_Display(ST_VECTOR_DATA(let->defvars)[i], port);
        St_WriteCString(" ", port);
        St_Display(ST_VECTOR_DATA(let->defvals)[i], port);
        St_WriteCString(")", port);
        if (i < len - 1)
        {
            St_WriteCString(" ", port);
        }
    }

    if (ST_VECTOR_LENGTH(let->body) != 0)
    {
        St_WriteCString(" ", port);
    }

    display_body(let->body, port);
    St_WriteCString(")", port);
}

static void display_lambda(struct StXLambda *lambda, StObject port)
{
    St_WriteCString("(lambda ", port);
    if (lambda->dotted)
    {
        size_t len = ST_VECTOR_LENGTH(lambda->vars);
        if (len == 1)
        {
            St_Display(ST_VECTOR_DATA(lambda->vars)[0], port);
        }
        else
        {
            St_WriteCString("(", port);
            for (size_t i = 0; i < len - 1; i++) {
                St_Display(ST_VECTOR_DATA(lambda->vars)[i], port);
                St_WriteCString(" ", port);
            }
            St_WriteCString(". ", port);
            St_Display(ST_VECTOR_DATA(lambda->vars)[len], port);
            St_WriteCString(")", port);
        }
    }
    else
    {
        St_WriteCString("(", port);
        display_list(lambda->vars, port);
        St_WriteCString(")", port);
    }

    size_t len = ST_VECTOR_LENGTH(lambda->defvars);
    if (len != 0)
    {
        St_WriteCString(" ", port);
    }

    for (size_t i = 0; i < len; i++) {
        St_WriteCString("(define ", port);
        St_Display(ST_VECTOR_DATA(lambda->defvars)[i], port);
        St_WriteCString(" ", port);
        St_Display(ST_VECTOR_DATA(lambda->defvals)[i], port);
        St_WriteCString(")", port);
        if (i < len - 1)
        {
            St_WriteCString(" ", port);
        }
    }

    if (ST_VECTOR_LENGTH(lambda->body) != 0)
    {
        St_WriteCString(" ", port);
    }

    display_body(lambda->body, port);
    St_WriteCString(")", port);
}

static void display(StObject obj, StObject port)
{
    StExpression xobj = ST_EXPRESSION(obj);

    switch (xobj->xtype) {
    case XVALUE:
        St_Display(xobj->value.value, port);
        break;

    case XSYMBOL:
        St_Display(xobj->symbol.value, port);
        break;

    case XQUOTE:
        St_WriteCString("(quote ", port);
        St_Display(xobj->quote.value, port);
        St_WriteCString(")", port);
        break;

    case XLET:
        display_let("let", &xobj->let, port);
        break;

    case XLETSTAR:
        display_let("let*", &xobj->let, port);
        break;

    case XLETREC:
        display_let("letrec", &xobj->let, port);
        break;

    case XLETRECSTAR:
        display_let("letrec*", &xobj->let, port);
        break;

    case XLAMBDA:
        display_lambda(&xobj->lambda, port);
        break;

    case XBEGIN:
        St_WriteCString("(begin ", port);
        display_body(xobj->begin.body, port);
        St_WriteCString(")", port);
        break;

    case XIF:
        St_WriteCString("(if ", port);
        St_Display(xobj->xif.xpred, port);
        St_WriteCString(" ", port);
        St_Display(xobj->xif.xthen, port);
        if (!ST_NULLP(xobj->xif.xelse))
        {
            St_WriteCString(" ", port);
            St_Display(xobj->xif.xelse, port);
        }
        St_WriteCString(")", port);
        break;

    case XSET:
        St_WriteCString("(set! ", port);
        St_Display(xobj->set.symbol, port);
        St_WriteCString(" ", port);
        St_Display(xobj->set.value, port);
        St_WriteCString(")", port);
        break;

    case XCALLCC:
        St_WriteCString("(call/cc ", port);
        St_Display(xobj->callcc.lambda, port);
        St_WriteCString(")", port);
        break;

    case XDEFINE:
        St_WriteCString("(define ", port);
        St_Display(xobj->define.symbol, port);
        St_WriteCString(" ", port);
        St_Display(xobj->define.value, port);
        St_WriteCString(")", port);
        break;

    case XDEFINEMACRO:
        St_WriteCString("(define-macro ", port);
        St_Display(xobj->define_macro.symbol, port);
        St_WriteCString(" ", port);
        St_Display(xobj->define_macro.lambda, port);
        St_WriteCString(")", port);
        break;

    case XAND:
        St_WriteCString("(and ", port);
        display_body(xobj->list.exprs, port);
        St_WriteCString(")", port);
        break;

    case XOR:
        St_WriteCString("(or ", port);
        display_body(xobj->list.exprs, port);
        St_WriteCString(")", port);
        break;

    case XLIST:
        display_list(xobj->list.exprs, port);
        break;
    }
}

// equalp

static bool equalp(StObject lhs, StObject rhs)
{
    (void)lhs;
    (void)rhs;
    // TODO
    return false;
}

// utilities

static StExpression AllocX(StExpressionType xtype, size_t size)
{
    StExpression o = St_Alloc2(TEXTERNAL, offsetof(struct StExpressionRec, value) + size);
    o->xtype = xtype;
    return o;
}

StExpression St_MakeExpression(StExpressionType xtype)
{
    switch (xtype) {
    case XVALUE:       return AllocX(xtype, sizeof(struct StXValue));
    case XSYMBOL:      return AllocX(xtype, sizeof(struct StXValue));
    case XQUOTE:       return AllocX(xtype, sizeof(struct StXValue));
    case XLET:         return AllocX(xtype, sizeof(struct StXLet));
    case XLETSTAR:     return AllocX(xtype, sizeof(struct StXLet));
    case XLETREC:      return AllocX(xtype, sizeof(struct StXLet));
    case XLETRECSTAR:  return AllocX(xtype, sizeof(struct StXLet));
    case XLAMBDA:      return AllocX(xtype, sizeof(struct StXLambda));
    case XBEGIN:       return AllocX(xtype, sizeof(struct StXBegin));
    case XIF:          return AllocX(xtype, sizeof(struct StXIf));
    case XSET:         return AllocX(xtype, sizeof(struct StXSet));
    case XCALLCC:      return AllocX(xtype, sizeof(struct StXCallCC));
    case XDEFINE:      return AllocX(xtype, sizeof(struct StXDefine));
    case XDEFINEMACRO: return AllocX(xtype, sizeof(struct StXDefineMacro));
    case XAND:         return AllocX(xtype, sizeof(struct StXList));
    case XOR:          return AllocX(xtype, sizeof(struct StXList));
    case XLIST:        return AllocX(xtype, sizeof(struct StXList));
    }
}

// parse

static StObject parse(StObject expr);

// returns a vector of (sym . expr)
static StObject parse_bindings(StObject expr)
{
    StObject h = Nil, t = Nil;

    ST_FOREACH(p, expr) {
        ST_BIND2("parse", ST_CAR(p), s, v);
        if (!ST_SYMBOLP(s))
        {
            St_Error("parse: require symbol");
        }

        ST_APPEND1(h, t, St_Cons(parse(s), parse(v)));
    }

    return St_MakeVectorFromList(h);
}

// returns (vars vals rest)
// vars :: vector of symbol
// vals :: vector of expr
static StObject find_defines(StObject expr)
{
    StObject vars = Nil, vart = Nil;
    StObject vals = Nil, valt = Nil;

    if (!ST_PAIRP(expr))
    {
        goto end;
    }

    ST_FOREACH(p, expr) {
        StObject e = ST_CAR(p);
        if (!ST_PAIRP(e))
        {
            goto end;
        }

        if (ST_CAR(e) != St_Intern("define"))
        {
            goto end;
        }

        ST_BIND2("syntax error: malformed define", ST_CDR(e), s, v);

        if (!ST_SYMBOLP(s))
        {
            St_Error("syntax error: malformed define, symbol required");
        }

        ST_APPEND1(vars, vart, parse(s));
        ST_APPEND1(vals, valt, parse(v));
    }
end:
    return ST_LIST3(St_MakeVectorFromList(vars), St_MakeVectorFromList(vals), expr);
}

static void expand_begin_sub(StObject expr, StObject *head, StObject *tail)
{
    if (!ST_PAIRP(expr))
    {
        ST_APPEND1(*head, *tail, expr);
        return;
    }

    ST_FOREACH(p, expr) {
        StObject e = ST_CAR(p);
        if (ST_PAIRP(e) && ST_CAR(p) == St_Intern("begin"))
        {
            expand_begin_sub(ST_CDR(expr), head, tail);
        }
        else
        {
            ST_APPEND1(*head, *tail, expr);
        }
    }
}

static StObject expand_begin(StObject expr)
{
    StObject h = Nil, t = Nil;
    expand_begin_sub(expr, &h, &t);
    return h;
}

// returns (vars vals exprs)
static StObject parse_body(StObject expr)
{
    StObject ds = find_defines(expand_begin(expr));
    ST_BIND3("", ds, vars, vals, rest);

    StObject h = Nil, t = Nil;

    ST_FOREACH(p, rest) {
        ST_APPEND1(h, t, parse(ST_CAR(p)));
    }

    StObject body = St_MakeVectorFromList(h);

    return ST_LIST3(vars, vals, body);
}

// retutns StExpression
static StObject parse_let(StExpressionType xtype, StObject expr)
{
    if (!ST_PAIRP(expr))
    {
        St_Error("let family: malformed let");
    }
    StObject bindings = parse_bindings(ST_CAR(expr));
    StObject body = parse_body(ST_CDR(expr));
    ST_BIND3("", body, vars, vals, exprs);

    StExpression e = St_MakeExpression(xtype);

    e->let.bindings = bindings;
    e->let.defvars = vars;
    e->let.defvals = vals;
    e->let.body = exprs;

    return ST_OBJECT(e);
}

// returns (params . dotted)
static StObject parse_params(StObject expr)
{
    StObject h = Nil, t = Nil;
    StObject dotted = False;

    if (!ST_PAIRP(expr))
    {
        St_Error("parse: parameter reuqired in lambda expression");
    }

    StObject p;
    for (p = expr; !ST_PAIRP(p); p = ST_CDR(p)) {
        if (!ST_SYMBOLP(ST_CAR(p)))
        {
            St_Error("parse: symbol required as parameter of lambda");
        }
        ST_APPEND1(h, t, parse(ST_CAR(p)));
    }

    if (ST_NULLP(p))
    {
        // do nothing
    }
    else if (ST_SYMBOLP(p))
    {
        ST_APPEND1(h, t, parse(p));
        dotted = True;
    }
    else
    {
        St_Error("parse: symbol required as parameter of lambda");
    }

    return St_Cons(h, dotted);
}

static StObject parse_lambda(StObject expr)
{
    if (!ST_PAIRP(expr))
    {
        St_Error("lambda: malformed lambda.");
    }

    StObject params = parse_params(ST_CAR(expr));
    StObject body = parse_body(ST_CDR(expr));
    ST_BIND3("", body, vars, vals, exprs);

    StExpression e = St_MakeExpression(XLAMBDA);
    e->lambda.vars = ST_CAR(params);
    e->lambda.dotted = ST_TRUTHYP(ST_CDR(params));
    e->lambda.defvars = vars;
    e->lambda.defvals = vals;
    e->lambda.body = exprs;

    return ST_OBJECT(e);
}

static StObject parse_list(StExpressionType xtype, StObject expr)
{
    StObject h = Nil, t = Nil;

    ST_FOREACH(p, expr) {
        ST_APPEND1(h, t, parse(ST_CAR(p)));
    }

    StObject exprs = St_MakeVectorFromList(h);

    StExpression e = St_MakeExpression(xtype);
    e->list.exprs = exprs;

    return ST_OBJECT(e);
}

static StObject parse(StObject expr)
{
    if (!ST_PAIRP(expr))
    {
        if (ST_SYMBOLP(expr))
        {
            StExpression e = St_MakeExpression(XSYMBOL);
            e->symbol.value = expr;
            return ST_OBJECT(e);
        }
        else
        {
            StExpression e = St_MakeExpression(XVALUE);
            e->value.value = expr;
            return ST_OBJECT(e);
        }
    }

    StObject car = ST_CAR(expr);
    StObject cdr = ST_CDR(expr);

#define CASE(sym)                               \
    if (car == St_Intern(sym))

    CASE("quote") {
        ST_BIND1("quote", cdr, expr);
        StExpression e = St_MakeExpression(XQUOTE);
        e->quote.value = expr;
        return ST_OBJECT(e);
    }
    CASE("let") {
        return parse_let(XLET, cdr);
    }
    CASE("let*") {
        return parse_let(XLETSTAR, cdr);
    }
    CASE("letrec") {
        return parse_let(XLETREC, cdr);
    }
    CASE("letrec*") {
        return parse_let(XLETRECSTAR, cdr);
    }
    CASE("lambda") {
        return parse_lambda(cdr);
    }
    CASE("bgin") {
        StObject h = Nil, t = Nil;

        ST_FOREACH(p, expand_begin(cdr)) {
            ST_APPEND1(h, t, parse(ST_CAR(p)));
        }

        StObject body = St_MakeVectorFromList(h);
        StExpression e = St_MakeExpression(XBEGIN);
        e->begin.body = body;
        return ST_OBJECT(e);
    }
    CASE("if") {
        int len = St_Length(cdr);
        StObject xpred = Nil;
        StObject xthen = Nil;
        StObject xelse = Nil;
        switch (len) {
        case 3: {
            xelse = parse(ST_CADDR(cdr));
        }
        case 2: {
            xthen = parse(ST_CADR(cdr));
            xpred = parse(ST_CAR(cdr));
            break;
        }
        default:
            St_Error("parse: if: wrong number of arguments");
        }

        StExpression e = St_MakeExpression(XIF);
        e->xif.xpred = xpred;
        e->xif.xthen = xthen;
        e->xif.xelse = xelse;
        return ST_OBJECT(e);
    }
    CASE("set!") {
        ST_BIND2("set", cdr, sym, expr);

        if (!ST_SYMBOLP(sym))
        {
            St_Error("set!: symbol required");
        }

        StExpression e = St_MakeExpression(XSET);
        e->set.symbol = parse(sym);
        e->set.value = parse(expr);
        return ST_OBJECT(e);
    }
    CASE("call/cc") {
        ST_BIND1("call/cc", cdr, lambda);

        StExpression e = St_MakeExpression(XCALLCC);
        e->callcc.lambda = parse(lambda);
        return ST_OBJECT(e);
    }
    CASE("define") {
        ST_BIND2("define", cdr, sym, expr);

        if (!ST_SYMBOLP(sym))
        {
            St_Error("define: symbol required");
        }

        StExpression e = St_MakeExpression(XDEFINE);
        e->define.symbol = parse(sym);
        e->define.value = parse(expr);
        return ST_OBJECT(e);
    }
    CASE("define-macro") {
        ST_BIND2("define-macro", cdr, sym, lambda);

        if (!ST_SYMBOLP(sym))
        {
            St_Error("define: symbol required");
        }

        StExpression e = St_MakeExpression(XDEFINE);
        e->define_macro.symbol = parse(sym);
        e->define_macro.lambda = parse(lambda);
        return ST_OBJECT(e);
    }
    CASE("and") {
        return parse_list(XAND, cdr);
    }
    CASE("or") {
        return parse_list(XOR, cdr);
    }
    else {
        return parse_list(XLIST, expr);
    }
}

StObject St_Parse(StObject module, StObject expr)
{
    StObject expanded = St_SyntaxExpand(module, expr);
    return parse(expanded);
}
