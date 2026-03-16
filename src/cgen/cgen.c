#include <cgen/cgen.h>

DefDynArrayImpl(TokenBuffer, Token);

typedef enum TokenizerState {
    TOKENIZER_NONE,
    TOKENIZER_COMMENT_C,
    TOKENIZER_COMMENT_CPP,
    TOKENIZER_ID,
} TokenizerState;

TokenBuffer Tokenize(SString data, StrBase* b, Allocator a) {
    TokenBuffer buffer = {a};

    i8 *chars = data.data;
    TokenizerState state = TOKENIZER_NONE;
    u32 curr_line = 1;
    u32 mark = 0;

    //setup constants

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

            //goto identifier handler
            if (isAlNum(chars[curr])) {
                state = TOKENIZER_ID;
                mark = curr;
                curr++;
                break;
            }

            //require at least 2 more chars
            if (curr + 1 < data.len) {

                //goto C comment handler
                if (chars[curr] == '/' && chars[curr + 1] == '*') {
                    curr += 2;
                    state = TOKENIZER_COMMENT_C;
                    break;
                }

                //goto Cpp comment handler
                if (chars[curr] == '/' && chars[curr + 1] == '/') {
                    curr += 2;
                    state = TOKENIZER_COMMENT_CPP;
                    break;
                }
            }

            //handle symbol
            TokenBufferPush(&buffer, (Token){.t = chars[curr]});
            curr++;
        } break;

        case TOKENIZER_ID: {
            if (isAlNum(chars[curr]) || chars[curr] == '_') {
                curr++;
                break; //continue;
            }
            
            StrID id = StrBaseAdd(b, (SString){.data = &chars[mark], .len = curr - mark});
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

        case TOKENIZER_COMMENT_C: {
            todo();
        } break;

        case TOKENIZER_COMMENT_CPP: {
            todo();
        } break;

        default: {
            debugerr("Invalid Tokenizer State");
            panic();
        } break;
        }

    }

    return buffer;
}

SString TokenName(Token t, StrBase* b, ArenaAllocator* persist) {

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

    for (u32 i = 0; i < val.len; i++) {
        out.data[out.len++] = val.data[i];
    }

    return out;
}
