/*
    Important!!!
    Must compile with -I set to include
*/

#ifndef SB_H
#define SB_H
#ifndef CUTILS_H
#define CUTILS_H

#include <pthread.h>
#include <stdint.h>

#define PAGE_SIZE (1 << 12L)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define Spread(...) __VA_ARGS__

#define KB(x) (x << 10L)
#define MB(x) (KB(x) << 10L)
#define GB(x) (MB(x) << 10L)

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(min, max, t) (MAX(MIN(max, t), min))

#define MSB16(i) (15 - __builtin_clz((unsigned int)i))
#define MSB32(i) (31 - __builtin_clz(i))
#define MSB64(i) (63 - __builtin_clzll(i))

#define CUTILS_INLINE __inline __attribute__((always_inline))

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

#define FALSE 0
#define TRUE 1

typedef uint8_t bool8;
typedef uint16_t bool16;
typedef uint32_t bool32;
typedef uint64_t bool64;

typedef float f32;
typedef double f64;

/*
    Endian Utils
*/

u16 FlipEndian16(u16 i);
u32 FlipEndian32(u32 i);
u64 FlipEndian64(u64 i);

f32 FlipEndianF(f32 f);
f64 FlipEndianD(f64 f);

/*
    Custom Memory Allocators
*/

#define NULL ((void*)0)

#define alloc_func_def(name) void*(name)(void* ctx, void* ptr, u64 old, u64 new)

typedef alloc_func_def(*alloc_func);

typedef struct Allocator {
    alloc_func a;
    void* ctx;
} Allocator;

#define Alloc(mem, size) mem.a(mem.ctx, NULL, 0, size)

#define Free(mem, ptr, size) mem.a(mem.ctx, ptr, size, 0)

#define Realloc(mem, ptr, old, new) mem.a(mem.ctx, ptr, old, new)

extern const Allocator GlobalAllocator;

// dynamic arena
typedef struct ArenaAllocator {
    Allocator a;
    u64 cap;
    u64 size;
    char data[];
}* ArenaAllocator;

ArenaAllocator ArenaCreate(Allocator a, u64 size);

u64 ArenaGetPos(ArenaAllocator a);
void ArenaSetPos(ArenaAllocator a, u64 pos);

void* ArenaAlloc(ArenaAllocator a, u64 size);
u64 ArenaExtendLast(ArenaAllocator a, u64 size);
void* ArenaAllocZero(ArenaAllocator a, u64 size);

void ArenaPop(ArenaAllocator a, u64 size);
void ArenaClear(ArenaAllocator a);

void ArenaDestroy(ArenaAllocator s);

// make generic allocator from Arena
Allocator ArenaGetAllocator(ArenaAllocator a);

// Scratch Arena
typedef struct ScratchArena {
    ArenaAllocator arena;
    u64 base;
} ScratchArena;

ScratchArena ScratchArenaGet(ArenaAllocator persist);
ScratchArena ScratchArenaBegin(ArenaAllocator a);
void ScratchArenaEnd(ScratchArena s);

// fixed size arena
typedef struct StackAllocator {
    u64 cap;
    u64 size;
    char data[];
}* StackAllocator;

StackAllocator StackCreate(Allocator a, u64 size);
void StackDestroy(Allocator a, StackAllocator s);

void* StackAlloc(StackAllocator s, u64 size);
void StackSet(StackAllocator s, u64 pos);
void StackReset(StackAllocator s);

/*
    Sized Strings
*/

typedef struct SString {
    u32 len;
    i8* data;
} SString;

typedef struct LString {
    u64 len;
    i8* data;
} LString;

bool8 Sstrcmp(SString a, SString b);
SString Sstrdup(Allocator a, SString str);

char* SStringGetCstr(SString s, ArenaAllocator a);

/*
    Array Slices
*/

#define slice(type)                                                            \
    struct {                                                                   \
        type* data;                                                            \
        u32 size;                                                              \
    }

#define sliceAssign(slice, arr, start, end)                                    \
    do {                                                                       \
        slice.data = &arr[start];                                              \
        slice.size = end - start;                                              \
    } while (0)

// NOTE(ELI): Only a single delimiter at the moment,
// didn't think that using multiple deliminters would be
// that important
SString Sstrtok(SString str, const char delim);

#define sstring(x)                                                             \
    (SString) { sizeof(x) - 1, (i8*)x }

#define sstringify(data, len)                                                  \
    (SString) { len, data }

// helpers
#define isAlpha(c) (((c | 32) - 'a') < 26u)

#define isNum(c) ((c - '0') <= 9u)

#define isAlNum(c) (isAlpha(c) || isNum(c))

#define isSpace(c) (c == ' ' || (c >= '\t' && c <= '\r'))

/*
    TIME Helpers
*/

typedef struct time_spec {
    u64 s;
    u64 ns;
} time_spec;

#define timecmp(t1, op, t2) (t1.s == t2.s ? t1.ns op t2.ns : t1.s op t2.s)

/*
    Lightweight Mutex
*/

typedef union Mutex {
    u32 data[1];
} Mutex;


void MutexLock(Mutex* mutex);
void MutexUnlock(Mutex* mutex);

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

    time_spec access;
    time_spec modified;

    // In future stuff like time, last modified, etc.
} file_info;

typedef struct file {
    u64 handle; // on windows this is a void*, on linux an int
    file_info stats;

    u16 accum;
    bool8 closefd; // 0 means close, 1 means don't close
    bool8 buffered;

    i8 buf[PAGE_SIZE];

    bool8 thread_safe;
    Mutex m;
} file;

file fileopen(const SString filename, file_permissions p);
void fileclose(file file);

file filestat(const SString filename);

u64 fileread(SString dst, file src);
u64 filewrite(file* dst, SString src);
void fileflush(file* dst);

u64 fileload(SString handle, const SString filename);
u64 filesave(const SString filename, SString handle);

void filedelete(SString filename);

void setdir(SString dir);
void setdirExe();

SString filepathpopdir(SString filepath);
SString filepathbasename(SString filepath);
SString filepathext(SString filepath);
u32 filepathdepth(SString filepath);

/*
    Directory Searching
*/
typedef enum DirType {
    T_FILE,
    T_DIR,
    T_END,
    T_UNKNOWN,
} DirType;

void DirBegin(SString filepath);
SString DirNext(DirType* t);

void DirBeginRec(SString filepath, u32 max_depth);
SString DirNextRec(DirType* t);

/*
    Custom Format
*/

/*
Format Table:

    %u -> int (unsigned)

    %d -> int (decimal)

    %x -> int (unsigned hex)

    %l -> long

    %n -> null terminated string

    %s -> SString (sized string)

    %f -> f64 (floating point)

    %c -> char (print int as char)

*/
u32 sformat(SString dst, const char* format, ...);

/*
Format Table:

    %u -> int (unsigned)

    %d -> int (decimal)

    %x -> int (unsigned hex)

    %l -> long (decimal by default)

    %n -> null terminated string

    %s -> SString (sized string)

    %f -> f64 (floating point)

    %c -> char (print int as char)
*/
u32 fformat(file* dst, const char* format, ...);

/*
    Custom Logging
*/

// initial settings:
//  logfile  -> stdout
//  warnfile -> stderr
//  errfile  -> stderr
extern file* logfile;
extern file* warnfile;
extern file* errfile;

u32 printlog(const char* format, ...);
u32 printwarn(const char* format, ...);
u32 printerr(const char* format, ...);

#define debuglog(format, ...)                                                  \
    printlog("\e[0m[LOG] " __FILE__ ":%d]:\t" format "\n", __LINE__,           \
             ##__VA_ARGS__)

#define debugwarn(format, ...)                                                 \
    printwarn("\e[38;2;255;255;0m[WARN]" __FILE__ ":%d]:\t" format "\e[0m\n",  \
              __LINE__, ##__VA_ARGS__)

#define debugerr(format, ...)                                                  \
    printerr("\e[38;2;255;0;0m[ERR] " __FILE__ ":%d]:\t" format "\e[0m\n",     \
             __LINE__, ##__VA_ARGS__)

/*
    Assertions
*/
#include <assert.h>

#define todo() assert(0)
#define panic() assert(0)
#define arrAssert(arr1, arr2, size)                                            \
    do {                                                                       \
        for (u64 i = 0; i < size; i++) { assert(arr1[i] == arr2[i]); }         \
    } while (0)

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

u64 FNVHash64(u8* data, u64 size);
u32 FNVHash32(u8* data, u32 size);

#endif

#ifdef SB_IMPL
#define DS_IMPL
#endif
#ifndef DS_H
#define DS_H
#include <string.h>

/*
    Inline Dynamic Arrays
*/

#define dynArray(type)                                                         \
    struct {                                                                   \
        Allocator a;                                                           \
        u64 size;                                                              \
        u64 cap;                                                               \
        type* data;                                                            \
    }

#define dynPush(array, val)                                                    \
    ((array.size + 1 > array.cap)                                              \
         ? (array.data = Realloc(                                              \
                array.a, array.data, array.cap * sizeof(array.data[0]),        \
                (array.cap ? array.cap * 2 : 4) * sizeof(array.data[0])),      \
            array.cap = array.cap ? array.cap * 2 : 4)                         \
         : 0,                                                                  \
     array.data[array.size++] = val)

#define dynReserve(array, size)                                                \
    ((array.cap < size)                                                        \
         ? (array.data = Realloc(array.a, array.data,                          \
                                 array.cap * sizeof(array.data[0]),            \
                                 size * sizeof(array.data[0])),                \
            array.cap = size)                                                  \
         : 0)

// INFO(ELI): This macro can't be included in expressions and must be
// on its own line. This is due to the loops which I could fix if
// I added more global functions but I decided I preferred to not
// do that.
#define dynResize(array, newsize)                                              \
    do {                                                                       \
        if (array.cap < newsize) {                                             \
            u64 oldsize = array.cap;                                           \
            while (array.cap < newsize) {                                      \
                array.cap = array.cap ? array.cap * 2 : 4;                     \
            }                                                                  \
            array.data =                                                       \
                Realloc(array.a, array.data, oldsize * sizeof(array.data[0]),  \
                        array.cap * sizeof(array.data[0]));                    \
        }                                                                      \
                                                                               \
        if (array.size < newsize) {                                            \
            char* d = (char*)&array.data[array.size];                          \
            for (u32 i = 0;                                                    \
                 i < (newsize - array.size) * sizeof(array.data[0]); i++)      \
                d[i] = 0;                                                      \
        }                                                                      \
        array.size = newsize;                                                  \
    } while (0);

#define dynBack(array) (array.data[array.size - 1])

#define dynFree(array)                                                         \
    (Free(array.a, array.data, array.cap * sizeof(array.data[0])))

// INFO(ELI): These Macros cannot be part of expressions since they require
// loops.

#define dynIns(array, idx, val)                                                \
    do {                                                                       \
        dynResize(array, array.size + 1);                                      \
        for (i64 i = array.size - 1; i >= idx; i--) {                          \
            array.data[i + 1] = array.data[i];                                 \
        }                                                                      \
        array.data[idx] = val;                                                 \
    } while (0)

#define dynDel(array, idx)                                                     \
    do {                                                                       \
        for (u32 i = idx; i < array.size - 1; i++) {                           \
            array.data[i] = array.data[i + 1];                                 \
        }                                                                      \
        dynResize(array, array.size - 1);                                      \
    } while (0)

#define dynExt(array, vals, num)                                               \
    do {                                                                       \
        dynResize(array, array.size + (num));                                  \
        for (u64 i = array.size - (num); i < array.size; i++) {                \
            array.data[i] = (vals)[i - array.size + (num)];                    \
        }                                                                      \
    } while (0)

/*
    Forward Dynamic Arrays
*/
#define DefDynArrayDecl(name, type)                                            \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u64 size;                                                              \
        u64 cap;                                                               \
        type* data;                                                            \
    } name;                                                                    \
    void name##Push(name* arr, type val);                                      \
    void name##Reserve(name* arr, u64 size);                                   \
    void name##Resize(name* arr, u64 size);                                    \
    type name##Back(name* arr);                                                \
    void name##Free(name* arr);                                                \
    void name##Ins(name* arr, u64 idx, type val);                              \
    void name##Del(name* arr, u64 idx);                                        \
    void name##Ext(name* arr, type* vals, u64 num)

#define DefDynArrayImpl(name, type)                                            \
    void name##Push(name* arr, type val) {                                     \
        if (arr->size + 1 > arr->cap) {                                        \
            u64 oldcap = arr->cap;                                             \
                                                                               \
            arr->cap = arr->cap ? arr->cap * 2 : 4;                            \
            arr->data = Realloc(arr->a, arr->data, oldcap * sizeof(val),       \
                                arr->cap * sizeof(val));                       \
        }                                                                      \
        arr->data[arr->size++] = val;                                          \
    }                                                                          \
    void name##Reserve(name* arr, u64 size) {                                  \
        if (arr->cap < size) {                                                 \
            arr->data =                                                        \
                Realloc(arr->a, arr->data, arr->cap * sizeof(arr->data[0]),    \
                        size * sizeof(arr->data[0]));                          \
            arr->cap = size;                                                   \
        }                                                                      \
    }                                                                          \
    void name##Resize(name* arr, u64 size) {                                   \
        if (arr->cap < size) {                                                 \
            u64 oldsize = arr->cap;                                            \
            while (arr->cap < size) {                                          \
                arr->cap = arr->cap ? arr->cap * 2 : 4;                        \
            }                                                                  \
            arr->data =                                                        \
                Realloc(arr->a, arr->data, oldsize * sizeof(arr->data[0]),     \
                        arr->cap * sizeof(arr->data[0]));                      \
        }                                                                      \
                                                                               \
        if (arr->size < size) {                                                \
            char* d = (char*)&arr->data[arr->size];                            \
            u32 len = (size - arr->size) * sizeof(arr->data[0]);               \
            for (u32 i = 0; i < len; i++) d[i] = 0;                            \
        }                                                                      \
        arr->size = size;                                                      \
    }                                                                          \
    type name##Back(name* arr) { return arr->data[arr->size - 1]; }            \
    void name##Free(name* arr) {                                               \
        Free(arr->a, arr->data, arr->cap * sizeof(arr->data[0]));              \
    }                                                                          \
    void name##Ins(name* arr, u64 idx, type val) {                             \
        name##Resize(arr, arr->size + 1);                                      \
        for (i64 i = arr->size - 1; i >= idx; i--) {                           \
            arr->data[i + 1] = arr->data[i];                                   \
        }                                                                      \
        arr->data[idx] = val;                                                  \
    }                                                                          \
    void name##Del(name* arr, u64 idx) {                                       \
        for (u32 i = idx; i < arr->size - 1; i++) {                            \
            arr->data[i] = arr->data[i + 1];                                   \
        }                                                                      \
        name##Resize(arr, arr->size - 1);                                      \
    }                                                                          \
    void name##Ext(name* arr, type* vals, u64 num) {                           \
        name##Resize(arr, arr->size + num);                                    \
        for (u64 i = arr->size - num; i < arr->size; i++) {                    \
            arr->data[i] = vals[i - arr->size + num];                          \
        }                                                                      \
    }

/*
    HashMap

    One is for non string keys, other is for arbitrary sized string
    key
*/

#define MAX_LOAD 0.9
#define MIN_CAP 16

#define DefHashMapDecl(name, key, val)                                         \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        struct {                                                               \
            key k;                                                             \
            i32 m;                                                             \
        }* keys;                                                               \
        val* vals;                                                             \
    } name;                                                                    \
    void name##Ins(name*, key, val);                                           \
    val* name##Get(name*, key);                                                \
    void name##Del(name*, key);                                                \
    void name##Clear(name*);                                                   \
    void name##Free(name*);

#define DefHashMapImpl(name, key, val)                                         \
    void name##Ins(name* map, key ky, val vl) {                                \
        if (map->cap * MAX_LOAD <= map->size + 1) {                            \
            u64 oldcap = map->cap;                                             \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
                                                                               \
            typeof(map->keys) oldkeys = map->keys;                             \
            typeof(map->vals) oldvals = map->vals;                             \
                                                                               \
            map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));        \
            map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));        \
                                                                               \
            memset(map->keys, -1, map->cap * sizeof(map->keys[0]));            \
            for (u32 i = 0; i < oldcap; i++) {                                 \
                if (oldkeys[i].m == -1)                                        \
                    continue;                                                  \
                                                                               \
                key tempk = oldkeys[i].k;                                      \
                val tempv = oldvals[i];                                        \
                u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));              \
                                                                               \
                u32 idx = hash % map->cap;                                     \
                u32 dist = 0;                                                  \
                                                                               \
                for (u64 i = 0; i < map->cap; i++) {                           \
                    if (map->keys[idx].m == -1) {                              \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    if (map->keys[idx].m < dist) {                             \
                        key tmpkey = map->keys[idx].k;                         \
                        val tmpval = map->vals[idx];                           \
                        u32 tmpdist = map->keys[idx].m;                        \
                                                                               \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                                                                               \
                        tempk = tmpkey;                                        \
                        tempv = tmpval;                                        \
                        dist = tmpdist;                                        \
                    }                                                          \
                                                                               \
                    if (memcmp(&map->keys[idx].k, &tempk, sizeof(key)) == 0) { \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    idx = (idx + 1) % map->cap;                                \
                    dist++;                                                    \
                }                                                              \
            }                                                                  \
                                                                               \
            Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));              \
            Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));              \
        }                                                                      \
                                                                               \
        key tempk = ky;                                                        \
        val tempv = vl;                                                        \
        u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                map->size++;                                                   \
                break;                                                         \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                key tmpkey = map->keys[idx].k;                                 \
                val tmpval = map->vals[idx];                                   \
                u32 tmpdist = map->keys[idx].m;                                \
                                                                               \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                                                                               \
                tempk = tmpkey;                                                \
                tempv = tmpval;                                                \
                dist = tmpdist;                                                \
            }                                                                  \
                                                                               \
            if (memcmp(&map->keys[idx].k, &tempk, sizeof(key)) == 0) {         \
                map->vals[idx] = tempv;                                        \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
    }                                                                          \
    val* name##Get(name* map, key ky) {                                        \
        if (!map->cap)                                                         \
            return NULL;                                                       \
        key tempk = ky;                                                        \
        u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return NULL;                                                   \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return NULL;                                                   \
            }                                                                  \
                                                                               \
            if (memcmp(&map->keys[idx].k, &tempk, sizeof(key)) == 0) {         \
                return &map->vals[idx];                                        \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        return NULL;                                                           \
    }                                                                          \
    void name##Del(name* map, key ky) {                                        \
                                                                               \
        key tempk = ky;                                                        \
                                                                               \
        u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return;                                                        \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return;                                                        \
            }                                                                  \
                                                                               \
            if (memcmp(&map->keys[idx].k, &tempk, sizeof(key)) == 0) {         \
                map->size--;                                                   \
                idx = (idx + 1) % map->cap;                                    \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        u32 prev = (idx + map->cap - 1) % map->cap;                            \
        while (map->keys[idx].m != -1 && map->keys[idx].m > 0) {               \
            map->keys[prev] = map->keys[idx];                                  \
            map->keys[prev].m -= 1;                                            \
                                                                               \
            map->vals[prev] = map->vals[idx];                                  \
            idx = (idx + 1) % map->cap;                                        \
            prev = (idx + map->cap - 1) % map->cap;                            \
        }                                                                      \
                                                                               \
        map->keys[prev].m = -1;                                                \
    }                                                                          \
    void name##Reserve(name* map, u64 size) {                                  \
        u32 oldcap = map->cap;                                                 \
        while (map->cap < size) {                                              \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
        }                                                                      \
                                                                               \
        typeof(map->keys) oldkeys = map->keys;                                 \
        typeof(map->vals) oldvals = map->vals;                                 \
                                                                               \
        map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));            \
        map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));            \
                                                                               \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
        for (u32 i = 0; i < oldcap; i++) {                                     \
            if (oldkeys[i].m == -1)                                            \
                continue;                                                      \
                                                                               \
            key tempk = oldkeys[i].k;                                          \
            val tempv = oldvals[i];                                            \
            u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                  \
                                                                               \
            u32 idx = hash % map->cap;                                         \
            u32 dist = 0;                                                      \
                                                                               \
            for (u64 i = 0; i < map->cap; i++) {                               \
                if (map->keys[idx].m == -1) {                                  \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                if (map->keys[idx].m < dist) {                                 \
                    key tmpkey = map->keys[idx].k;                             \
                    val tmpval = map->vals[idx];                               \
                    u32 tmpdist = map->keys[idx].m;                            \
                                                                               \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                                                                               \
                    tempk = tmpkey;                                            \
                    tempv = tmpval;                                            \
                    dist = tmpdist;                                            \
                }                                                              \
                                                                               \
                if (memcmp(&map->keys[idx].k, &tempk, sizeof(key)) == 0) {     \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                idx = (idx + 1) % map->cap;                                    \
                dist++;                                                        \
            }                                                                  \
        }                                                                      \
                                                                               \
        Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));                  \
        Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));                  \
    }                                                                          \
    void name##Clear(name* map) {                                              \
        map->size = 0;                                                         \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
    }                                                                          \
    void name##Free(name* map) {                                               \
        Free(map->a, map->keys, map->cap * sizeof(map->keys[0]));              \
        Free(map->a, map->vals, map->cap * sizeof(map->vals[0]));              \
    }

#define DefStringMapDecl(name, val)                                            \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        struct {                                                               \
            SString k;                                                         \
            i32 m;                                                             \
        }* keys;                                                               \
        val* vals;                                                             \
    } name;                                                                    \
    void nameIns(name*, SString, val);                                         \
    val* nameGet(name*, SString);                                              \
    void nameDel(name*, SString);                                              \
    void nameFree(name*);

#define DefStringMapImpl(name, val)                                            \
    void name##Ins(name* map, SString ky, val vl) {                            \
        if (map->cap * MAX_LOAD <= map->size + 1) {                            \
            u64 oldcap = map->cap;                                             \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
                                                                               \
            typeof(map->keys) oldkeys = map->keys;                             \
            typeof(map->vals) oldvals = map->vals;                             \
                                                                               \
            map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));        \
            map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));        \
                                                                               \
            memset(map->keys, -1, map->cap * sizeof(map->keys[0]));            \
            for (u32 i = 0; i < oldcap; i++) {                                 \
                if (oldkeys[i].m == -1)                                        \
                    continue;                                                  \
                                                                               \
                typeof(map->keys[0].k) tempk = oldkeys[i].k;                   \
                typeof(map->vals[0]) tempv = oldvals[i];                       \
                u32 hash = FNVHash32((u8*)tempk.data, tempk.len);              \
                                                                               \
                u32 idx = hash % map->cap;                                     \
                u32 dist = 0;                                                  \
                                                                               \
                for (u64 i = 0; i < map->cap; i++) {                           \
                    if (map->keys[idx].m == -1) {                              \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    if (map->keys[idx].m < dist) {                             \
                        typeof(map->keys[0].k) tmpkey = map->keys[idx].k;      \
                        typeof(map->vals[0]) tmpval = map->vals[idx];          \
                        u32 tmpdist = map->keys[idx].m;                        \
                                                                               \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                                                                               \
                        tempk = tmpkey;                                        \
                        tempv = tmpval;                                        \
                        dist = tmpdist;                                        \
                    }                                                          \
                                                                               \
                    if (Sstrcmp(map->keys[idx].k, tempk)) {                    \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    idx = (idx + 1) % map->cap;                                \
                    dist++;                                                    \
                }                                                              \
            }                                                                  \
                                                                               \
            Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));              \
            Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));              \
        }                                                                      \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
        typeof(map->vals[0]) tempv = vl;                                       \
        u32 hash = FNVHash32((u8*)tempk.data, tempk.len);                      \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                map->size++;                                                   \
                break;                                                         \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                typeof(map->keys[0].k) tmpkey = map->keys[idx].k;              \
                typeof(map->vals[0]) tmpval = map->vals[idx];                  \
                u32 tmpdist = map->keys[idx].m;                                \
                                                                               \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                                                                               \
                tempk = tmpkey;                                                \
                tempv = tmpval;                                                \
                dist = tmpdist;                                                \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                map->vals[idx] = tempv;                                        \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
    }                                                                          \
    val* name##Get(name* map, SString ky) {                                    \
        typeof(map->keys[0].k) tempk = ky;                                     \
        u32 hash = FNVHash32((u8*)tempk.data, tempk.len);                      \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return NULL;                                                   \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return NULL;                                                   \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                return &map->vals[idx];                                        \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        return NULL;                                                           \
    }                                                                          \
    void name##Del(name* map, SString ky) {                                    \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
                                                                               \
        u32 hash = FNVHash32((u8*)tempk.data, tempk.len);                      \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return;                                                        \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return;                                                        \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                map->size--;                                                   \
                idx = (idx + 1) % map->cap;                                    \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        u32 prev = (idx + map->cap - 1) % map->cap;                            \
        while (map->keys[idx].m != -1 && map->keys[idx].m > 0) {               \
            map->keys[prev] = map->keys[idx];                                  \
            map->keys[prev].m -= 1;                                            \
                                                                               \
            map->vals[prev] = map->vals[idx];                                  \
            idx = (idx + 1) % map->cap;                                        \
            prev = (idx + map->cap - 1) % map->cap;                            \
        }                                                                      \
                                                                               \
        map->keys[prev].m = -1;                                                \
    }                                                                          \
    void name##Reserve(name* map, u64 size) {                                  \
        u32 oldcap = map->cap;                                                 \
        while (map->cap < size) {                                              \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
        }                                                                      \
                                                                               \
        typeof(map->keys) oldkeys = map->keys;                                 \
        typeof(map->vals) oldvals = map->vals;                                 \
                                                                               \
        map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));            \
        map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));            \
                                                                               \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
        for (u32 i = 0; i < oldcap; i++) {                                     \
            if (oldkeys[i].m == -1)                                            \
                continue;                                                      \
                                                                               \
            typeof(map->keys[0].k) tempk = oldkeys[i].k;                       \
            typeof(map->vals[0]) tempv = oldvals[i];                           \
            u32 hash = FNVHash32((u8*)tempk.data, tempk.len);                  \
                                                                               \
            u32 idx = hash % map->cap;                                         \
            u32 dist = 0;                                                      \
                                                                               \
            for (u64 i = 0; i < map->cap; i++) {                               \
                if (map->keys[idx].m == -1) {                                  \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                if (map->keys[idx].m < dist) {                                 \
                    typeof(map->keys[0].k) tmpkey = map->keys[idx].k;          \
                    typeof(map->vals[0]) tmpval = map->vals[idx];              \
                    u32 tmpdist = map->keys[idx].m;                            \
                                                                               \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                                                                               \
                    tempk = tmpkey;                                            \
                    tempv = tmpval;                                            \
                    dist = tmpdist;                                            \
                }                                                              \
                                                                               \
                if (Sstrcmp(map->keys[idx].k, tempk)) {                        \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                idx = (idx + 1) % map->cap;                                    \
                dist++;                                                        \
            }                                                                  \
        }                                                                      \
                                                                               \
        Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));                  \
        Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));                  \
    }                                                                          \
    void name##Clear(name* map) {                                              \
        map->size = 0;                                                         \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
    }                                                                          \
    void name##Free(name* map) {                                               \
        Free(map->a, map->keys, map->cap * sizeof(map->keys[0]));              \
        Free(map->a, map->vals, map->cap * sizeof(map->vals[0]));              \
    }

/*
    Memory Pool implementation

    Used for allocating pools
*/

#define PoolDecl(name, item)                                                   \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        item* slots;                                                           \
        u32* freelist;                                                         \
        u32 cap;                                                               \
        u32 freesize;                                                          \
    } name;                                                                    \
    u32 name##Alloc(name* p);                                                  \
    void name##Free(name* p, u32 idx);                                         \
    void name##Destroy(name* p);

#define PoolImpl(name, item)                                                   \
    u32 name##Alloc(name* p) {                                                 \
        if (p->freesize == 0) {                                                \
            u32 oldcap = p->cap;                                               \
            p->cap = p->cap ? p->cap * 2 : 8;                                  \
            p->slots = Realloc(p->a, p->slots, oldcap * sizeof(item),          \
                               p->cap * sizeof(item));                         \
            p->freelist = Realloc(p->a, p->freelist, oldcap * sizeof(u32),     \
                                  p->cap * sizeof(u32));                       \
            for (u32 i = oldcap; i < p->cap; i++) {                            \
                p->slots[i] = (item){};                                        \
                p->freelist[p->freesize++] = i;                                \
            }                                                                  \
        }                                                                      \
        return p->freelist[--p->freesize];                                     \
    }                                                                          \
    void name##Free(name* p, u32 idx) { p->freelist[p->freesize++] = idx; }    \
    void name##Destroy(name* p) {                                              \
        Free(p->a, p->slots, p->cap * sizeof(item));                           \
        Free(p->a, p->freelist, p->cap * sizeof(u32));                         \
    }

/*
    Simple Queue

    Fixed Size Queue of items
*/

#define DefSimpleQueueDecl(name, type)                                         \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        u32 first;                                                             \
        type* data;                                                            \
    } name

#define DefSimpleQueueImpl(name, type)                                         \
    name name##Create(Allocator a, u32 max_size) {                             \
        name q = {                                                             \
            .a = a,                                                            \
            .cap = max_size,                                                   \
        };                                                                     \
        q.data = Alloc(a, max_size * sizeof(type));                            \
        return q;                                                              \
    }                                                                          \
    void name##Destroy(name* q) {                                              \
        Free(q->a, q->data, q->cap * sizeof(type));                            \
        q->cap = 0;                                                            \
    }                                                                          \
    bool8 name##Empty(name* q) { return q->size == 0; }                        \
    bool8 name##Full(name* q) { return q->size == q->cap; }                    \
    bool8 name##Push(name* q, type item) {                                     \
        if (name##Full(q)) {                                                   \
            return FALSE;                                                      \
        }                                                                      \
        u32 slot = (q->first + q->size) % q->cap;                              \
        q->data[slot] = item;                                                  \
        q->size++;                                                             \
        return TRUE;                                                           \
    }                                                                          \
    bool8 name##Pop(name* q, type* out) {                                      \
        if (name##Empty(q)) {                                                  \
            return FALSE;                                                      \
        }                                                                      \
        *out = q->data[q->first];                                              \
        q->first = (q->first + 1) % q->cap;                                    \
        q->size--;                                                             \
        return TRUE;                                                           \
    }

/*
    Dynamically Sized Queues
*/

#define DefDynQueueDecl(name, type)                                            \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        u32 first;                                                             \
        type* data;                                                            \
    } name;

#define DefDynQueueImpl(name, type)                                            \
    name name##Create(Allocator a) { return (name){.a = a}; }                  \
    void name##Destroy(name* q) {                                              \
        if (!q->data)                                                          \
            return;                                                            \
        Free(q->a, q->data, q->cap);                                           \
    }                                                                          \
    bool8 name##Empty(name* q) { return q->size == 0; }                        \
    bool8 name##Full(name* q) { return q->size >= q->cap; }                    \
    void name##Push(name* q, type item) {                                      \
        if (name##Full(q)) {                                                   \
            u32 oldcap = q->cap;                                               \
            q->cap = q->cap ? q->cap * 2 : 1;                                  \
                                                                               \
            type* new = Alloc(q->a, q->cap * sizeof(q->data[0]));              \
            if (!new) {                                                        \
                q->cap = oldcap;                                               \
                return;                                                        \
            }                                                                  \
                                                                               \
            for (u32 i = 0; i < q->size; i++) {                                \
                new[i] = q->data[(q->first + i) % oldcap];                     \
            }                                                                  \
                                                                               \
            Free(q->a, q->data, oldcap * sizeof(q->data[0]));                  \
            q->data = new;                                                     \
            q->first = 0;                                                      \
        }                                                                      \
                                                                               \
        u32 slot = (q->first + q->size) % q->cap;                              \
        q->data[slot] = item;                                                  \
        q->size++;                                                             \
    }                                                                          \
    bool8 name##Pop(name* q, type* out) {                                      \
        if (name##Empty(q)) {                                                  \
            return FALSE;                                                      \
        }                                                                      \
        *out = q->data[q->first];                                              \
        q->first = (q->first + 1) % q->cap;                                    \
        q->size--;                                                             \
        return TRUE;                                                           \
    }

/*
    Exponential Array - Pointer Stable Dynamically sized Container
*/

#define MAX_CHUNKS 32
#define MIN_CHUNK_SIZE 4

#define InitExar(mem, type)                                                    \
    (Exar) { .a = mem, .e_size = sizeof(type) }

typedef struct Exar {
    Allocator a;
    u32 size;
    u32 e_size;
    char* chunks[MAX_CHUNKS]; // variable size
} Exar;

void* ExarInsert(Exar* e, void* data);
void* ExarGet(Exar* e, u32 index);
void ExarClear(Exar* e);
void ExarFree(Exar* e);

#ifdef DS_IMPL

/*
    Exponential Array - Pointer Stable Dynamically sized Container
*/

void* ExarInsert(Exar* e, void* data) {
    u32 index = MIN_CHUNK_SIZE + e->size++;

    u32 ct = index >> MSB32(MIN_CHUNK_SIZE);
    u32 chunk = (ct ? MSB32(ct) + 1 : 0);
    u32 cindex = (~(1 << (chunk + 1)) & index);
    chunk--;

    if (!e->chunks[chunk]) {
        u32 chunk_size = MIN_CHUNK_SIZE << chunk;
        e->chunks[chunk] = Alloc(e->a, chunk_size * e->e_size);
    }

    // e->chunks[chunk][cindex] = val;

    char* dest = ((e->chunks[chunk]) + cindex * e->e_size);
    for (u32 i = 0; i < e->e_size; i++) { dest[i] = ((char*)data)[i]; }

    return (void*)dest;
}

void* ExarGet(Exar* e, u32 index) {
    if (index >= e->size)
        return NULL;
    index += MIN_CHUNK_SIZE;

    u32 ct = index >> MSB32(MIN_CHUNK_SIZE);
    u32 chunk = (ct ? MSB32(ct) + 1 : 0);
    u32 cindex = (~(1 << (chunk + 1)) & index);
    chunk--;

    if (!e->chunks[chunk])
        return NULL;
    char* dest = ((e->chunks[chunk]) + cindex * e->e_size);
    return dest;
}

void ExarClear(Exar* e) { e->size = 0; }

void ExarFree(Exar* e) {
    for (u32 i = 0; i < MAX_CHUNKS; i++) {
        u32 chunk_size = MIN_CHUNK_SIZE << i;
        if (e->chunks[i]) {
            Free(e->a, e->chunks[i], chunk_size * e->e_size);
            e->chunks[i] = NULL;
        } else
            break;
    }
    e->size = 0;
}

#endif
#endif

typedef struct TaskGraph TaskGraph;

struct Task;

#define task_func(name, task_arg) void(name)(struct Task * task_arg)

typedef struct Task {
    TaskGraph* g; // back ref
    SString name; // for debug

    bool8 visited;
    u32 blockers;

    dynArray(struct Task*) pre;  // things this task depends on
    dynArray(struct Task*) post; // things which depend on this task

    task_func(*func, t); // proc for io tasks

    dynArray(u32) cmds; // command which makes up this task
    dynArray(u32) args; // args for proc
} Task;

typedef struct TaskGraph {
    Exar tasks;
    dynArray(char) str_data;

    struct {
        dynArray(int) processes;
        dynArray(Task*) tasks;
    } running;
} TaskGraph;

typedef dynArray(i8) CBuf;

#define AutoRebuild(argc, argv, ...)                                           \
    _AutoRebuild(__FILE__, argc, argv,                                         \
                 ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddDummyTask(g, ...)                                                   \
    _AddTask(g, "", ARRAY_SIZE(((Task*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddTask(g, name, ...)                                                  \
    _AddTask(g, name, ARRAY_SIZE(((Task*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddTaskCond(cond, g, name, ...)                                        \
    (cond ? AddTask(g, name, ##__VA_ARGS__) : NULL)

#define AddCmd(t, ...)                                                         \
    _AddCmd(t, ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddArg(t, ...)                                                         \
    _AddArg(t, ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddProc(t, p) (t->func = p)

#define CompileCmd(g, file, c, ...)                                            \
    Compile(g, file, ARRAY_SIZE((((const char*[]){c}))), (const char*[]){c},   \
            ARRAY_SIZE(((Task*[]){__VA_ARGS__})), ##__VA_ARGS__)

TaskGraph CreateTaskGraph();
void ClearTaskGraph(TaskGraph* g);
void FreeTaskGraph(TaskGraph* g);
Task* _AddTask(TaskGraph* g, const char* name, u32 num_dep, ...);
void _AddCmd(Task* t, u32 num_args, ...);

void AddDep(Task* t, Task* d);
void PrintTaskTree(Task* t, bool8 v);

// returns TRUE if failed, FALSE on success
// Intended to be used like if (Exec(...)) { handle_error }
bool8 Exec(Task* t, u32 max_processes);

void BegCompile(TaskGraph* g, bool8 enable_cmds, SString compile_loc);
Task* EndCompile(TaskGraph* g, Task* t);

Task* Compile(TaskGraph* g, const char* filename, u32 cmd_size,
              const char** cmd, u32 num_arg, ...);

#ifdef SB_IMPL
#include <linux/limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <unistd.h>

#include <dirent.h>

#include <byteswap.h>

// use platform threads in general case
// required for stuff like printlog
#include <pthread.h>


/*
    Mutex
*/

#include <limits.h>
#include <stdatomic.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <linux/futex.h>


//simple mutex based around Lambport's Bakery Algorithm
//Atomics are used for checking whether to enter the mutex
//and futex is used to sleep on wait

typedef struct _Mutex {
    atomic_int ticket;
    u32 serving;
} _Mutex;

typedef struct _Mutex_lock {
    atomic_int lock;
} _Mutex_lock;

static inline int futex(u32* uaddr, int op, u32 val, const time_spec* timeout, u32* uaddr2, u32 val2) {
    return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val2);
}

// 0 -> unlocked
// 1 -> locked
// 2 -> waiters
void MutexLock(Mutex* mutex) {
    _Mutex_lock* m = (_Mutex_lock*)mutex;

    while (1) {
        int expected = 0;

        u32 count = 8;
        bool8 success = FALSE;
        while (count--) {
            expected = 0;
            success = atomic_compare_exchange_weak_explicit(&m->lock, &expected, 1, memory_order_acquire, memory_order_relaxed);
            if (success) {
                return;
            }
        }

        if (expected != 2) success = atomic_compare_exchange_weak_explicit(&m->lock, &expected, 2, memory_order_acquire, memory_order_relaxed);
        if (success) futex((u32*)&m->lock, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
    }
}

void MutexUnlock(Mutex* mutex) {
    _Mutex_lock* m = (_Mutex_lock*)mutex;
    //only wake if contention

    int c = atomic_exchange_explicit(&m->lock, 0, memory_order_release);
    if (c == 2) futex((u32*)&m->lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}


/*
    Endian Utilities
*/
u16 FlipEndian16(u16 i) { return bswap_16(i); }
u32 FlipEndian32(u32 i) { return bswap_32(i); }
u64 FlipEndian64(u64 i) { return bswap_64(i); }

f32 FlipEndianF(f32 f) { return bswap_32(f); }

f64 FlipEndianD(f64 f) { return bswap_64(f); }

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

// Stack Allocator
StackAllocator StackCreate(Allocator a, u64 size) {
    StackAllocator s = Alloc(a, sizeof(struct StackAllocator) + size);
    memset(s, 0, sizeof(struct StackAllocator) + size);
    s->cap = size;

    return s;
}

void StackDestroy(Allocator a, StackAllocator s) {
    Free(a, s, s->cap + sizeof(struct StackAllocator));
}

void* StackAlloc(StackAllocator s, u64 size) {
    if (s->size + size > s->cap)
        return NULL;
    void* out = &s->data[s->size];
    s->size += size;

    return out;
}

void StackSet(StackAllocator s, u64 pos) {
    if (pos > s->cap)
        return;
    s->size = pos;
}

void StackReset(StackAllocator s) { s->size = 0; }

// Dynamic Arena Allocator
ArenaAllocator ArenaCreate(Allocator a, u64 size) {
    ArenaAllocator arena = Alloc(a, size + sizeof(struct ArenaAllocator));

    // use prev pointer to store whether to dynamically grow
    *arena = (struct ArenaAllocator){
        .a = a,
        .cap = size,
        .size = 0,
    };

    return arena;
}

u64 ArenaGetPos(ArenaAllocator a) { return a->size; }

static alloc_func_def(arena_alloc) {
    if (!new) {
        // free
        // Arena's don't free, do nothing
        return NULL;
    }

    if (!ptr && !old) {
        // alloc
        return ArenaAlloc(ctx, new);
    }

    // realloc
    // Just alloc again because no free
    return ArenaAlloc(ctx, new);
}

Allocator ArenaGetAllocator(ArenaAllocator a) {
    return (Allocator){
        .a = arena_alloc,
        .ctx = a,
    };
}

void* ArenaAlloc(ArenaAllocator arena, u64 size) {
    ArenaAllocator a = arena;
    if (size + a->size > (a->cap)) {
        return NULL;
    }

    void* p = &a->data[a->size];
    a->size += size;
    return p;
}

u64 ArenaExtendLast(ArenaAllocator arena, u64 size) {
    ArenaAllocator a = arena;
    if (size + a->size > (a->cap)) {
        return 0;
    }
    a->size += size;
    return size;
}

void* ArenaAllocZero(ArenaAllocator a, u64 size) {
    void* p = ArenaAlloc(a, size);
    if (p)
        memset(p, 0, size);
    return p;
}

void ArenaPop(ArenaAllocator arena, u64 size) {
    ArenaAllocator a = arena;
    if (size > a->size) {
        a->size = 0;
    } else {
        a->size -= size;
    }
}

void ArenaSetPos(ArenaAllocator a, u64 pos) {
    u64 diff = a->size - pos;
    ArenaPop(a, diff);
}

void ArenaClear(ArenaAllocator a) {
    u64 size = a->cap;
    Allocator mem = a->a;
    a->size = 0;
}

void ArenaDestroy(ArenaAllocator s) {
    Free(s->a, s, s->cap + sizeof(struct ArenaAllocator));
}

ScratchArena ScratchArenaBegin(ArenaAllocator a) {
    return (ScratchArena){.arena = a, .base = ArenaGetPos(a)};
}

void ScratchArenaEnd(ScratchArena s) { ArenaSetPos(s.arena, s.base); }

// Should never live longer than a single
// function call.

static __thread ArenaAllocator a = 0;
static __thread ArenaAllocator b = 0;

static void FreeArenas() {
    ArenaDestroy(a);
    ArenaDestroy(b);
}

ScratchArena ScratchArenaGet(ArenaAllocator persist) {
    if (!a) {
        a = ArenaCreate(GlobalAllocator, MB(200L));
        b = ArenaCreate(GlobalAllocator, MB(200L));
        atexit(FreeArenas);
    }

    if (persist == a) {
        return ScratchArenaBegin(b);
    }
    return ScratchArenaBegin(a);
}

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

char* SStringGetCstr(SString s, ArenaAllocator a) {
    char* cstr = ArenaAlloc(a, s.len + 1);
    memcpy(cstr, s.data, s.len);
    cstr[s.len] = 0;
    return cstr;
}

/*
    File Implementations
    Platform Dependent
*/

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

    // INFO(ELI): We use a buffer here since SString is not garunteed to be
    // Null terminated. It usually is, but it may not be.
    char buf[PATH_MAX + 1] = {0};
    memcpy(buf, filename.data, filename.len);
    int fd = open(buf, oflags, 0666);

    struct stat statbuf;
    if (fd < 0 || fstat(fd, &statbuf)) {
        // error here
        // TODO(ELI): Once logging is working,
        // make log error and return invalid file
        debugerr("Failed to Open file: %s %d", filename, fd);

        return (file){.handle = -1};
    }

    return (file){
        .handle = fd,
        .stats =
            {
                .size = statbuf.st_size,
                .modified =
                    {
                        .s = statbuf.st_mtim.tv_sec,
                        .ns = statbuf.st_mtim.tv_nsec,
                    },
                .access =
                    {
                        .s = statbuf.st_atim.tv_sec,
                        .ns = statbuf.st_atim.tv_nsec,
                    },
            },
    };
}

file filestat(const SString filename) {
    ScratchArena sc = ScratchArenaGet(NULL);

    char* const data = ArenaAllocZero(sc.arena, filename.len + 1);
    memcpy(data, filename.data, filename.len);

    struct stat statbuf;
    if (stat(data, &statbuf)) {
        ScratchArenaEnd(sc);
        return (file){.handle = -1};
    }

    ScratchArenaEnd(sc);

    return (file){
        .handle = -1,
        .stats =
            {
                .size = statbuf.st_size,
                .modified =
                    {
                        .s = statbuf.st_mtim.tv_sec,
                        .ns = statbuf.st_mtim.tv_nsec,
                    },
                .access =
                    {
                        .s = statbuf.st_atim.tv_sec,
                        .ns = statbuf.st_atim.tv_nsec,
                    },
            },
    };
}

static u64 flushdata(u64 handle, SString src) {
    u64 size = src.len;
    char* buffer = (char*)src.data;
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
    char* buffer = (char*)dst.data;

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

u64 filewrite(file* dst, SString src) {
    u64 size = src.len;
    char* buffer = (char*)src.data;
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

void fileflush(file* file) {
    if (file->buffered && file->accum) {
        flushdata(file->handle,
                  (SString){.len = file->accum, .data = file->buf});
        file->accum = 0;
    }
}

u64 fileload(SString handle, const SString filename) {
    file f = fileopen(filename, FILE_READ);
    if (f.handle == -1)
        return 0;
    u64 size = fileread(handle, f);
    fileclose(f);
    return size;
}

u64 filesave(const SString filename, SString handle) {
    file f = fileopen(filename, FILE_WRITE | FILE_TRUNC | FILE_CREAT);
    if (f.handle == -1)
        return 0;
    u64 size = filewrite(&f, handle);
    fileclose(f);
    return size;
}

void filedelete(SString filename) {
    char buf[PATH_MAX + 1] = {0};
    sformat((SString){.len = PATH_MAX, .data = (i8*)buf}, "%s", filename);
    remove(buf);
}

void setdir(SString dir) {
    char buf[PATH_MAX + 1] = {0};
    sformat((SString){.len = PATH_MAX, .data = (i8*)buf}, "%s", dir);
    chdir(buf);
}

void setdirExe() {
    char tmp[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", tmp, PATH_MAX) < 0) {
        debugerr("Error: Failed to find Build Binary");
        exit(-1);
    }
    char* cursor = tmp;
    while (cursor[0]) cursor++;
    while (cursor > tmp && cursor[0] != '/') cursor--;
    cursor[0] = 0;

    chdir(tmp);
}



SString filepathpopdir(SString filepath) {
    SString out = filepath;
    while (out.len && out.data[out.len - 1] != '/') { out.len--; }
    if (out.len)
        out.len--;

    if (out.len <= 0)
        return filepath;

    return out;
}

SString filepathbasename(SString filepath) {
    SString out = filepath;

    while (out.len && out.data[out.len - 1] != '.') { out.len--; }

    if (!out.len)
        return filepath;

    out.data = filepath.data;
    out.len = out.len - 1;

    return out;
}

SString filepathext(SString filepath) {
    SString out = filepath;

    while (out.len && out.data[out.len - 1] != '.') { out.len--; }

    if (!out.len)
        return (SString){0};

    out.data = &filepath.data[out.len - 1];
    out.len = filepath.len - (out.len - 1);

    return out;
}

u32 filepathdepth(SString filepath) {
    u32 depth = 0;
    u32 i = 0;

    if (filepath.data[i] == '.' && filepath.data[i + 1] == '/') {
        i += 2;
    }

    for (; i < filepath.len; i++) {
        if (filepath.data[i] == '/')
            depth++;
    }

    return depth;
}

/*
    Custom Format
*/

typedef void (*wchar)(char c, void* ctx);

typedef struct formatInfo {
    wchar f;
    void* ctx;
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

#include <float.h>

// Based on the implementation of the Errol0
// algorithm provided by the original paper.
static void format_float(formatInfo info, f64 num) {
    if (num < 0) {
        info.f('-', info.ctx);
        num *= -1.0;
    }

    if (num >= FLT_MAX) {
        info.f('i', info.ctx);
        info.f('n', info.ctx);
        info.f('f', info.ctx);
        return;
    }

    if (num == 0.0) {
        info.f('0', info.ctx);
        info.f('.', info.ctx);
        info.f('0', info.ctx);
        return;
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

static ptrdiff_t format_arg(formatInfo info, va_list* args,
                            const char* format) {
    ptrdiff_t out = 0;
    switch (format[0]) {
    case 'd': {
        i64 s = va_arg((*args), i32);
        if (s < 0) {
            info.f('-', info.ctx);
            s *= -1;
        }
        u64 num = s;

        if (num == 0) {
            info.f('0', info.ctx);
            out++;
            break;
        }

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
    case 'u': {
        u64 num = va_arg((*args), u32);

        if (num == 0) {
            info.f('0', info.ctx);
            out++;
            break;
        }

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
    case 'l': {
        i64 s = va_arg((*args), i64);
        if (s < 0) {
            info.f('-', info.ctx);
            s *= -1;
        }
        u64 num = s;

        if (num == 0) {
            info.f('0', info.ctx);
            out++;
            break;
        }

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
        f64 s = va_arg((*args), f64);
        format_float(info, s);
        out++;
    } break;
    case 's': {
        SString s = va_arg((*args), SString);
        for (u32 i = 0; i < s.len; i++) { info.f(s.data[i], info.ctx); }
        out++;
    } break;
    case 'c': {
        int c = va_arg((*args), int);
        info.f(c, info.ctx);
        out++;
    } break;
    case 'n': {
        const char* s = va_arg((*args), const char*);
        while (s[0]) {
            info.f(s[0], info.ctx);
            s++;
        }
        out++;
    } break;
    case 'x': {
        u64 num = va_arg((*args), u64);

        if (num == 0) {
            info.f('0', info.ctx);
            out++;
            break;
        }

        // calc number of digits
        u32 digits = 0;
        u64 hexpower = 1;
        for (u32 i = 0; i < ARRAY_SIZE(power10plus); i++) {
            if ((num / hexpower) == 0UL)
                break;
            digits++;
            hexpower <<= 4; // multiply by 16
        }
        hexpower >>= 4;

        // print digits
        for (i32 i = digits - 1; i >= 0; i--) {
            u32 digit = num / hexpower;
            num -= digit * hexpower;
            if (digit < 10)
                info.f(digit + '0', info.ctx);
            else
                info.f(digit + 'a' - 10, info.ctx);

            hexpower >>= 4; // divide by 16
        }

        out++;
    } break;
    default: {
        todo();
    }
    }
    return out;
}

static void vformat(formatInfo info, va_list* args, const char* format) {

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

static void strput(char c, void* ctx) {
    strInfo* info = ctx;
    if (info->data.len > info->idx)
        info->data.data[info->idx++] = c;
}

u32 sformat(SString dst, const char* format, ...) {
    strInfo info = {.data = dst, .idx = 0};

    formatInfo finfo = {
        .ctx = &info,
        .f = strput,
    };

    va_list args;
    va_start(args, format);

    vformat(finfo, &args, format);

    va_end(args);

    return info.idx;
}

typedef struct fileInfo {
    file* dst;
    u64 idx;
} fileInfo;

static void fileput(char c, void* ctx) {
    fileInfo* info = ctx;
    info->idx++;
    filewrite(info->dst, (SString){.len = 1, .data = (i8*)&c});
}

u32 fformat(file* dst, const char* format, ...) {
    fileInfo info = {
        .dst = dst,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, &args, format);
    va_end(args);
    return info.idx;
}

file stdout_file = {
    .handle = STDOUT_FILENO,
    .buffered = 1,
    .closefd = 1,
};

file* logfile = &stdout_file;
file* warnfile = &stdout_file;
file* errfile = &stdout_file;

void flush_log(void) { fileflush(logfile); }

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
static Mutex m = {0};

u32 printlog(const char* format, ...) {
    if (logfile->thread_safe) MutexLock(&logfile->m);

    static bool8 schedule_flush = FALSE;
    if (!schedule_flush) {
        schedule_flush = TRUE;
        atexit(flush_log);
    }

    fileInfo info = {
        .dst = logfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, &args, format);
    va_end(args);

    if (logfile->thread_safe) MutexUnlock(&logfile->m);
    return info.idx;
}

u32 printwarn(const char* format, ...) {
    if (warnfile->thread_safe) MutexLock(&warnfile->m);

    fileInfo info = {
        .dst = warnfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, &args, format);
    va_end(args);

    if (warnfile->thread_safe) MutexUnlock(&warnfile->m);
    return info.idx;
}

u32 printerr(const char* format, ...) {
    if (errfile->thread_safe) MutexLock(&errfile->m);

    fileInfo info = {
        .dst = errfile,
    };

    formatInfo finfo = {
        .ctx = &info,
        .f = fileput,
    };

    va_list args;
    va_start(args, format);
    vformat(finfo, &args, format);
    va_end(args);

    if (errfile->thread_safe) MutexUnlock(&errfile->m);
    return info.idx;
}

// Directory Stuff

static DIR* dirpn = NULL;

void DirBegin(SString filepath) {
    char buf[PATH_MAX + 1] = {0};
    memcpy(buf, filepath.data, filepath.len);
    dirpn = opendir(buf);
}

SString DirNext(DirType* t) {
    if (!dirpn)
        return (SString){0};

    struct dirent* d = NULL;
    d = readdir(dirpn);

    *t = T_END;
    if (!d) {
        closedir(dirpn);
        return (SString){0};
    }

    if (d->d_type == DT_DIR)
        *t = T_DIR;
    else if (d->d_type == DT_REG)
        *t = T_FILE;
    else
        *t = T_UNKNOWN;

    return (SString){.data = (i8*)d->d_name, .len = strlen(d->d_name)};
}

static dynArray(dynArray(char)) paths = {.a = GlobalAllocator};
static dynArray(char) curr_path = {.a = GlobalAllocator};
static dynArray(char) out_path = {.a = GlobalAllocator};
static DIR* curr = NULL;
static u32 max_depth = 0;

void DirBeginRec(SString filepath, u32 depth) {
    ScratchArena sc = ScratchArenaGet(NULL);
    char* file = SStringGetCstr(filepath, sc.arena);

    curr = opendir(file);

    if (curr) {
        curr_path.size = 0;
        dynExt(curr_path, filepath.data, filepath.len);
        max_depth = depth;
    }

    ScratchArenaEnd(sc);
}

SString DirNextRec(DirType* t) {
    struct dirent* d = NULL;
    ScratchArena sc = ScratchArenaGet(NULL);

    while (curr || paths.size) {
        d = readdir(curr);

        if (!d) {
            closedir(curr);
            dynFree(curr_path);
            curr = NULL;
            if (!paths.size)
                break;

            curr_path.data = dynBack(paths).data;
            curr_path.size = dynBack(paths).size;
            curr_path.cap = dynBack(paths).cap;
            curr_path.a = dynBack(paths).a;
            paths.size--;

            char* pathname = SStringGetCstr(
                (SString){.data = (i8*)curr_path.data, .len = curr_path.size},
                sc.arena);
            curr = opendir(pathname);
            ScratchArenaEnd(sc);

            continue;
        }

        if (strcmp(d->d_name, ".") == 0)
            continue;
        if (strcmp(d->d_name, "..") == 0)
            continue;

        switch (d->d_type) {
        case DT_DIR:
            *t = T_DIR;
            dynArray(char) new = {.a = GlobalAllocator};
            if (curr_path.data) {
                dynExt(new, curr_path.data, curr_path.size);
                dynPush(new, '/');
            }
            u32 len = strlen(d->d_name);
            dynExt(new, d->d_name, len);
            if (max_depth <= filepathdepth((SString){.data = (i8*)new.data,
                                                     .len = new.size})) {
                dynFree(new);
                break;
            }
            dynResize(paths, paths.size + 1);
            dynBack(paths).data = new.data;
            dynBack(paths).size = new.size;
            dynBack(paths).cap = new.cap;
            dynBack(paths).a = new.a;
            break;
        case DT_REG:
            *t = T_FILE;
            break;
        default:
            *t = T_UNKNOWN;
            break;
        }
        out_path.size = 0;
        dynExt(out_path, curr_path.data, curr_path.size);
        dynPush(out_path, '/');
        u32 len = strlen(d->d_name);
        dynExt(out_path, d->d_name, len);

        return (SString){.data = (i8*)out_path.data, .len = out_path.size};
    }

    *t = T_END;
    ScratchArenaEnd(sc);
    return (SString){0};
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

u64 FNVHash64(u8* data, u64 size) {
    u64 hash = 14695981039346656037UL;
    for (u64 i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 1099511628211UL;
    }
    return hash;
}

u32 FNVHash32(u8* data, u32 size) {
    u32 hash = 2166136261UL;
    for (u32 i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 16777619UL;
    }
    return hash;
}

#include <stdarg.h>
#include <unistd.h>

void _AutoRebuild(const char* filename, int argc, char** argv, u32 num_flags,
                  ...) {

    char sbuf[PATH_MAX] = {0};
    char dbuf[PATH_MAX] = {0};

    const char* dst_name = realpath(argv[0], dbuf); 
    const char* src_name = realpath(filename, sbuf);
    assert(src_name);
    assert(dst_name);

    debuglog("src: %n", src_name);
    debuglog("dst: %n", dst_name);

    file f1 = filestat((SString){strlen(src_name), (i8*)src_name});
    file f2 = filestat((SString){strlen(dst_name), (i8*)dst_name});
    bool8 rebuild = timecmp(f1.stats.modified, >=, f2.stats.modified);

    if (!rebuild)
        return;

    TaskGraph g = CreateTaskGraph();

    va_list args;
    va_start(args, num_flags);

    Task* t = AddTask(&g, "recompile");
    AddCmd(t, "cc", src_name);
    for (u32 i = 0; i < num_flags; i++) {
        const char* arg = va_arg(args, const char*);
        AddCmd(t, arg);
    }
    AddCmd(t, "-o", argv[0]);

    va_end(args);

    if (Exec(t, 1)) {
        debugerr("Failed to recompile");
        panic();
    }

    printlog("Rebuild\n");
    fileflush(logfile);
    execvp(argv[0], argv);
    debugerr("Failed to Exec Rebuilt Build Script");
    panic();

    FreeTaskGraph(&g);
}

TaskGraph CreateTaskGraph() {
    return (TaskGraph){
        .tasks = InitExar(GlobalAllocator, Task),
        .str_data.a = GlobalAllocator,

        .running.tasks.a = GlobalAllocator,
        .running.processes.a = GlobalAllocator,
    };
}

void ClearTaskGraph(TaskGraph* g) {
    for (u32 i = 0; i < g->tasks.size; i++) {
        Task* e = ExarGet(&g->tasks, i);
        dynFree(e->cmds);
        dynFree(e->args);
        dynFree(e->pre);
        dynFree(e->post);
    }
    ExarClear(&g->tasks);
    g->str_data.size = 0;
    g->running.tasks.size = 0;
    g->running.processes.size = 0;
}

void FreeTaskGraph(TaskGraph* g) {

    for (u32 i = 0; i < g->tasks.size; i++) {
        Task* e = ExarGet(&g->tasks, i);
        dynFree(e->cmds);
        dynFree(e->args);
        dynFree(e->pre);
        dynFree(e->post);
    }

    ExarFree(&g->tasks);
    dynFree(g->str_data);
    dynFree(g->running.tasks);
    dynFree(g->running.processes);
}

static u32 AddStr(TaskGraph* t, SString str) {

    u32 idx = t->str_data.size;
    dynResize(t->str_data, t->str_data.size + str.len + 1);
    memcpy(&t->str_data.data[idx], str.data, str.len);
    t->str_data.data[t->str_data.size - 1] = 0;

    return idx;
}

void AddDep(Task* t, Task* d) {
    if (!d)
        return;
    if (d->cmds.size || d->func) {
        dynPush(t->pre, d);
        return;
    }
    // d must have no effect since d->func == NULL
    // and d->cmds.size == 0

    // close transitives from dummy tasks
    for (u32 i = 0; i < d->pre.size; i++) { AddDep(t, d->pre.data[i]); }
}

Task* _AddTask(TaskGraph* g, const char* name, u32 num_dep, ...) {
    Task v = {
        .g = g,
        .name = (SString){strlen(name), (i8*)name},
        .cmds.a = GlobalAllocator,
        .args.a = GlobalAllocator,
        .post.a = GlobalAllocator,
        .pre.a = GlobalAllocator,
    };
    Task* t = ExarInsert(&g->tasks, &v);

    va_list args;
    va_start(args, num_dep);

    for (u32 i = 0; i < num_dep; i++) {
        Task* d = va_arg(args, Task*);
        AddDep(t, d);
    }

    va_end(args);
    return t;
}

void _AddArg(Task* t, u32 num_args, ...) {
    if (!t)
        return;

    TaskGraph* g = t->g;

    va_list args;
    va_start(args, num_args);
    for (u32 i = 0; i < num_args; i++) {
        const char* arg = va_arg(args, const char*);
        u32 id = AddStr(g, (SString){strlen(arg), (i8*)arg});
        dynPush(t->args, id);
    }
    va_end(args);
}

void _AddCmd(Task* t, u32 num_args, ...) {
    if (!t)
        return;

    TaskGraph* g = t->g;

    va_list args;
    va_start(args, num_args);
    for (u32 i = 0; i < num_args; i++) {
        const char* arg = va_arg(args, const char*);
        u32 id = AddStr(g, (SString){strlen(arg), (i8*)arg});
        dynPush(t->cmds, id);
    }
    va_end(args);
}

static void PrintTask(Task* t, CBuf* b, bool8 last, bool8 v) {

    TaskGraph* g = t->g;
    printlog("%s", (SString){b->size, b->data});
    if (last) {
        printlog("\\--");
        dynExt((*b), "   ", 3);
    } else {
        printlog("+--");
        dynExt((*b), "|  ", 3);
    }

    if (v) {
        printlog("%s-", t->name);
        for (u32 i = 0; i < t->cmds.size; i++) {
            printlog(" %n", &g->str_data.data[t->cmds.data[i]]);
        }
        printlog("\n");
    } else {
        printlog("%s\n", t->name);
    }

    for (u32 i = 0; i < t->pre.size; i++) {
        PrintTask(t->pre.data[i], b, i == t->pre.size - 1, v);
    }
    b->size -= 3;
}

// Print tree structure for debugging
void PrintTaskTree(Task* t, bool8 v) {
    if (!t) {
        printlog("Null Task: (May have been Pruned by Condition)\n");
        return;
    }

    CBuf c = {GlobalAllocator};
    PrintTask(t, &c, TRUE, v);
    dynFree(c);
}

#include <stdio.h>

DefDynQueueDecl(TaskQueue, Task*);
DefDynQueueImpl(TaskQueue, Task*);

static void RunTask(Task* t, TaskQueue* q) {
    if (!t) {
        debugerr("Tried to Run NULL Task\n");
        return; // skip
    }

    TaskGraph* g = t->g;
    printlog("Running: %s\n", t->name);

    t->visited = FALSE;
    if (t->func) {
        // run func on main process
        // avoids ipc
        t->func(t);
    }

    if (!t->cmds.size) {
        // process exit if no cmd is to be run
        for (u32 i = 0; i < t->post.size; i++) {
            Task* n = t->post.data[i];
            if (n->blockers) n->blockers--;

            if (n->blockers == 0 && n->visited && (n->func || n->cmds.size)) {
                n->visited = FALSE;
                TaskQueuePush(q, n);
            }
        }
        return;
    }

    pid_t id = fork();

    if (id != 0) {
        if (id == -1) {
            printerr("Failed to Create Process: %n\n", strerror(errno));
            return;
        }
        // parent
        dynPush(g->running.processes, id);
        dynPush(g->running.tasks, t);
        return;
    }


    // child
    ScratchArena a = ScratchArenaGet(NULL);

    // alloc enough for all args + NULL ptr
    char** args = ArenaAllocZero(a.arena, sizeof(char*) * (t->cmds.size + 1));

    // This can be triggered if a blank task makes it into
    // the graph. This shouldn't ever happen since blanks tasks can't
    // be made dependents
    if (!t->cmds.size && !t->func) {
        fprintf(stderr, "Error, invalid task: %s\n", t->name.data);
        exit(1);
    }

    for (u32 i = 0; i < t->cmds.size; i++) {
        args[i] = &g->str_data.data[t->cmds.data[i]];
    }

    execvp(args[0], args);
    panic();
}

#include <sys/wait.h>

static void GatherTasks(ArenaAllocator a, Task* t, TaskQueue* queue) {

    // mark visited
    if (!t->visited) {
        t->visited = TRUE;
        t->blockers = t->pre.size;

        t->post.size = 0;

        if (t->pre.size == 0) {
            TaskQueuePush(queue, t);
        }

        // populate dependents
        for (u32 i = 0; i < t->pre.size; i++) {
            Task* dep = t->pre.data[i];
            GatherTasks(
                    a, dep,
                    queue); // important Gather Tasks initializes and clears data
                            // So it must run before we start writing to dep's post
            dynPush(dep->post, t);
        }
    }
}

bool8 Exec(Task* t, u32 max_processes) {
    assert(max_processes > 0);
    TaskGraph* g = t->g;
    ScratchArena sc = ScratchArenaGet(NULL);

    TaskQueue queue = TaskQueueCreate(ArenaGetAllocator(sc.arena));
    dynReserve(g->running.tasks, max_processes);
    dynReserve(g->running.processes, max_processes);

    GatherTasks(sc.arena, t, &queue);

    bool8 failed = FALSE;

    // while tasks are in process or queued, continue
    while (!TaskQueueEmpty(&queue) || g->running.processes.size) {
        if (g->running.processes.size >= max_processes ||
            TaskQueueEmpty(&queue)) {
            // wait
            int res = 0;
            pid_t id = wait(&res);

            if (id == -1) {
                printerr("Wait failed\n");
                panic();
            }

            u32 match = -1;
            for (u32 i = 0; i < g->running.processes.size; i++) {
                if (id == g->running.processes.data[i]) {
                    match = i;
                    break;
                }
            }

            if (!WIFEXITED(res) || WEXITSTATUS(res) != 0 || WIFSIGNALED(res) || match == -1) {
                printerr("\e[38;2;255;0;0m[ERR]: Task %s Failed\e[0m\n", g->running.tasks.data[match]->name);
                printerr( "\e[38;2;255;255;0m[WARN]: Cancelling Task Exec: %s\e[0m\n", t->name);
                dynDel(g->running.processes, match);
                dynDel(g->running.tasks, match);
                failed = TRUE;
                break;
            }

            Task* fin = g->running.tasks.data[match];

            for (u32 i = 0; i < fin->post.size; i++) {
                Task* n = fin->post.data[i];
                if (n->blockers) n->blockers--;

                if (n->blockers == 0 && n->visited && (n->func || n->cmds.size)) {
                    n->visited = FALSE;
                    TaskQueuePush(&queue, n);
                }
            }

            dynDel(g->running.processes, match);
            dynDel(g->running.tasks, match);
        }

        // grab task if possible
        Task* curr = NULL;
        if (TaskQueuePop(&queue, &curr)) {
            // run task
            RunTask(curr, &queue);
        }
    }

    while (g->running.processes.size) {
        // wait
        int res;
        pid_t id = wait(&res);
        if (id == -1) {
            printerr("Wait failed\n");
            panic();
        }

        u32 match = -1;
        for (u32 i = 0; i < g->running.processes.size; i++) {
            if (id == g->running.processes.data[i]) {
                match = i;
                break;
            }
        }

        if (!WIFEXITED(res) || WEXITSTATUS(res) != 0 || match == -1) {
            printerr("\e[38;2;255;0;0m[ERR]: Task %s Failed\e[0m\n",
                     g->running.tasks.data[match]->name);
            printerr(
                "\e[38;2;255;255;0m[WARN]: Cancelling Task Exec: %s\e[0m\n",
                t->name);
            dynDel(g->running.processes, match);
            dynDel(g->running.tasks, match);
            failed = TRUE;
            break;
        }

        dynDel(g->running.processes, match);
        dynDel(g->running.tasks, match);
    }

    ScratchArenaEnd(sc);
    return failed;
}

// DO NOT TOUCH
// Internal Implementation data, should not be touched
// unless you know what you are doing
static struct {
    bool8 written;
    bool8 enabled;
    SString compile_file;
    file compile_cmds;
    Task* first;
} _c_intern;

task_func(beg_compile, t) {
    if (!_c_intern.enabled)
        return;
    _c_intern.compile_cmds =
        fileopen(_c_intern.compile_file, FILE_WRITE | FILE_CREAT | FILE_TRUNC);
    filewrite(&_c_intern.compile_cmds, sstring("[\n"));
}

task_func(add_entry, t) {
    if (!_c_intern.enabled)
        return;
    ScratchArena sc = ScratchArenaGet(NULL);
    TaskGraph* g = t->g;

    char* buffer = ArenaAlloc(sc.arena, PATH_MAX + 1);
    getcwd(buffer, PATH_MAX);

    SString compile_entry = sstring("{\n"
                                    "\t\"directory\": \"%n\",\n"
                                    "\t\"file\": \"%n\",\n" // artifact
                                    "\t\"arguments\": [ ");

    if (_c_intern.written) {
        fformat(&_c_intern.compile_cmds, ",\n");
    }
    _c_intern.written = TRUE;

    fformat(&_c_intern.compile_cmds, (char*)compile_entry.data, buffer,
            &g->str_data.data[t->args.data[0]]);

    for (u32 i = 1; i < t->args.size; i++) {
        if (i > 1)
            fformat(&_c_intern.compile_cmds, ",");
        fformat(&_c_intern.compile_cmds, "\"%n\" ",
                &g->str_data.data[t->args.data[i]]);
    }
    fformat(&_c_intern.compile_cmds, "]\n"
                                     "}");

    ScratchArenaEnd(sc);
}

task_func(end_compile, t) {
    if (!_c_intern.enabled)
        return;
    _c_intern.written = FALSE;
    filewrite(&_c_intern.compile_cmds, sstring("\n]\n"));
    fileclose(_c_intern.compile_cmds);
}

void BegCompile(TaskGraph* g, bool8 enable_cmds, SString compile_loc) {
    _c_intern.enabled = enable_cmds;
    _c_intern.compile_file = compile_loc;

    Task* start = AddTask(g, "init_compile");
    AddProc(start, beg_compile);
    _c_intern.first = start;
}

/**
 * filename is the name of the final build artifact
 */
Task* Compile(TaskGraph* g, const char* filename, u32 cmd_size,
              const char** cmd, u32 num_arg, ...) {
    // compilation
    Task* compile = AddTask(g, filename, _c_intern.first);
    AddCmd(compile, "cc", "-o", filename);
    AddProc(compile, add_entry);
    AddArg(compile, filename);

    va_list args;
    va_start(args, num_arg);
    for (u32 i = 0; i < num_arg; i++) {
        Task* arg = va_arg(args, Task*);
        AddDep(compile, arg);
    }
    va_end(args);

    for (u32 i = 0; i < cmd_size; i++) {
        AddCmd(compile, cmd[i]);
        AddArg(compile, cmd[i]);
    }

    return compile;
}

Task* EndCompile(TaskGraph* g, Task* t) {
    if (!_c_intern.enabled)
        return t; // skips connecting to graph
    Task* end = AddTask(g, "end compile", _c_intern.first);
    AddProc(end, end_compile);

    AddDep(end, t);
    return end;
}

#endif
#endif
