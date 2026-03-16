#ifndef CGEN_H
#define CGEN_H

#include <core/cutils.h>
#include <strbase/strbase.h>
#include <ds/ds.h>

typedef enum TokenType {
    TOKEN_ID = 128,
    TOKEN_STRUCT,
    TOKEN_TYPEDEF,
} TokenType;

typedef struct Token {
    TokenType t;
    u32 id; //side array info
    u32 line;
} Token;

DefDynArrayDecl(TokenBuffer, Token);

TokenBuffer Tokenize(SString data, StrBase* b, Allocator a);
SString TokenName(Token t, StrBase* b, ArenaAllocator* persist);


#endif
