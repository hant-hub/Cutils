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
    TYPE_VOID = 1 << 0,

    TYPE_CHAR = 1 << 1,
    TYPE_INT = 1 << 2,
    TYPE_FLOAT = 1 << 3,
    TYPE_LONG = 1 << 4,
    TYPE_DOUBLE = 1 << 5,

    //derived types
    TYPE_CONST = 1 << 6,
    TYPE_POINTER = 1 << 7,
    TYPE_ALIAS = 1 << 8,
    TYPE_STRUCT = 1 << 9,
    TYPE_UNION = 1 << 10,
    TYPE_ENUM = 1 << 11,
    TYPE_FUNC = 1 << 12,
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
    INFO(ELI): The Named Type Map is used to go
    from a string to a type, and the outer type map
    is used to go from a type we already have to see
    if there are any types that derive from it.

    We have key collisions regarding the nil type, but
    we can 
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

typedef struct ParsingState ParsingState;

//Current Plan is to just only proc on identifiers
typedef void (*proc)(ParsingState* p);
DefHashMapDecl(ProcMap, StrID, proc);

ParsingState* BeginParser(TokenBuffer* buffer);
void AddProc(ParsingState* p, proc newproc);
void RunParsers(ParsingState* p);

u32 ParseType(TypeInfo* info, ParsingState* p);

#endif
