#include <cgen/cgen.h>

DefDynArrayImpl(TokenBuffer, Token);

DefHashMapImpl(NamedTypeMap, StrID, u32);
DefHashMapImpl(OuterTypeMap, TypeID, u32);
DefHashMapImpl(ProcMap, StrID, proc);

typedef enum TokenizerState {
    TOKENIZER_NONE,
    TOKENIZER_COMMENT_C,
    TOKENIZER_SKIP_LINE, // cpp comment + preprocessor directives
    TOKENIZER_ID,
} TokenizerState;

TokenBuffer Tokenize(SString data, StrBase *b, Allocator a) {
    TokenBuffer buffer = {a};

    i8 *chars = data.data;
    TokenizerState state = TOKENIZER_NONE;
    u32 curr_line = 1;
    u32 mark = 0;

    // setup constants

    StrID struct_name = StrBaseAdd(b, sstring("struct"));
    StrID typedef_name = StrBaseAdd(b, sstring("typedef"));

    for (u32 curr = 0; curr < data.len;) {
        switch (state) {
        case TOKENIZER_NONE: {
            if (chars[curr] == ' ' || chars[curr] == '\t') {
                curr++;
                break;
            }

            if (chars[curr] == '\n') {
                curr_line++;
                curr++;
                break;
            }

            // goto identifier handler
            if (isAlNum(chars[curr])) {
                state = TOKENIZER_ID;
                mark = curr;
                curr++;
                break;
            }

            // require at least 2 more chars
            if (curr + 1 < data.len) {

                // goto C comment handler
                if (chars[curr] == '/' && chars[curr + 1] == '*') {
                    curr += 2;
                    state = TOKENIZER_COMMENT_C;
                    break;
                }

                // goto Cpp comment handler
                if (chars[curr] == '/' && chars[curr + 1] == '/') {
                    curr += 2;
                    state = TOKENIZER_SKIP_LINE;
                    break;
                }
            }

            // skip preprocessor directives
            if (chars[curr] == '#') {
                curr++;
                state = TOKENIZER_SKIP_LINE;
                break;
            }

            // handle symbol
            TokenBufferPush(&buffer, (Token){.t = chars[curr]});
            curr++;
        } break;

        case TOKENIZER_ID: {
            if (isAlNum(chars[curr]) || chars[curr] == '_') {
                curr++;
                break; // continue;
            }

            StrID id = StrBaseAdd(
                b, (SString){.data = &chars[mark], .len = curr - mark});
            Token tok = {
                .id = id,
                .line = curr_line,
            };

            if (id == struct_name) {
                tok.t = TOKEN_STRUCT;
            } else if (id == typedef_name) {
                tok.t = TOKEN_TYPEDEF;
            } else {
                tok.t = TOKEN_ID;
            }
            TokenBufferPush(&buffer, tok);
            state = TOKENIZER_NONE;
        } break;

        case TOKENIZER_SKIP_LINE: {
            if (chars[curr] == '\n') {
                curr++;
                state = TOKENIZER_NONE;
                break;
            }

            curr++;
        } break;

        case TOKENIZER_COMMENT_C: {

            // at least 2 chars left
            if (curr + 1 < data.len) {
                if (chars[curr] == '*' && chars[curr] == '/') {
                    curr += 2;
                    state = TOKENIZER_NONE;
                    break;
                }
            }

            curr++;
        } break;

        default: {
            debugerr("Invalid Tokenizer State");
            panic();
        } break;
        }
    }

    return buffer;
}

SString TokenName(Token t, StrBase *b, ArenaAllocator *persist) {

    if (t.t < TOKEN_ID) {
        SString out = {
            .data = ArenaAlloc(persist, 1),
            .len = 1,
        };
        out.data[0] = (i8)t.t;
        return out;
    }

    SString token_names[] = {
        sstring("Id"),
        sstring("Struct"),
        sstring("Typedef"),
        sstring("Unknown Token"),
    };
    u32 idx = MIN(t.t - TOKEN_ID, ARRAY_SIZE(token_names) - 1);

    if (idx != 0) {
        return token_names[idx];
    }

    SString val = GetStr(b, t.id);
    SString out = {
        .data = ArenaAlloc(persist, token_names[idx].len + 2 + val.len),
        .len = 0,
    };

    for (u32 i = 0; i < token_names[idx].len; i++) {
        out.data[out.len++] = token_names[idx].data[i];
    }

    out.data[out.len++] = ':';
    out.data[out.len++] = ' ';

    for (u32 i = 0; i < val.len; i++) { out.data[out.len++] = val.data[i]; }

    return out;
}

void HandleError(ParsingState *p) {
    switch (p->err.code) {
    case ERROR_UNEXPECTED_EOF: {
        printerr("%d: Unexpected EOF, expected %d\n", p->err.parserLine, p->err.expected);
        panic();
    } break;
    case ERROR_UNEXPECTED_TOKEN: {
        printerr("%d: Unexpected token: Expected %d, Got %d at %d",
                p->err.parserLine, p->err.expected, p->err.tok.t, p->err.tok.line);
        panic();
    } break;
    default: {
        printerr("%d, %d Unknown Error Code!\n", p->err.parserLine, p->err.tok.line);
        panic();
    } break;
    }
}

void _Expect(ParsingState* p, TokenType t, u32 lineNum) {
    if (p->curr >= p->t->size) {
        //EOF
        p->err.code = ERROR_UNEXPECTED_EOF;
        p->err.parserLine = lineNum;
        longjmp(p->err.crash_handler, 1);
    }

    Token tok = p->t->data[p->curr++];

    if (tok.t != t) {
        //Unexpected Token
        p->err.code = ERROR_UNEXPECTED_TOKEN;
        p->err.parserLine = lineNum;
        p->err.expected = t;
        p->err.tok = tok;
        longjmp(p->err.crash_handler, 1);
    }
}

Token _NextToken(ParsingState* p, u32 lineNum) {
    p->curr++;
    if (p->curr >= p->t->size) {
        //EOF
        p->err.code = ERROR_UNEXPECTED_EOF;
        p->err.parserLine = lineNum;
        longjmp(p->err.crash_handler, 1);
    }

    return p->t->data[p->curr];
}

#define NextToken(p) _NextToken(p, __LINE__)
#define Expect(p, t) _Expect(p, t, __LINE__)

TypeID GetOuterID(u32 inner, TypeFlags flag) {
    return (inner << 4) | MSB16(flag);
}

u32 GetBaseType(ParsingState *p, StrID name) {
    TypeInfo *type = &p->type;

    u32 *idx = NamedTypeMapGet(&type->nmap, name);
    if (idx)
        return *idx; // no duplicates

    Type newtype = {
        .flags = TYPE_BASE,
        .name = name,
        .inner = 0, // type nil
    };

    NamedTypeMapIns(&type->nmap, name, type->types.size);
    dynPush(type->types, newtype);

    return type->types.size - 1;
}


u32 GetType(ParsingState* p, Type t) {
    TypeInfo *type = &p->type;
    
    TypeID id = GetOuterID(t.inner, t.flags);
    u32* tid = OuterTypeMapGet(&type->omap, id);
    if (tid) {
        return *tid;
    }

    OuterTypeMapIns(&type->omap, id, type->types.size);
    dynPush(type->types, t);

    return type->types.size - 1;
}


ParsingState BeginParser(Allocator a, TokenBuffer *buffer, StrBase *b) {
    ParsingState p = (ParsingState){
        .t = buffer,
        .base = b,
        .type =
            {
                .types.a = a,
                .members.a = a,
                .nmap.a = a,
                .omap.a = a,
            },
    };

    GetBaseType(&p, StrBaseAdd(b, sstring("void")));
    GetBaseType(&p, StrBaseAdd(b, sstring("char")));
    GetBaseType(&p, StrBaseAdd(b, sstring("int")));
    GetBaseType(&p, StrBaseAdd(b, sstring("float")));
    GetBaseType(&p, StrBaseAdd(b, sstring("long")));
    GetBaseType(&p, StrBaseAdd(b, sstring("double")));

    int v = setjmp(p.err.crash_handler);
    if (v) {
        HandleError(&p);
    }

    return p;
}

u32 ParseStruct(ParsingState *p);

u32 HandleIndirection(ParsingState *p, u32 base) {
    while (1) {
        Token t = NextToken(p);
        
        if (t.t != '*') {
            break;
        }

        Type ref = {
            .name = 0,
            .inner = base,
        };

        if (t.t == '*') {
            ref.flags = TYPE_POINTER;
        }
        
        base = GetType(p, ref);
    }

    return base;
}

u32 ParseType(ParsingState *p) {
    Token curr = p->t->data[p->curr];
    switch (p->t->data[p->curr].t) {
    case TOKEN_ID: {
        u32 base = GetBaseType(p, curr.id);
        base = HandleIndirection(p, base);

        return base;
    } break;
    case TOKEN_STRUCT: {
    } break;
    case TOKEN_TYPEDEF: {
    } break;
    }

    todo();
}

void AddProc(ParsingState *p, proc newproc);
void RunParsers(ParsingState *p);

void TypeInfoFree(TypeInfo* t) {
    NamedTypeMapFree(&t->nmap);
    OuterTypeMapFree(&t->omap);
    dynFree(t->types);
    dynFree(t->members);
}
