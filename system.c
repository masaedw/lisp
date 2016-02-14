#include <unistd.h>

#include "lisp.h"

StObject St_SysPipe()
{
    int fds[2];

    if (pipe(fds) != 0)
    {
        St_Error("pipe error");
    }

    return St_Cons(St_MakeFdPort(fds[0], true), St_MakeFdPort(fds[1], true));
}

int St_SysFork()
{
    int pid = fork();
    if (pid == -1)
    {
        St_Error("fork error");
    }

    return pid;
}
