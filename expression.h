#pragma once

#include "lisp.h"

typedef enum {
    XVALUE = 1,
    XSYMBOL,
    XQUOTE,
    XLET,
    XLETSTAR,
    XLETREC,
    XLETRECSTAR,
    XLAMBDA,
    XBEGIN,
    XIF,
    XSET,
    XCALLCC,
    XDEFINE,
    XDEFINEMACRO,
    XAND,
    XOR,
    XLIST,
} StExpressionType;

extern StExternalTypeInfo StExpressionTypeInfo;

struct StExpressionRec
{
    ST_EXTERNAL_OBJECT_HEADER;
    StExpressionType xtype;

    union
    {
        struct StXValue
        {
            StObject value;
        } value;

        struct StXSymbol
        {
            StObject symbol;
        } symbol;

        struct StXQuote
        {
            StObject expr;
        } quote;

        struct StXLet
        {
            StObject bindings; // vector of (symbol . expr)
            StObject defvars; // internal define symbols. vector of symbol
            StObject defvals; // internal define values. vector of expr
            StObject body; // vector of expr except begin expr
        } let, letstar, letrec, letrecstar;

        struct StXLambda
        {
            StObject vars; // vector of symbol
            bool dotted; // if true, vars is (lambda x ...) or (lambda (x y . z) ...)
            StObject defvars; // internal define symbols. vector of symbol
            StObject defvals; // internal define values. vector of expr
            StObject body; // vector of expr except begin expr
        } lambda;

        struct StXBegin
        {
            StObject body; // vector of expr
        } begin;

        struct StXIf
        {
            StObject xpred; // expr

            StObject xthen; // expr
            StObject xelse; // expr or Nil
        } xif;

        struct StXSet
        {
            StObject symbol;
            StObject value; // expr
        } set;

        struct StXCallCC
        {
            StObject lambda; // lambda expr
        } callcc;

        struct StXDefine
        {
            StObject symbol;
            StObject value; // expr
        } define;

        struct StXDefineMacro
        {
            StObject symbol;
            StObject lambda;
        } define_macro;

        struct StXList
        {
            StObject exprs; // vector of expr
        } and, or, list;
    };
};
typedef struct StExpressionRec *StExpression;
#define ST_EXPRESSION(obj) ((StExpression)(obj))

StExpression St_MakeExpression(StExpressionType type);
