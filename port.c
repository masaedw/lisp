#include <sys/select.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "lisp.h"

#define FD(o)            (o)->fd_port.fd
#define SIZE(o)          (o)->fd_port.size
#define P(o)             (o)->fd_port.p
#define BUF(o)           (o)->fd_port.buf
#define NEED_TO_CLOSE(o) (o)->fd_port.need_to_close
#define EOFP(o)          (o)->fd_port.eof
#define BUFSIZE          1024*10

StObject St_MakeFdPort(int fd, bool need_to_close)
{
    StObject o = St_Alloc(TFDPORT, sizeof(struct StFdPort));
    FD(o) = fd;
    BUF(o) = St_Malloc(BUFSIZE);
    SIZE(o) = 0;
    P(o) = -1;
    NEED_TO_CLOSE(o) = need_to_close;
    EOFP(o) = false;

    return o;
}

#define IS_EMPTY_BUF(o) (P(o) == -1)
#define BUF_REF(port) St_Integer(BUF(port)[P(port)])

static bool FillBuffer(StObject port)
{
    ssize_t size = read(FD(port), BUF(port), BUFSIZE);
    if (size == -1)
    {
        St_Error("read error");
    }

    SIZE(port) = size;
    P(port) = 0;

    if (size == 0)
    {
        EOFP(port) = true;
        return false;
    }

    return true;
}

static StObject GetByteFromBuffer(StObject port)
{
    StObject c = BUF_REF(port);
    P(port)++;
    if (SIZE(port) == P(port))
    {
        P(port) = -1;
    }
    return c;
}

StObject St_ReadChar(StObject port)
{
    return St_ReadU8(port); // char is byte for now
}

StObject St_PeekChar(StObject port)
{
    return St_PeekU8(port); // char is byte for now
}

StObject St_ReadLine(StObject port)
{
    return Nil; // TODO
}

bool St_CharReadyP(StObject port)
{
    return St_U8ReadyP(port); // char is byte for now
}

StObject St_ReadString(int k, StObject port)
{
    return Nil; // TODO
}

StObject St_ReadU8(StObject port)
{
    if (EOFP(port))
    {
        return Eof;
    }

    if (!IS_EMPTY_BUF(port) || FillBuffer(port))
    {
        return GetByteFromBuffer(port);
    }
    else
    {
        return Eof;
    }
}

StObject St_PeekU8(StObject port)
{
    if (EOFP(port))
    {
        return Eof;
    }

    if (!IS_EMPTY_BUF(port) || FillBuffer(port))
    {
        return BUF_REF(port);
    }
    else
    {
        return Eof;
    }
}

bool St_U8ReadyP(StObject port)
{
    if (EOFP(port) || !IS_EMPTY_BUF(port))
    {
        return true;
    }

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(FD(port), &fs);
    struct timeval tval = { 0, 0 };

    int r = select(FD(port) + 1, &fs, NULL, NULL, &tval);

    if (r == -1)
    {
        St_Error("select error");
    }

    return r == 1;
}
