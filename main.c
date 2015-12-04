#include "lisp.h"

int main(int argc, char** argv)
{
    Object *expr = St_Read(stdin);
    St_Print(expr);

    return 0;
}
