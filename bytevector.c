#include <string.h>

#include "lisp.h"

#define LENGTH(x) (x)->bytevector.size
#define DATA(x) (x)->bytevector.data

StObject St_MakeBytevector(int size, int byte)
{
    StObject o = St_Alloc(TBYTEVECTOR, sizeof(struct StBytevector) + size - 1);
    LENGTH(o) = size;
    if (byte >= 0)
    {
        memset(DATA(o), byte, size);
    }
    return o;
}

StObject St_MakeBytevectorFromList(StObject bytes)
{
    int len = St_Length(bytes);
    StObject o = St_MakeBytevector(len, -1);
    int i = 0;
    ST_FOREACH(p, bytes) {
        St_BytevectorU8Set(o, i++, ST_INT_VALUE(ST_CAR(p)));
    }
    return o;
}

int St_BytevectorLength(StObject b)
{
    return LENGTH(b);
}

#define VALIDATE_INDEX(n, b, k)                                         \
    do {                                                                \
        if ((k) < 0 || LENGTH(b) <= (k))                                \
        {                                                               \
            St_Error("%s: index out of range: %d against %d", (n), (k), LENGTH(b)); \
        }                                                               \
    } while (0)

#define VALIDATE_SIZE(n, b, k)                                          \
    do {                                                                \
        if ((k) < 0 || LENGTH(b) < (k))                                 \
        {                                                               \
            St_Error("%s: index out of range: %d against %d", (n), (k), LENGTH(b)); \
        }                                                               \
    } while (0)


uint8_t St_BytevectorU8Ref(StObject b, int k)
{
    VALIDATE_INDEX("bytevector-u8-ref", b, k);

    return DATA(b)[k];
}

void St_BytevectorU8Set(StObject b, int k, uint8_t byte)
{
    VALIDATE_INDEX("bytevector-u8-set!", b, k);

    DATA(b)[k] = byte;
}

StObject St_MakeBytevectorFrom(StObject b, int start, int end)
{
    if (start < 0)
    {
        start = 0;
    }
    VALIDATE_INDEX("bytevector-copy", b, start);

    if (end < 0)
    {
        end = LENGTH(b);
    }
    VALIDATE_SIZE("bytevector-copy", b, end);

    int newlen = end - start;

    if (newlen < 0)
    {
        St_Error("bytevector-copy: end %d is less than start %d", end, start);
    }

    StObject o = St_MakeBytevector(newlen, -1);
    memcpy(DATA(o), DATA(b) + start, newlen);
    return o;
}

void St_BytevectorCopy(StObject to, int at, StObject from, int start, int end)
{
    VALIDATE_INDEX("bytevector-copy!", to, at);

    if (start < 0)
    {
        start = 0;
    }
    VALIDATE_INDEX("bytevector-copy!", from, start);

    if (end < 0)
    {
        end = LENGTH(from);
    }
    VALIDATE_SIZE("bytevector-copy!", from, end);

    int copylen = end - start;

    if (copylen < 0)
    {
        St_Error("bytevector-copy!: end %d is less than start %d", end, start);
    }

    int capa = LENGTH(to) - at;

    if (capa < copylen)
    {
        St_Error("bytevector-copy!: length to copy %d is more than capacity %d", copylen, capa);
    }

    memmove(DATA(to) + at, DATA(from) + start, copylen);
}

StObject St_BytevectorAppend(StObject vectors)
{
    int len = 0;
    ST_FOREACH(p, vectors) {
        len += LENGTH(ST_CAR(p));
    }
    StObject o = St_MakeBytevector(len, -1);

    int d = 0;
    ST_FOREACH(p, vectors) {
        memcpy(DATA(o) + d, DATA(ST_CAR(p)), LENGTH(ST_CAR(p)));
        d += LENGTH(ST_CAR(p));
    }
    return o;
}
