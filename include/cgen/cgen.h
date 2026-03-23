#ifndef CGEN_H
#define CGEN_H

#include <core/cutils.h>
#include <ds/ds.h>
#include <strbase/strbase.h>

typedef enum TokenType {
    TOKEN_ID = 128,
    TOKEN_STRUCT,
    TOKEN_TYPEDEF,
} TokenType;

typedef struct Token {
    TokenType t;
    u32 id; // side array info
    u32 line;
} Token;

DefDynArrayDecl(TokenBuffer, Token);

TokenBuffer Tokenize(SString data, StrBase *b, Allocator a);
SString TokenName(Token t, StrBase *b, ArenaAllocator *persist);

typedef enum TypeFlags : u16 {
    //base types
    TYPE_BASE = 1 << 0,

    //derived types
    TYPE_CONST = 1 << 1,
    TYPE_POINTER = 1 << 2,
    TYPE_ALIAS = 1 << 3,
    TYPE_STRUCT = 1 << 4,
    TYPE_UNION = 1 << 5,
    TYPE_ENUM = 1 << 6,
    TYPE_FUNC = 1 << 7,
} TypeFlags;

typedef struct Type {
    TypeFlags flags;
    u32 inner; // limited to 26 bits
    StrID name;

    //members if we have any
    struct { 
        u32 first; //32 bits
        u32 num;
    };
} Type;

typedef struct Member {
    u32 type;
    StrID name;
} Member;

//(inner << 4) | MSB16(flags)
//26 + 4 bits
typedef u32 TypeID;

/*
    INFO(ELI): 
    Derived Types do not store a name since the name
    is implicit. They are deduplicated via the Outer Map.

    The Named Type map is used for all types which derive
    void, namely base types, structs, unions, and aliases.
    (although the anonymous versions will not appear in the
    named map)

    The reason is that the Named map is only used to lookup
    when the name is already parsed into a token, this avoids
    having to do lots of string formatting.
*/

DefHashMapDecl(NamedTypeMap, StrID, u32);
DefHashMapDecl(OuterTypeMap, TypeID, u32);

typedef struct TypeInfo {
    NamedTypeMap nmap;
    OuterTypeMap omap;
    dynArray(Type) types;
    dynArray(Member) members;
} TypeInfo;

TypeID GetOuterID(u32 inner, TypeFlags flag);

#include <setjmp.h>

typedef enum ErrorCode {
    ERROR_NONE,
    ERROR_UNEXPECTED_TOKEN,
    ERROR_UNEXPECTED_EOF,
} ErrorCode;

typedef struct ParsingState {
    TokenBuffer *t;
    TypeInfo type;
    StrBase* base;
    u32 curr;

    struct {
        ErrorCode code;
        jmp_buf crash_handler;

        Token tok;
        TokenType expected;
        u32 parserLine;
    } err;
} ParsingState;

typedef struct ParsingState ParsingState;

//Current Plan is to just only proc on identifiers
typedef void (*proc)(ParsingState* p);
DefHashMapDecl(ProcMap, StrID, proc);

ParsingState BeginParser(Allocator a, TokenBuffer *buffer, StrBase* b);
void AddProc(ParsingState* p, proc newproc);
void RunParsers(ParsingState* p);

u32 ParseType(ParsingState* p);

void TypeInfoFree(TypeInfo* t);

#endif
