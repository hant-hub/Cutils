#ifndef CUTILS_H
#define CUTILS_H

#include <stdint.h>

#define PAGE_SIZE (1 << 12)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/*
    Helpful typedefs
*/

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t bool8;
typedef uint16_t bool16;
typedef uint32_t bool32;
typedef uint64_t bool64;

typedef float f32;
typedef double f64;

/*
    Custom Memory Allocators
*/

#define NULL ((void *)0)

#define alloc_func_def(name)                                                   \
    void *(name)(void *ctx, void *ptr, u64 old, u64 new)

typedef alloc_func_def(*alloc_func);

typedef struct Allocator {
    alloc_func a;
    void *ctx;
} Allocator;

#define Alloc(mem, size) mem.a(mem.ctx, NULL, 0, size)

#define Free(mem, ptr, size) mem.a(mem.ctx, ptr, size, 0)

#define Realloc(mem, ptr, old, new) mem.a(mem.ctx, ptr, old, new)

extern const Allocator GlobalAllocator;

/*
    Sized Strings
*/

typedef struct SString {
    u32 len;
    i8 *data;
} SString;

typedef struct LString {
    u64 len;
    i8 *data;
} LString;

bool8 Sstrcmp(SString a, SString b);
SString Sstrdup(Allocator a, SString str);

// NOTE(ELI): Only a single delimiter at the moment,
// didn't think that using multiple deliminters would be
// that important
SString Sstrtok(SString str, const char delim);

#define sstring(x)                                                             \
    (SString) { sizeof(x) - 1, (i8 *)x }

/*
    FILE Handling
*/

// File descriptors and void* fit in FILE

typedef enum file_permissions {
    FILE_READ = 1 << 0,
    FILE_WRITE = 1 << 1,
    FILE_CREAT = 1 << 2,
    FILE_TRUNC = 1 << 3,
} file_permissions;

typedef struct file_info {
    u32 size;
    // In future stuff like time, last modified, etc.
} file_info;

typedef struct file {
    u64 handle; // on windows this is a void*, on linux an int
    file_info stats;

    u16 accum;
    bool8 closefd; // 0 means close, 1 means don't close
    bool8 buffered;
    i8 buf[PAGE_SIZE];
} file;

file fileopen(const SString filename, file_permissions p);
void fileclose(file file);

u64 fileread(SString dst, file src);
u64 filewrite(file *dst, SString src);
void fileflush(file *dst);

u64 fileload(SString *handle, const SString filename);
u64 filesave(const SString filename, SString *handle);

/*
    Custom Format
*/

/*
Format Table:

    %d -> int

    %n -> null terminated string

    %s -> SString (sized string)

    %f -> f64 (floating point)

*/
u32 sformat(SString dst, const char *format, ...);

/*
Format Table:

    %d -> int

    %n -> null terminated string

    %s -> SString (sized string)

    %f -> f64 (floating point)

*/
// Not threadsafe
u32 fformat(file *dst, const char *format, ...);

/*
    Custom Logging
*/

// initial settings:
//  logfile  -> stdout
//  warnfile -> stderr
//  errfile  -> stderr
extern file logfile;
extern file warnfile;
extern file errfile;

u32 printlog(const char *format, ...);
u32 printwarn(const char *format, ...);
u32 printerr(const char *format, ...);

#define debuglog(format, ...)                                                  \
    printlog("[LOG] " __FILE__ ":%d]:\t" format "\n", __LINE__, ##__VA_ARGS__)

#define debugwarn(format, ...)                                                 \
    printwarn("\033[38;2;255;255;0m[WARN]" __FILE__ ":%d]:\t" format           \
              "\033[0m\n",                                                     \
              __LINE__, ##__VA_ARGS__)

#define debugerr(format, ...)                                                  \
    printerr("\033[38;2;255;0;0m[ERR] " __FILE__ ":%d]:\t" format "\033[0m\n", \
             __LINE__, ##__VA_ARGS__)

/*
    Assertions
*/
#include <assert.h>

#define todo() assert(0)

#define panic() assert(0)

/*
    Math
*/

typedef struct v2u {
    u32 x;
    u32 y;
} v2u;

typedef struct v2i {
    i32 x;
    i32 y;
} v2i;

typedef struct v2l {
    i64 x;
    i64 y;
} v2l;

typedef struct v2f {
    f32 x;
    f32 y;
} v2f;

typedef struct v2d {
    f64 x;
    f64 y;
} v2d;

v2u v2uAdd(v2u a, v2u b);
v2i v2iAdd(v2i a, v2i b);
v2f v2fAdd(v2f a, v2f b);
v2d v2dAdd(v2d a, v2d b);

v2u v2uMul(v2u a, v2u b);
v2i v2iMul(v2i a, v2i b);
v2f v2fMul(v2f a, v2f b);
v2d v2dMul(v2d a, v2d b);

#endif
