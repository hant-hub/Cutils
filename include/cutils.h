#ifndef CUTILS_H
#define CUTILS_H

#include <stdint.h>

#define PAGE_SIZE (1<<12)



/*
    Helpful typedefs
*/

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;



/*
    Custom Memory Allocators
*/

#define NULL ((void*)0)

#define alloc_func_def(name) \
    void* (*name)(void* ctx, void* ptr, u64 old, u64 new)

typedef alloc_func_def(alloc_func);

typedef struct Allocator {
    alloc_func a; 
    void* ctx;
} Allocator;

#define alloc(mem, size) \
    mem.a(mem.ctx, NULL, 0, size)

#define free(mem, ptr, size) \
    mem.a(mem.ctx, ptr, size, 0)

#define realloc(mem, ptr, old, new) \
    mem.a(mem.ctx, ptr, old, new)




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

u32 Sstrcmp();
SString Sstrdup();
SString Sstrtok();


/*
    FILE Handling
*/

//File descriptors and void* fit in FILE

typedef struct file_info {
    u32 size;
    //In future stuff like time, last modified, etc.
} file_info;

typedef struct file {
    u64 handle; //on windows this is a void*, on linux an int
    file_info stats;
    i8 buf[PAGE_SIZE];
} file;

file fileopen(const SString filename);
void fileclose(file file);

u64 fileread(SString dst, file src);
u64 filewrite(file dst, SString src);

u64 fileload(SString* handle, const SString filename);
u64 filesave(const SString filename, SString* handle);

/*
    Custom Format
*/

u32 sformat();
u32 fformat();


/*
    Custom Logging
*/

u32 printlog();
u32 printwarn();
u32 printerr();


#endif
