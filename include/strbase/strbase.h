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

#ifdef STRBASE_IMPL

// hashmap
static void HashResize(StrBase *base) {
    if (base->hashsize < base->hashcap * STRBASE_LOAD_MAX)
        return;

    u32 oldsize = base->hashcap;
    while (base->hashsize >= base->hashcap * STRBASE_LOAD_MAX) {
        base->hashcap = base->hashcap ? base->hashcap * 2 : STRBASE_MIN_SIZE;
    }

    u32 *oldkeys = base->stridx;
    i32 *oldmeta = base->meta;

    base->stridx = Alloc(base->mem, base->hashcap * sizeof(u32));
    base->meta = Alloc(base->mem, base->hashcap * sizeof(u32));

    memset(base->meta, -1, base->hashcap * sizeof(u32));

    for (u32 i = 0; i < oldsize; i++) {
        if (oldmeta[i] == -1)
            continue;

        // hash insert
        u32 key = oldkeys[i];
        u32 counter = 0;

        SString s = base->strstore[key];
        u32 idx = FNVHash32((u8 *)s.data, s.len) % base->hashcap;

        for (u32 i = 0; i < base->hashcap; i++) {
            if (base->meta[idx] == STRBASE_INAVLID_STR) {
                // empty
                base->meta[idx] = counter;
                base->stridx[idx] = key;
                break;
            }

            if (base->meta[idx] < counter) {
                // steal
                u32 tmpcounter = base->meta[idx];
                u32 tmpslot = base->stridx[idx];

                base->meta[idx] = counter;
                base->stridx[idx] = key;

                counter = tmpcounter;
                key = tmpslot;
            }

            idx = (idx + 1) % base->hashcap;
            counter++;
        }
    }

    Free(base->mem, oldkeys, oldsize * sizeof(u32));
    Free(base->mem, oldmeta, oldsize * sizeof(u32));
}

// dyn array

static u32 AllocSlot(StrBase *base) {
    if (base->strsize >= base->maxslots) {
        u32 oldsize = base->maxslots;
        base->maxslots = base->maxslots ? base->maxslots * 2 : STRBASE_MIN_SIZE;

        base->strstore = Realloc(base->mem, base->strstore, oldsize * sizeof(SString),
                                 base->maxslots * sizeof(SString));

        memset(&base->strstore[oldsize], 0, (base->maxslots - oldsize) * sizeof(SString));
    }

    return base->strsize++;
}


// Will copy string into internally managed table
// free string memory afterward
StrID StrBaseAdd(StrBase *base, SString s) {
    HashResize(base);

    u32 idx = FNVHash32((u8 *)s.data, s.len) % base->hashcap;
    u32 counter = 0;

    u32 out = STRBASE_INAVLID_STR;
    base->hashsize++;

    for (u32 i = 0; i < base->hashcap; i++) {
        if (base->meta[idx] == STRBASE_INAVLID_STR) {
            // empty
            u32 slot = AllocSlot(base);

            base->meta[idx] = counter;
            base->stridx[idx] = slot;

            base->strstore[slot] = Sstrdup(base->mem, s);

            return slot;
        }

        if (base->meta[idx] < counter) {
            // steal
            break;
        }

        if (Sstrcmp(s, base->strstore[base->stridx[idx]])) {
            // duplicate
            base->hashsize--;
            return base->stridx[idx];
        }

        idx = (idx + 1) % base->hashcap;
        counter++;
    }

    // robin hood
    u32 key = base->stridx[idx];
    {
        u32 tmpcounter = base->meta[idx];

        u32 slot = AllocSlot(base);

        base->meta[idx] = counter;
        base->stridx[idx] = slot;

        base->strstore[slot] = Sstrdup(base->mem, s);

        counter = tmpcounter;
        out = slot;
    }

    for (u32 i = 0; i < base->hashcap; i++) {
        if (base->meta[idx] == STRBASE_INAVLID_STR) {
            // empty
            base->meta[idx] = counter;
            base->stridx[idx] = key;

            return out;
        }

        if (base->meta[idx] < counter) {
            // steal
            u32 tmpcounter = base->meta[idx];
            u32 tmpslot = base->stridx[idx];

            base->meta[idx] = counter;
            base->stridx[idx] = key;

            counter = tmpcounter;
            key = tmpslot;
        }

        idx = (idx + 1) % base->hashcap;
        counter++;
    }

    return STRBASE_INAVLID_STR;
}

// Returns Zero on miss (this should never happen)
SString StrBaseGet(StrBase *base, StrID s) { return (SString){0}; }


void StrBaseFree(StrBase *base) {
    for (u32 i = 0; i < base->maxslots; i++) {
        Free(base->mem, base->strstore[i].data, base->strstore[i].len);
    }

    Free(base->mem, base->strstore, base->maxslots * sizeof(SString));

    Free(base->mem, base->stridx, base->hashcap * sizeof(u32));
    Free(base->mem, base->meta, base->hashcap * sizeof(u32));
}

#endif
#endif
