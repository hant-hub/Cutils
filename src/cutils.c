#include <cutils.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Memory Allocators
*/

static alloc_func_def(globalAllocator) {

    if (!new) {
        // free
        free(ptr);
        return NULL;
    }

    if (!ptr && !old) {
        // alloc
        return malloc(new);
    }

    // realloc
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

    SString tok = {.data = buffer.data};
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
    if (buffer.len)
        buffer.len--;

    return tok;
}

/*
    File Implementations
    Platform Dependent
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
file fileopen(const SString filename, file_permissions p) {

    int oflags = O_RDONLY;

    if ((p & FILE_WRITE) && (p & FILE_READ)) {
        oflags |= O_RDWR;
    } else if (p & FILE_WRITE) {
        oflags |= O_WRONLY;
    }

    if (p & FILE_CREAT) {
        oflags |= O_CREAT;
    }

    if (p & FILE_TRUNC) {
        oflags |= O_TRUNC;
    }

    int fd = open((char *)filename.data, oflags, 0666);

    struct stat statbuf;
    if (fd < 0 || fstat(fd, &statbuf)) {
        // error here
        // TODO(ELI): Once logging is working,
        // make log error and return invalid file
        debugerr("Filed to Open file: %n", filename);

        return (file){.handle = -1};
    }

    return (file){
        .handle = fd,
        .stats =
            {
                .size = statbuf.st_size,
            },
    };
}

static u64 flushdata(u64 handle, SString src) {
    u64 size = src.len;
    char *buffer = (char *)src.data;
    u64 accum = 0;

    while (accum != size) {
        u64 bytes = write(handle, buffer + accum, size - accum);

        if (bytes < 0) {
            // error
            panic();
        }

        if (bytes == 0) {
            // EOF
            break;
        }

        accum += bytes;
    }

    return accum;
}

void fileclose(file file) {
    printf("buf: %d, acc: %d\n", file.buffered, file.accum);
    if (file.buffered && file.accum) {
        flushdata(file.handle, (SString){.len = file.accum, .data = file.buf});
    }

    if (!file.closefd)
        close(file.handle);
}

u64 fileread(SString dst, file src) {
    u64 size = dst.len;
    char *buffer = (char *)dst.data;

    u64 accum = 0;

    while (accum != size) {
        i64 bytes = read(src.handle, buffer, size);
        if (bytes < 0) {
            // error
            panic();
        }

        if (bytes == 0) {
            // EOF
            break;
        }

        accum += bytes;
    }

    return accum;
}

u64 filewrite(file *dst, SString src) {
    u64 size = src.len;
    char *buffer = (char *)src.data;
    u64 accum = 0;

    if (!dst->buffered) {
        return flushdata(dst->handle, src);
    } else {
        for (u64 i = 0; i < size; i++) {
            if (dst->accum >= PAGE_SIZE) {
                dst->accum = 0;
                flushdata(dst->handle,
                          (SString){.len = PAGE_SIZE, .data = dst->buf});
            }

            dst->buf[dst->accum++] = buffer[i];
        }

        return size;
    }
}

void fileflush(file *file) {
    if (file->buffered && file->accum) {
        flushdata(file->handle,
                  (SString){.len = file->accum, .data = file->buf});
        file->accum = 0;
    }
}

u64 fileload(SString *handle, const SString filename);
u64 filesave(const SString filename, SString *handle);

/*
    Custom Format
*/

typedef void (*wchar)(char c, void *ctx);

typedef struct formatInfo {
    wchar f;
    void *ctx;
} formatInfo;

// 10^{i + 1}
static const u64 power10plus[] = {
    1UL,
    10UL,
    100UL,
    1000UL,
    10000UL,
    100000UL,
    1000000UL,
    10000000UL,
    100000000UL,
    1000000000UL,
    10000000000UL,
    100000000000UL,
    1000000000000UL,
    10000000000000UL,
    100000000000000UL,
    1000000000000000UL,
    10000000000000000UL,
    100000000000000000UL,
    1000000000000000000UL,
    10000000000000000000UL,
};

#define ERROL_EPS 0.0000001

static v2d hpnormalize(v2d x) {
    f64 val = x.x;
    x.x += x.y;
    x.y += val - x.x;

    return x;
}

static v2d hpmul10(v2d x) {
    f64 off, val = x.x;

    x.x *= 10.0;
    x.y *= 10.0;

    off = x.x;
    off -= val * 8.0;
    off -= val * 2.0;

    x.y -= off;
    return hpnormalize(x);
}

static v2d hpdiv10(v2d x) {
    f64 val = x.x;

    x.x /= 10.0;
    x.y /= 10.0;

    val -= x.x * 8.0;
    val -= x.x * 2.0;

    x.y += val / 10.0;

    return hpnormalize(x);
}

static f64 fpnext(f64 x) {
    union {
        f64 f;
        u64 i;
    } bits;
    bits.f = x;

    bits.i++;
    return bits.f;
}

static f64 fpprev(f64 x) {
    union {
        f64 f;
        u64 i;
    } bits;
    bits.f = x;

    bits.i--;
    return bits.f;
}

// Based on the implementation of the Errol0
// algorithm provided by the original paper.
static void format_float(formatInfo info, f64 num) {
    if (num < 0) {
        info.f('-', info.ctx);
        num *= -1;
    }

    f64 ten = 1.0;
    i32 exp = 1;
    v2d mid, inhi, inlo;

    mid.x = num;
    mid.y = 0.0;

    while (((mid.x > 10.0) || ((mid.x == 10.0) && (mid.y >= 0.0))) &&
           (exp < 308))
        exp++, mid = hpdiv10(mid), ten /= 10.0;

    while (((mid.x < 1.0) || ((mid.x == 1.0) && (mid.y < 0.0))) && (exp > -307))
        exp--, mid = hpmul10(mid), ten *= 10.0;

    inhi.x = mid.x;
    inhi.y = mid.y + (fpnext(num) - num) * ten / (2.0 + ERROL_EPS);

    inlo.x = mid.x;
    inlo.y = mid.y + (fpprev(num) - num) * ten / (2.0 + ERROL_EPS);

    inhi = hpnormalize(inhi);
    inlo = hpnormalize(inlo);

    while (inhi.x > 10.0 || (inhi.x == 10.0 && (inhi.y >= 0.0)))
        exp++, inhi = hpdiv10(inhi), inlo = hpdiv10(inlo);

    while (inhi.x < 1.0 || (inhi.x == 1.0 && (inhi.y < 0.0)))
        exp--, inhi = hpmul10(inhi), inlo = hpmul10(inlo);

    if (exp <= 0) {
        info.f('0', info.ctx);
        i32 shift = exp;
        if (shift < 0)
            info.f('.', info.ctx);

        while (shift < 0) {
            shift++;
            info.f('0', info.ctx);
        }
    }

    while (inhi.x != 0.0 || inhi.y != 0.0) {
        exp--;
        if (exp == -1) {
            info.f('.', info.ctx);
            continue;
        }

        u8 ldig, hdig = 0;

        hdig = (u8)(inhi.x);
        if ((inhi.x == hdig) && (inhi.y < 0))
            hdig -= 1;

        ldig = (u8)(inlo.x);
        if ((inlo.x == ldig) && (inlo.y < 0))
            ldig -= 1;

        if (ldig != hdig)
            break;

        // printf("digit: %d %d\n", hdig, ldig);
        info.f(hdig + '0', info.ctx);

        inhi.x -= hdig;
        inhi = hpmul10(inhi);

        inlo.x -= ldig;
        inlo = hpmul10(inlo);
    }

    f64 mdig = (inhi.x + inlo.x) / 2.0 + 0.5;
    info.f((u8)mdig + '0', info.ctx);
}

static ptrdiff_t format_arg(formatInfo info, va_list args, const char *format) {
    ptrdiff_t out = 0;
    switch (format[0]) {
    case 'd': {
        i64 s = va_arg(args, i32);
        if (s < 0) {
            info.f('-', info.ctx);
            s *= -1;
        }
        u64 num = s;

        // calc number of digits
        u32 digits = 0;
        for (u32 i = 0; i < ARRAY_SIZE(power10plus); i++) {
            if ((num / power10plus[i]) == 0UL)
                break;
            digits++;
        }

        // print digits
        for (i32 i = digits - 1; i >= 0; i--) {
            u32 digit = num / power10plus[i];
            num -= digit * power10plus[i];
            info.f(digit + '0', info.ctx);
        }

        out++;
    } break;
    case 'f': {
        f64 s = va_arg(args, f64);
        format_float(info, s);
        out++;
    } break;
    case 's': {
        SString s = va_arg(args, SString);
        for (u32 i = 0; i < s.len; i++) {
            info.f(s.data[i], info.ctx);
        }
        out++;
    } break;
    case 'n': {
        const char *s = va_arg(args, const char *);
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

static void vformat(formatInfo info, va_list args, const char *format) {

    while (format[0]) {
        switch (format[0]) {
        case '%':
            format++;
            format += format_arg(info, args, format);
            break;
        default: {
            info.f(format[0], info.ctx);
            format++;
        } break;
        }
    }
}

typedef struct strInfo {
    SString data;
    u32 idx;
} strInfo;

static void strput(char c, void *ctx) {
    strInfo *info = ctx;
    if (info->data.len > info->idx)
        info->data.data[info->idx++] = c;
}

u32 sformat(SString dst, const char *format, ...) {
    strInfo info = {.data = dst, .idx = 0};

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

typedef struct fileInfo {
    file *dst;
    u64 idx;
} fileInfo;

static void fileput(char c, void *ctx) {
    fileInfo *info = ctx;
    info->idx++;
    filewrite(info->dst, (SString){.len = 1, .data = (i8 *)&c});

    if (c == '\n') {
        fileflush(info->dst);
    }
}

u32 fformat(file *dst, const char *format, ...) {
    fileInfo info = {
        .dst = dst,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, args, format);
    va_end(args);
    return info.idx;
}

file logfile = {
    .handle = STDOUT_FILENO,
    .closefd = 1,
};

file warnfile = {
    .handle = STDERR_FILENO,
    .closefd = 1,
};
file errfile = {
    .handle = STDERR_FILENO,
    .closefd = 1,
};

u32 printlog(const char *format, ...) {
    fileInfo info = {
        .dst = &logfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, args, format);
    va_end(args);
    return info.idx;
}

u32 printwarn(const char *format, ...) {
    fileInfo info = {
        .dst = &warnfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, args, format);
    va_end(args);
    return info.idx;
}

u32 printerr(const char *format, ...) {
    fileInfo info = {
        .dst = &errfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, args, format);
    va_end(args);
    return info.idx;
}

/*
    Math
*/

v2u v2uAdd(v2u a, v2u b) { return (v2u){a.x + b.x, a.y + b.y}; }
v2i v2iAdd(v2i a, v2i b) { return (v2i){a.x + b.x, a.y + b.y}; }
v2f v2fAdd(v2f a, v2f b) { return (v2f){a.x + b.x, a.y + b.y}; }
v2d v2dAdd(v2d a, v2d b) { return (v2d){a.x + b.x, a.y + b.y}; }

v2u v2uMul(v2u a, v2u b) { return (v2u){a.x * b.x, a.y * b.y}; }
v2i v2iMul(v2i a, v2i b) { return (v2i){a.x * b.x, a.y * b.y}; }
v2f v2fMul(v2f a, v2f b) { return (v2f){a.x * b.x, a.y * b.y}; }
v2d v2dMul(v2d a, v2d b) { return (v2d){a.x * b.x, a.y * b.y}; }
