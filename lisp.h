#ifndef LISP_H
#define LISP_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <gc.h>

// type tag
enum {
    TCELL = 1,
    TSYMBOL,
    TCHAR,
    TSTRING,
    TVECTOR,
    TBYTEVECTOR,
    TSYNTAX,
    TSUBR,
    TLAMBDA,
    TMACRO,
    TFDPORT,
};

struct StObjectHeader;
typedef struct StObjectHeader *StObject;
#define ST_OBJECT(x) ((StObject)(x))

#define ST_OBJECT_HEADER int type

struct StObjectHeader
{
    ST_OBJECT_HEADER;
};

struct StCellRec
{
    ST_OBJECT_HEADER;
    StObject car;
    StObject cdr;
};
typedef struct StCellRec *StCell;
#define ST_CELL(x) ((StCell)(x))

struct StSymbolRec
{
    ST_OBJECT_HEADER;
    char value[];
};
typedef struct StSymbolRec *StSymbol;
#define ST_SYMBOL(x) ((StSymbol)(x))
#define ST_SYMBOL_VALUE(x) (ST_SYMBOL(x)->value)

struct StStringRec
{
    ST_OBJECT_HEADER;
    size_t len;
    char value[];
};
typedef struct StStringRec *StString;
#define ST_STRING(x) ((StString)(x))
#define ST_STRING_LENGTH(x) (ST_STRING(x)->len)
#define ST_STRING_VALUE(x) (ST_STRING(x)->value)

struct StCharacterRec
{
    ST_OBJECT_HEADER;
    uint32_t value;
};

struct StVectorRec
{
    ST_OBJECT_HEADER;
    size_t len;
    StObject data[];
};
typedef struct StVectorRec *StVector;
#define ST_VECTOR(x) ((StVector)(x))
#define ST_VECTOR_LENGTH(x) (ST_VECTOR(x)->len)
#define ST_VECTOR_DATA(x) (ST_VECTOR(x)->data)

struct StBytevectorRec
{
    ST_OBJECT_HEADER;
    size_t len;
    uint8_t data[];
};
typedef struct StBytevectorRec *StBytevector;
#define ST_BYTEVECTOR(x) ((StBytevector)(x))
#define ST_BYTEVECTOR_LENGTH(x) (ST_BYTEVECTOR(x)->len)
#define ST_BYTEVECTOR_DATA(x) (ST_BYTEVECTOR(x)->data)

typedef StObject (*StSyntaxFunction)(StObject form);
struct StSyntaxRec
{
    ST_OBJECT_HEADER;
    StSyntaxFunction body;
    const char *name;
};
typedef struct StSyntaxRec *StSyntax;
#define ST_SYNTAX(x) ((StSyntax)(x))
#define ST_SYNTAX_BODY(x) (ST_SYNTAX(x)->body)
#define ST_SYNTAX_NAME(x) (ST_SYNTAX(x)->name)

typedef StObject (*StSubrFunction)(StObject args);
struct StSubrRec
{
    ST_OBJECT_HEADER;
    StSubrFunction body;
    const char *name;
};
typedef struct StSubrRec *StSubr;
#define ST_SUBR(x) ((StSubr)(x))
#define ST_SUBR_BODY(x) (ST_SUBR(x)->body)
#define ST_SUBR_NAME(x) (ST_SUBR(x)->name)

struct StLambdaRec
{
    ST_OBJECT_HEADER;
    StObject body;
    StObject free;
    int arity;
};
typedef struct StLambdaRec *StLambda;
#define ST_LAMBDA(x) ((StLambda)(x))
#define ST_LAMBDA_BODY(x) (ST_LAMBDA(x)->body)
#define ST_LAMBDA_FREE(x) (ST_LAMBDA(x)->free)
#define ST_LAMBDA_ARITY(x) (ST_LAMBDA(x)->arity)

struct StMacroRec
{
    ST_OBJECT_HEADER;
    StObject proc;
    StObject symbol;
};
typedef struct StMacroRec *StMacro;
#define ST_MACRO(x) ((StMacro)(x))
#define ST_MACRO_PROC(x) (ST_MACRO(x)->proc)
#define ST_MACRO_SYMBOL(x) (ST_MACRO(x)->symbol)

struct StFdPortRec
{
    ST_OBJECT_HEADER;
    int fd;
    ssize_t size;
    int p;
    uint8_t *buf;
    bool need_to_close;
    bool eof;
    bool closed;
};
typedef struct StFdPortRec *StFdPort;
#define ST_FDPORT(x) ((StFdPort)(x))
#define ST_FDPORT_FD(x) (ST_FDPORT(x)->fd)

// Tagged pointer structure
//
// name       lower bits  value
// ---------  ----------  -----
// type tag   _____11
// object     _____00
// integer    _____01
// singleton  __00010     Nil
// singleton  __00110     True
// singleton  __01010     False
// singleton  __01110     Unbound
// singleton  __10010     Eof

#define ST_TAG_BITS     2
#define ST_TAG_MASK     0b11
#define ST_OBJECT_TAG   0b00
#define ST_INT_TAG      0b01
#define ST_POINTER_TAG(x) ((intptr_t)(x) & ST_TAG_MASK)

#define Nil     ST_OBJECT(0b00010)
#define True    ST_OBJECT(0b00110)
#define False   ST_OBJECT(0b01010)
#define Unbound ST_OBJECT(0b01110)
#define Eof     ST_OBJECT(0b10010)

void St_Error(const char *fmt, ...) __attribute__((noreturn));

#define St_Malloc GC_MALLOC

void *St_Alloc2(int type, size_t size);

#define ST_OBJECTP(obj)     (ST_POINTER_TAG(obj) == ST_OBJECT_TAG)
#define ST_TAGP(obj, tag)   (ST_OBJECTP(obj) && (obj)->type == (tag))
#define ST_NULLP(obj)       ((obj) == Nil)
#define ST_TRUEP(obj)       ((obj) == True)
#define ST_FALSEP(obj)      ((obj) == False)
#define ST_UNBOUNDP(obj)    ((obj) == Unbound)
#define ST_EOFP(obj)        ((obj) == Eof)
#define ST_PAIRP(obj)       ST_TAGP(obj, TCELL)
#define ST_INTP(obj)        (ST_POINTER_TAG(obj) == ST_INT_TAG)
#define ST_SYMBOLP(obj)     ST_TAGP(obj, TSYMBOL)
#define ST_VECTORP(obj)     ST_TAGP(obj, TVECTOR)
#define ST_BYTEVECTORP(obj) ST_TAGP(obj, TBYTEVECTOR)
#define ST_CHARP(obj)       ST_TAGP(obj, TCHAR)
#define ST_STRINGP(obj)     ST_TAGP(obj, TSTRING)
#define ST_SYNTAXP(obj)     ST_TAGP(obj, TSYNTAX)
#define ST_SUBRP(obj)       ST_TAGP(obj, TSUBR)
#define ST_LAMBDAP(obj)     ST_TAGP(obj, TLAMBDA)
#define ST_MACROP(obj)      ST_TAGP(obj, TMACRO)
#define ST_PROCEDUREP(obj)  (ST_SUBRP(obj) || ST_LAMBDAP(obj))
#define ST_FDPORTP(obj)     ST_TAGP(obj, TFDPORT)

// List and Pair

StObject St_Cons(StObject car, StObject cdr);
StObject St_Acons(StObject key, StObject val, StObject cdr);
StObject St_Reverse(StObject list);
int St_Length(StObject list);
bool St_ListP(StObject maybe_list);
bool St_DottedListP(StObject maybe_list);
StObject St_Assq(StObject obj, StObject alist);
StObject St_Assv(StObject obj, StObject alist);

#define ST_LIST1(a0)                 St_Cons((a0), Nil)
#define ST_LIST2(a0, a1)             St_Cons((a0), ST_LIST1((a1)))
#define ST_LIST3(a0, a1, a2)         St_Cons((a0), ST_LIST2((a1), (a2)))
#define ST_LIST4(a0, a1, a2, a3)     St_Cons((a0), ST_LIST3((a1), (a2), (a3)))
#define ST_LIST5(a0, a1, a2, a3, a4) St_Cons((a0), ST_LIST4((a1), (a2), (a3), (a4)))

#define ST_CAR(pair) (ST_CELL(pair)->car)
#define ST_CDR(pair) (ST_CELL(pair)->cdr)
#define ST_CDDR(list) ST_CDR(ST_CDR(list))
#define ST_CDAR(list) ST_CDR(ST_CAR(list))
#define ST_CADR(list) ST_CAR(ST_CDR(list))
#define ST_CAAR(list) ST_CAR(ST_CAR(list))
#define ST_CADDR(list) ST_CAR(ST_CDR(ST_CDR(list)))
#define ST_CADDDR(list) ST_CAR(ST_CDR(ST_CDR(ST_CDR(list))))

#define ST_CAR_SET(pair, value) (ST_CAR(pair) = (value))
#define ST_CDR_SET(pair, value) (ST_CDR(pair) = (value))

#define ST_APPEND1(head, tail, value)                   \
    do {                                                \
        if (ST_NULLP(head))                             \
        {                                               \
            (head) = (tail) = St_Cons((value), Nil);    \
        }                                               \
        else                                            \
        {                                               \
            ST_CDR_SET(tail, St_Cons((value), Nil));    \
            tail = ST_CDR(tail);                        \
        }                                               \
    } while (0)

#define ST_FOREACH(p, list) for (StObject p = (list); !ST_NULLP(p); p = ST_CDR(p))

// List as Set

bool St_SetMemberP(StObject obj, StObject s);
StObject St_SetCons(StObject obj, StObject s);
StObject St_SetUnion(StObject s1, StObject s2);
StObject St_SetAppend(StObject s1, StObject s2);
StObject St_SetMinus(StObject s1, StObject s2);
StObject St_SetIntersect(StObject s1, StObject s2);

// Symbol

StObject St_Intern(const char *symbol_string);

// Primitive utilities

#define ST_CHECK_ARG_LEN(name, args, len)                   \
    do {                                                    \
        int _len = St_Length(args);                         \
                                                            \
        if (_len != (len))                                  \
        {                                                   \
            St_Error(name ": wrong number of arguments");   \
        }                                                   \
    } while (0)

#define ST_ARGS0(name, args)                    \
    ST_CHECK_ARG_LEN(name, args, 0)


#define ST_ARGS1(name, args, a1)                \
    ST_CHECK_ARG_LEN(name, args, 1);            \
    StObject (a1) = ST_CAR(args)

#define ST_ARGS2(name, args, a1, a2)            \
    ST_CHECK_ARG_LEN(name, args, 2);            \
    StObject (a1) = ST_CAR(args);               \
    StObject (a2) = ST_CADR(args)

#define ST_ARGS3(name, args, a1, a2, a3)        \
    ST_CHECK_ARG_LEN(name, args, 3);            \
    StObject (a1) = ST_CAR(args);               \
    StObject (a2) = ST_CADR(args);              \
    StObject (a3) = ST_CADDR(args)

#define ST_ARGS4(name, args, a1, a2, a3, a4)    \
    ST_CHECK_ARG_LEN(name, args, 4);            \
    StObject (a1) = ST_CAR(args);               \
    StObject (a2) = ST_CADR(args);              \
    StObject (a3) = ST_CADDR(args);             \
    StObject (a4) = ST_CADDDR(args)

// Coercers

#define ST_BOOLEAN(b) ((b) ? True : False)
// TODO overflow check
#define St_Integer(value) ST_OBJECT(((intptr_t)(value) << ST_TAG_BITS) | ST_INT_TAG)
#define ST_INT_VALUE(x) (((intptr_t)(x) >> ST_TAG_BITS))

// String

StObject St_MakeEmptyString(int len);
StObject St_MakeString(int len, char* buf);
size_t St_StringLength(StObject s);
bool St_StringEqualP(StObject s1, StObject s2);
StObject St_StringAppend(StObject list);
char *St_StringGetCString(StObject string);

// Vector

StObject St_MakeVector(int size);
StObject St_VectorRef(StObject vector, int idx);
void St_CopyVector(StObject dst, StObject src, int size);
void St_VectorSet(StObject vector, int idx, StObject obj);
size_t St_VectorLength(StObject vector);

// Bytevector

StObject St_MakeBytevector(int size, int byte);
StObject St_MakeBytevectorFromList(StObject bytes);
size_t St_BytevectorLength(StObject bytevector);
uint8_t St_BytevectorU8Ref(StObject bytevector, int k);
void St_BytevectorU8Set(StObject bytevector, int k, uint8_t byte);
StObject St_MakeBytevectorFrom(StObject bytevector, int start, int end);
void St_BytevectorCopy(StObject to, int at, StObject from, int start, int end);
StObject St_BytevectorAppend(StObject vectors);
bool St_BytevectorEqualP(StObject b1, StObject b2);

// Dynamic Vector (complex type)

StObject St_MakeDVector(int size, int capa);
StObject St_DVectorRef(StObject vector, int idx);
void St_DVectorSet(StObject vector, int idx, StObject obj);
int St_DVectorLength(StObject vector);
StObject St_DVectorData(StObject vector);
int St_DVectorCapacity(StObject vector);
int St_DVectorPush(StObject vector, StObject obj);

// Module

extern StObject GlobalModule;
StObject St_MakeModule(StObject alist);
StObject St_ModuleFind(StObject module, StObject sym);
int St_ModuleFindOrInitialize(StObject module, StObject sym, StObject init);
void St_ModulePush(StObject module, StObject sym, StObject value);
void St_ModuleSet(StObject module, int idx, StObject val);
StObject St_ModuleRef(StObject module, int idx);
StObject St_ModuleSymbols(StObject module);
void St_InitModule();

// Basic functions

bool St_EqvP(StObject lhs, StObject rhs);
bool St_EqualP(StObject lhs, StObject rhs);
StObject St_Gensym();
StObject St_Apply(StObject proc, StObject args);
void St_Load(const char *filename);

// Port

StObject St_MakeFdPort(int fd, bool need_to_close);
//StObject St_Read(StObject port);
StObject St_ReadChar(StObject port);
StObject St_PeekChar(StObject port);
StObject St_ReadLine(StObject port);
bool St_CharReadyP(StObject port);
StObject St_ReadString(int k, StObject port);
StObject St_ReadU8(StObject port);
StObject St_PeekU8(StObject port);
bool St_U8ReadyP(StObject port);
void St_Newline(StObject port);
void St_WriteBuffer(const char *buf, size_t len, StObject port);
void St_WriteCString(const char *str, StObject port);
void St_WriteU8(uint8_t byte, StObject port);
void St_ClosePort(StObject port);
//void St_CloseReadPort(StObject port);
//void St_CloseWritePort(StObject port);

//StObject St_ReadBytevector(int k, StObject port);
//int St_ReadBytevectorX(StObject bytevector, StObject port, int start, int end);
extern StObject St_StandardInputPort;
extern StObject St_StandardOutputPort;
extern StObject St_StandardErrorPort;
extern StObject St_CurrentInputPort;
extern StObject St_CurrentOutputPort;
extern StObject St_CurrentErrorPort;
void St_InitPort();

// System

// returns pair of ports: (in . out)
StObject St_SysPipe();
int St_SysFork();
void St_SysPause();
void St_SysExit(int status) __attribute__((noreturn));
void St_SysKill(int pid, int signal);
void St_SysWaitPid(int pid);
void St_InitSystem();

// Environment

void St_AddSyntax(StObject module, const char *key, StSyntaxFunction syntax);
void St_AddSubr(StObject module, const char *key, StSubrFunction subr);

void St_InitPrimitives();
void St_InitSyntax();
void St_InitVm();

// Parser


StObject St_Read(StObject port);

// Printer

void St_Display(StObject obj, StObject port);
void St_Print(StObject obj, StObject port);

// Evaluator

StObject St_Eval_VM(StObject module, StObject obj);
StObject St__Eval_INSN(StObject module, StObject insn);

// Compiler

StObject St_MacroExpand(StObject module, StObject expr);
StObject St_Compile(StObject expr, StObject module, StObject next);

#endif
