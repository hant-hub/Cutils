#include <cutils.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
    Memory Allocators
*/

static alloc_func_def(globalAllocator) {
    
    if (!new) {
        //free
        free(ptr);
        return NULL; 
    }


    if (!ptr && !old) {
        //alloc
        return malloc(new);
    }

    //realloc
    return realloc(ptr, new);
}
const Allocator GlobalAllocator = {
    .a = globalAllocator,
};



/*
    String Implementations
*/

bool8 Sstrcmp(SString a, SString b) {
    return (a.len == b.len) && (memcmp(a.data, b.data, a.len) == 0);
}

SString Sstrdup(Allocator a, SString str) {
    SString new = {
        .len = str.len,
        .data = Alloc(a, str.len),
    };
    
    memcpy(new.data, str.data, str.len);
    return new;
}

SString Sstrtok(SString str, const char delim) {
    static SString buffer = {0};

    if (str.len) {
        buffer = str;
    }

    if (!buffer.len)
        return (SString){0};

    SString tok = {
        .data = buffer.data
    };
    while (buffer.len) {
        char c = buffer.data[0];
        if (c == delim) {
            break;
        }

        buffer.data++;
        buffer.len--;

        tok.len++;
    }

    buffer.data++;
    if (buffer.len) buffer.len--;

    return tok;
}

/*
    Custom Format
*/

typedef void(*wchar)(char c, void* ctx);

typedef struct formatInfo {
    wchar f;
    void* ctx;
} formatInfo;


typedef struct strInfo {
    SString data;
    u32 idx;
} strInfo;

static void strput(char c, void* ctx) {
    strInfo* info = ctx;
    if (info->data.len > info->idx)
        info->data.data[info->idx++] = c;
}

static const u64 power_table[] = {
    10UL,
    10UL * 10UL,
    10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
    10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL * 10UL,
};

static ptrdiff_t format_arg(formatInfo info, va_list args, const char* format) {
    ptrdiff_t out = 0;
    switch(format[0]) {
        case 'd':
        {
            i64 s = va_arg(args, i32);
            if (s < 0) {
                info.f('-', info.ctx);
                s *= -1;
            }
            u64 num = s;

            //calc number of digits
            u32 digits = 0;
            for (u32 i = 0; i < ARRAY_SIZE(power_table); i++) {
                if ((num/power_table[i]) == 0UL) break;
                digits++;
            }

            //print digits
            for (i32 i = digits - 1; i >= 0; i--) {
                u32 digit = num/power_table[i];
                num -= digit * power_table[i];
                info.f(digit + '0', info.ctx);
            }
            info.f(num + '0', info.ctx);

            out++;
        } break;
        case 's':
        {
            SString s = va_arg(args, SString);
            for (u32 i = 0; i < s.len; i++) {
                info.f(s.data[i], info.ctx);
            }
            out++;            
        } break;
        case 'n':
        {
            const char* s = va_arg(args, const char*);
            while (s[0]) {
                info.f(s[0], info.ctx);
                s++;
            }
            out++;            
        } break;
        default: {
            todo();
        }
    }
    return out;
}

static void vformat(formatInfo info, va_list args, const char* format) {

    while (format[0]) {
        switch (format[0]) {
            case '%':
                format++;
                format += format_arg(info, args, format);
                break;
            default:
                {
                    info.f(format[0], info.ctx);
                    format++;
                } break;
        }
    }

}

u32 sformat(SString dst, const char* format, ...) {
    strInfo info = {
        .data = dst,
        .idx = 0
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = strput,
    };

    va_list args;
    va_start(args, format);
    
    vformat(finfo, args, format);

    va_end(args);
    return info.idx;
}

