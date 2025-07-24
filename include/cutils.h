#ifndef CUTILS_H
#define CUTILS_H

#include <stdint.h>

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

#define alloc_func_def(name) \
    void* (*name)(void* ctx, void* ptr, u64 oldsize, u64 newsize)

typedef alloc_func_def(alloc_func);

typedef struct Allocator {
    alloc_func a; 
    void* ctx;
} Allocator;

/*
    Custom Format
*/






#endif
