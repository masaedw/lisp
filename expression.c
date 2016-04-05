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
        St_Display(xobj->symbol.symbol, port);
        break;

    case XQUOTE:
        St_WriteCString("(quote ", port);
        St_Display(xobj->quote.expr, port);
        St_WriteCString(")", port);
        break;

    case XLET:
        display_let("let", &xobj->let, port);
        break;

    case XLETSTAR:
        display_let("let*", &xobj->letstar, port);
        break;

    case XLETREC:
        display_let("letrec", &xobj->letrec, port);
        break;

    case XLETRECSTAR:
        display_let("letrec*", &xobj->letrecstar, port);
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
        display_body(xobj->and.exprs, port);
        St_WriteCString(")", port);
        break;

    case XOR:
        St_WriteCString("(or ", port);
        display_body(xobj->or.exprs, port);
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
    case XSYMBOL:      return AllocX(xtype, sizeof(struct StXSymbol));
    case XQUOTE:       return AllocX(xtype, sizeof(struct StXQuote));
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

typedef struct
{
    StObject obj;
    StObject rest;
} StXCont;

// parse

// returns obj as vector of (sym . expr)
static StXCont parse_bindings(StObject expr)
{
    // TODO: Implement
    (void)expr;
    return (StXCont) { };
}

static StXCont parse_body(StObject expr)
{
    // TODO: Implement
    (void)expr;
    return (StXCont) { };
}

static StObject parse(StObject expr)
{
    if (!ST_PAIRP(expr))
    {
        if (ST_SYMBOLP(expr))
        {
            StExpression e = St_MakeExpression(XSYMBOL);
            e->symbol.symbol = expr;
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
        e->quote.expr = expr;
        return ST_OBJECT(e);
    }

    CASE("let") {
        StXCont bindings = parse_bindings(cdr);
        StXCont body = parse_body(bindings.rest);
    }
    /*
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
    */
    return Nil;
}

StObject St_Parse(StObject module, StObject expr)
{
    StObject expanded = St_SyntaxExpand(module, expr);
}
