#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "lisp.h"

#define FD(o)            ST_FDPORT_FD(o)
#define SIZE(o)          ST_FDPORT(o)->size
#define P(o)             ST_FDPORT(o)->p
#define BUF(o)           ST_FDPORT(o)->buf
#define NEED_TO_CLOSE(o) ST_FDPORT(o)->need_to_close
#define EOFP(o)          ST_FDPORT(o)->eof
#define CLOSEDP(o)       ST_FDPORT(o)->closed
#define BUFSIZE          1024*10

/*

  read buffer structure:

  BUFSIZE       |<---------------------->|
  BUF(o)        |******------------------| *: read data   -: invalid data
  SIZE(o)       |<---->|     SIZE of read data
  P(o)              ^        Pointer of next unread byte. When all bytes are read, P(o) points -1.

 */

StObject St_MakeFdPort(int fd, bool need_to_close)
{
    StObject o = St_Alloc2(TFDPORT, sizeof(struct StFdPortRec));
    FD(o) = fd;
    BUF(o) = St_Malloc(BUFSIZE);
    SIZE(o) = 0;
    P(o) = -1;
    NEED_TO_CLOSE(o) = need_to_close;
    EOFP(o) = false;
    CLOSEDP(o) = false;

    return o;
}

StObject St_OpenInputPort(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        St_Error("open-input-port: open failed");
    }

    return St_MakeFdPort(fd, true);
}

StObject St_OpenOutputPort(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
    {
        St_Error("open-output-port: open failed");
    }

    return St_MakeFdPort(fd, true);
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
    StObject h = Nil, t = Nil;

    while (true) {
        StObject c = St_ReadChar(port);
        if (ST_EOFP(c) || ST_INT_VALUE(c) == '\n')
        {
            break;
        }
        ST_APPEND1(h, t, c);
    }

    int len = St_Length(h);
    char buf[len];
    int i = 0;

    ST_FOREACH(p, h) {
        buf[i++] = ST_INT_VALUE(ST_CAR(p));
    }

    return St_MakeString(len, buf);
}

bool St_CharReadyP(StObject port)
{
    return St_U8ReadyP(port); // char is byte for now
}

StObject St_ReadString(int k, StObject port)
{
    if (k <= 0)
    {
        St_Error("read-string: invalid length");
    }

    char buf[k];
    size_t len;

    for (len = 0; len < (size_t)k; len++) {
        StObject c = St_ReadChar(port);
        if (ST_EOFP(c))
        {
            break;
        }
        buf[len] = ST_INT_VALUE(c);
    }

    return St_MakeString(len, buf);
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

void St_Newline(StObject port)
{
    St_WriteU8('\n', port);
}

void St_WriteBuffer(const char *buf, size_t len, StObject port)
{
    if (ST_FALSEP(port))
    {
        port = St_CurrentOutputPort;
    }

    size_t written = 0;

    do {
        ssize_t l = write(FD(port), buf + written, len - written);
        if (l == -1)
        {
            St_Error("write error");
        }
        written += l;
    } while (written < len);
}

void St_WriteCString(const char *str, StObject port)
{
    size_t len = strlen(str);
    St_WriteBuffer(str, len, port);
}

void St_WriteU8(uint8_t byte, StObject port)
{
    if (ST_FALSEP(port))
    {
        port = St_CurrentOutputPort;
    }

    ssize_t l = write(FD(port), &byte, 1);

    if (l == -1)
    {
        St_Error("write error");
    }
}

void St_ClosePort(StObject port)
{
    if (CLOSEDP(port))
    {
        return;
    }

    close(FD(port));
    CLOSEDP(port) = true;
}

#define PORT_PROC_BODY(name, body)                          \
    do {                                                    \
        int len = St_Length(args);                          \
                                                            \
        StObject port = False;                              \
                                                            \
        switch (len) {                                      \
        case 1: {                                           \
            port = ST_CAR(args);                            \
            if (!ST_FDPORTP(port))                          \
            {                                               \
                St_Error(name "port required");             \
            }                                               \
        }                                                   \
        case 0:                                             \
            return body;                                    \
        default:                                            \
            St_Error(name ": wrong number of arguments");   \
        }                                                   \
    } while (0)

static StObject subr_read_line(StObject args)
{
    PORT_PROC_BODY("read-line", St_ReadLine(port));
}

static StObject subr_read_char(StObject args)
{
    PORT_PROC_BODY("read-char", St_ReadChar(port));
}

static StObject subr_write_u8(StObject args)
{
    int len = St_Length(args);

    StObject port = False;
    StObject byte = Unbound;

    switch (len) {
    case 2:
        port = ST_CADR(args);
        if (!ST_FDPORTP(port))
        {
            St_Error("write-u8: port required");
        }

    case 1:
        byte = ST_CAR(args);
        if (!ST_INTP(byte) || ST_INT_VALUE(byte) < 0 || 0xff < ST_INT_VALUE(byte))
        {
            St_Error("write-u8: byte required");
        }

        St_WriteU8((uint8_t)ST_INT_VALUE(byte), port);

        return Nil;

    default:
        St_Error("write-u8: wrong number of arguments");
    }
}

static StObject subr_display(StObject args)
{
    StObject obj = Unbound;

    if (ST_PAIRP(args))
    {
        obj = ST_CAR(args);
        args = ST_CDR(args);
    }

    PORT_PROC_BODY("display", (St_Display(obj, port), Nil));
}

static StObject subr_newline(StObject args)
{
    PORT_PROC_BODY("newline", (St_Newline(port), Nil));
}

static StObject subr_close_port(StObject args)
{
    ST_ARGS1("close-port", args, port);

    St_ClosePort(port);

    return Nil;
}

static StObject subr_open_input_file(StObject args)
{
    ST_ARGS1("open-input-file", args, path);

    if (!ST_STRINGP(path))
    {
        St_Error("open-input-file: string required");
    }

    return St_OpenInputPort(St_StringGetCString(path));
}

static StObject subr_open_output_file(StObject args)
{
    ST_ARGS1("open-output-file", args, path);

    if (!ST_STRINGP(path))
    {
        St_Error("open-output-file: string required");
    }

    return St_OpenOutputPort(St_StringGetCString(path));
}

StObject St_StandardInputPort  = Unbound;
StObject St_StandardOutputPort = Unbound;
StObject St_StandardErrorPort  = Unbound;

StObject St_CurrentInputPort  = Unbound;
StObject St_CurrentOutputPort = Unbound;
StObject St_CurrentErrorPort  = Unbound;

void St_InitPort(void)
{
    St_CurrentInputPort  = St_StandardInputPort  = St_MakeFdPort(0, false);
    St_CurrentOutputPort = St_StandardOutputPort = St_MakeFdPort(1, false);
    St_CurrentErrorPort  = St_StandardErrorPort  = St_MakeFdPort(2, false);

    StObject m = GlobalModule;

    St_AddSubr(m, "read-line", subr_read_line);
    St_AddSubr(m, "read-char", subr_read_char);
    St_AddSubr(m, "write-u8", subr_write_u8);
    St_AddSubr(m, "display", subr_display);
    St_AddSubr(m, "newline", subr_newline);
    St_AddSubr(m, "close-port", subr_close_port);
    St_AddSubr(m, "open-input-file", subr_open_input_file);
    St_AddSubr(m, "open-output-file", subr_open_output_file);
}
