#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lisp.h"
#include "subr.h"

StObject St_CurrentExecScriptName = Unbound; // filename or "-" (stdin)
static int Argc;
static char **Argv;
static StObject ArgvObj = Unbound;

StObject St_CommandLine(void)
{
    if (ST_UNBOUNDP(ArgvObj))
    {
        StObject h = Nil, t = Nil;
        for (int i = 0; i < Argc; i++) {
            ST_APPEND1(h, t, St_MakeStringFromCString(Argv[i]));
        }
        ArgvObj = h;

        if (!St_StringEqualP(St_CurrentExecScriptName, St_MakeStringFromCString("-")))
        {
            ST_FOREACH(p, ArgvObj) {
                if (St_StringEqualP(ST_CAR(p), St_CurrentExecScriptName))
                {
                    ArgvObj = p;
                    break;
                }
            }
        }
    }

    return ArgvObj;
}

StObject St_GetEnvironment(StObject name)
{
    char *value = getenv(St_StringGetCString(name));

    if (value == NULL)
    {
        return False;
    }

    return St_MakeStringFromCString(value);
}

extern char **environ;

StObject St_GetEnvironments(void)
{
    StObject h = Nil, t = Nil;

    for (char **e = environ; *e != NULL; e++) {
        char *p = strchr(*e, '=');
        ST_APPEND1(h, t, St_Cons(St_MakeString(p - *e, *e), St_MakeStringFromCString(p + 1)));
    }

    return h;
}

StObject St_SysPipe(void)
{
    int fds[2];

    if (pipe(fds) != 0)
    {
        St_Error("pipe error");
    }

    return St_Cons(St_MakeFdPort(fds[0], true), St_MakeFdPort(fds[1], true));
}

int St_SysFork(void)
{
    int pid = fork();
    if (pid == -1)
    {
        St_Error("fork error");
    }

    return pid;
}

void St_SysPause(void)
{
    pause();
}

void St_SysExit(int s)
{
    exit(s);
}

void St_SysKill(int pid, int signal)
{
    if (kill(pid, signal) == -1)
    {
        St_Error("kill error. pid: %d, signal: %d", pid, signal);
    }
}

void St_SysWaitPid(int pid)
{
    int stat_loc = 0;
    if (waitpid(pid, &stat_loc, 0) == -1)
    {
        St_Error("waitpid erorr. pid: %d", pid);
    }
}

static StObject subr_command_line(StCallInfo *cinfo)
{
    ST_ARGS0("command-line", cinfo);

    return St_CommandLine();
}

static StObject subr_get_environment_variable(StCallInfo *cinfo)
{
    ST_ARGS1("get-environment-variable", cinfo, name);

    if (!ST_STRINGP(name))
    {
        St_Error("get-environment-variable: string required");
    }

    return St_GetEnvironment(name);
}

static StObject subr_get_environment_variables(StCallInfo *cinfo)
{
    ST_ARGS0("get-environment-variables", cinfo);

    return St_GetEnvironments();
}

static StObject subr_sys_pipe(StCallInfo *cinfo)
{
    ST_ARGS0("sys-pipe", cinfo);

    return St_SysPipe();
}

static StObject subr_sys_fork(StCallInfo *cinfo)
{
    ST_ARGS0("sys-fork", cinfo);

    return St_Integer(St_SysFork());
}

static StObject subr_sys_pause(StCallInfo *cinfo)
{
    ST_ARGS0("sys-pause", cinfo);

    St_SysPause();

    return Nil;
}

static StObject subr_sys_exit(StCallInfo *cinfo)
{
    ST_ARGS1("sys-exit", cinfo, s);

    if (!ST_INTP(s))
    {
        St_Error("sys-exit: integer required");
    }

    St_SysExit(ST_INT_VALUE(s));

    return Nil;
}

static StObject subr_sys_kill(StCallInfo *cinfo)
{
    ST_ARGS2("sys-kill", cinfo, pid, signal);

    if (!ST_INTP(pid) || !ST_INTP(signal))
    {
        St_Error("sys-kill: integer requried");
    }

    St_SysKill(ST_INT_VALUE(pid), ST_INT_VALUE(signal));

    return Nil;
}

static StObject subr_sys_waitpid(StCallInfo *cinfo)
{
    ST_ARGS1("sys-waitpid", cinfo, pid);

    if (!ST_INTP(pid))
    {
        St_Error("sys-waitpid: integer required");
    }

    St_SysWaitPid(ST_INT_VALUE(pid));

    return Nil;
}

void St_InitSystem(int argc, char** argv)
{
    Argc = argc;
    Argv = argv;

    StObject m = GlobalModule;

    St_AddSubr(m, "command-line", subr_command_line);
    St_AddSubr(m, "get-environment-variable", subr_get_environment_variable);
    St_AddSubr(m, "get-environment-variables", subr_get_environment_variables);
    St_AddSubr(m, "sys-pipe", subr_sys_pipe);
    St_AddSubr(m, "sys-fork", subr_sys_fork);
    St_AddSubr(m, "sys-pause", subr_sys_pause);
    St_AddSubr(m, "sys-exit", subr_sys_exit);
    St_AddSubr(m, "sys-kill", subr_sys_kill);
    St_AddSubr(m, "sys-waitpid", subr_sys_waitpid);

#define DEFINE_SIGNAL(s) St_ModulePush(m, St_Intern(#s), St_Integer(s))

    DEFINE_SIGNAL(SIGHUP);
    DEFINE_SIGNAL(SIGINT);
    DEFINE_SIGNAL(SIGQUIT);
    DEFINE_SIGNAL(SIGILL);
    DEFINE_SIGNAL(SIGTRAP);
    DEFINE_SIGNAL(SIGABRT);
//    DEFINE_SIGNAL(SIGEMT);
    DEFINE_SIGNAL(SIGFPE);
    DEFINE_SIGNAL(SIGKILL);
    DEFINE_SIGNAL(SIGBUS);
    DEFINE_SIGNAL(SIGSEGV);
    DEFINE_SIGNAL(SIGSYS);
    DEFINE_SIGNAL(SIGPIPE);
    DEFINE_SIGNAL(SIGALRM);
    DEFINE_SIGNAL(SIGTERM);
    DEFINE_SIGNAL(SIGURG);
    DEFINE_SIGNAL(SIGSTOP);
    DEFINE_SIGNAL(SIGTSTP);
    DEFINE_SIGNAL(SIGCONT);
    DEFINE_SIGNAL(SIGCHLD);
    DEFINE_SIGNAL(SIGTTIN);
    DEFINE_SIGNAL(SIGTTOU);
    DEFINE_SIGNAL(SIGIO);
    DEFINE_SIGNAL(SIGXCPU);
    DEFINE_SIGNAL(SIGXFSZ);
    DEFINE_SIGNAL(SIGVTALRM);
    DEFINE_SIGNAL(SIGPROF);
    DEFINE_SIGNAL(SIGWINCH);
//    DEFINE_SIGNAL(SIGINFO);
    DEFINE_SIGNAL(SIGUSR1);
    DEFINE_SIGNAL(SIGUSR2);

#undef DEFINE_SIGNAL
}
