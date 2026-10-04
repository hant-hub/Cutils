#include <strbase/strbase.h>
#include <string.h>


// hashmap
static void HashResize(StrBase* base) {
    if (base->hashsize < base->hashcap * STRBASE_LOAD_MAX)
        return;

    u32 oldsize = base->hashcap;
    while (base->hashsize >= base->hashcap * STRBASE_LOAD_MAX) {
        base->hashcap = base->hashcap ? base->hashcap * 2 : STRBASE_MIN_SIZE;
    }

    u32* oldkeys = base->stridx;
    i32* oldmeta = base->meta;

    base->stridx = Alloc(base->mem, base->hashcap * sizeof(u32));
    base->meta = Alloc(base->mem, base->hashcap * sizeof(u32));

    memset(base->meta, -1, base->hashcap * sizeof(u32));

    for (u32 i = 0; i < oldsize; i++) {
        if (oldmeta[i] == -1)
            continue;

        // hash insert
        u32 key = oldkeys[i];
        u32 counter = 0;

        SString s = base->strs.data[key];
        u32 idx = FNVHash32((u8*)s.data, s.len) % base->hashcap;

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

static u32 AddStr(StrBase* b, SString s) {
    b->strmem.a = b->mem;
    b->strs.a = b->mem;

    SString out = {
        .data = DynArenaAlloc(&b->strmem, s.len + 1),
        .len = s.len
    };
    memcpy(out.data, s.data, s.len);
    out.data[s.len] = 0; //null terminator
    
    u32 slot = b->strs.size;
    dynPush(b->strs, out);
    return slot;
}


// Will copy string into internally managed table
// free string memory afterward
StrID StrBaseAdd(StrBase* base, SString s) {
    HashResize(base);

    u32 idx = FNVHash32((u8*)s.data, s.len) % base->hashcap;
    u32 counter = 0;

    u32 out = STRBASE_INAVLID_STR;
    base->hashsize++;

    for (u32 i = 0; i < base->hashcap; i++) {
        if (base->meta[idx] == STRBASE_INAVLID_STR) {
            // empty
            //u32 slot = AllocSlot(base);
            u32 slot = AddStr(base, s);

            base->meta[idx] = counter;
            base->stridx[idx] = slot;

            //base->strstore[slot] = Sstrdup(base->mem, s);

            return slot;
        }

        if (base->meta[idx] < counter) {
            // steal
            break;
        }

        if (Sstrcmp(s, base->strs.data[base->stridx[idx]])) {
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

        //u32 slot = AllocSlot(base);
        u32 slot = AddStr(base, s);

        base->meta[idx] = counter;
        base->stridx[idx] = slot;

        //base->strstore[slot] = Sstrdup(base->mem, s);

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
SString StrBaseGet(StrBase* base, StrID s) { return (SString){0}; }

void StrBaseFree(StrBase* base) {
    dynFree(base->strs);
    DynArenaDestroy(base->strmem);

    Free(base->mem, base->stridx, base->hashcap * sizeof(u32));
    Free(base->mem, base->meta, base->hashcap * sizeof(u32));
}
