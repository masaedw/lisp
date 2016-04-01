#include "expression.h"

static void print(StObject obj, StObject port);
static bool equalp(StObject lhs, StObject rhs);

StExternalTypeInfo StExpressionTypeInfo = (StExternalTypeInfo) { "<expression>", print, equalp };

static void print(StObject obj, StObject port)
{
    (void)obj;
    (void)port;
    // TODO
}

static bool equalp(StObject lhs, StObject rhs)
{
    (void)lhs;
    (void)rhs;
    // TODO
    return false;
}
