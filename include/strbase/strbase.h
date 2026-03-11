#ifndef STRBASE_H
#define STRBASE_H

#include <core/cutils.h>

/*
    INFO: This requires Cutils to be implemented
    and linked. StrBase.c will assume that somewhere there
    is a Cutils implementation.
*/

#define STRBASE_INAVLID_STR -1

#ifndef STRBASE_LOAD_MAX
#define STRBASE_LOAD_MAX 0.75
#endif

#ifndef STRBASE_MIN_SIZE
#define STRBASE_MIN_SIZE 4
#endif

typedef u32 StrID; // direct index into strstore

typedef struct StrBase {
    Allocator mem; // Assume Dynamic Memory

    // hashmap for deduplication
    u32 *stridx;
    i32 *meta; // meta data for robin hood hashing

    u32 hashsize;
    u32 hashcap;

    // Stable Storage (index stability)
    SString *strstore;

    u32 strsize;
    u32 maxslots;
} StrBase;

#define GetStr(base, id) ((base)->strstore[id])

StrID StrBaseAdd(StrBase *base, SString s);
SString StrBaseGet(StrBase *base, StrID s);
void StrBaseFree(StrBase *base);

#endif
