#include <unistd.h>

#include "lisp.h"

StObject St_SysPipe()
{
    int fds[2];

    if (pipe(fds) != 0)
    {
        St_Error("pipe error");
    }

    StObject o = St_MakeVector(2);
    St_VectorSet(o, 0, St_Integer(fds[0]));
    St_VectorSet(o, 1, St_Integer(fds[1]));
    return o;
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
