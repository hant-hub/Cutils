
#ifndef NAME
#error Must set map name
#define NAME swar
#endif

#ifndef KEY
#error Must set key for map
#define KEY u32
#endif

#ifndef VAL
#error Must set val for map
#define VAL u32
#endif

#ifndef HASH
#define HASH(a) FNVHash64((u8*)&a, sizeof(a))
#endif

#ifndef EQ
#define EQ(a, b) (a == b)
#endif

#ifndef USR_DATA
#define USR_DATA
#endif

#ifndef ON_INS
#define ON_INS(m, k, v) do { NAME* nm = m; KEY ks = k; VAL vs = v; (void) nm; (void) ks; (void) vs;} while(0)
#endif

#ifndef ON_DEL
#define ON_DEL(m, k, v) do { NAME* nm = m; KEY ks = k; VAL vs = v; (void) nm; (void) ks; (void) vs;} while(0)
#endif

#ifndef ON_CLEAR
#define ON_CLEAR(m, k, v) do { NAME* nm = m; KEY ks = k; VAL vs = v; (void) nm; (void) ks; (void) vs;} while(0)
#endif

#include <core/cutils.h>
#include <string.h>

#define ENTRY CAT(NAME, _entry)
typedef struct ENTRY {
    KEY key;
    VAL val;
} ENTRY;

#include "fixed.h"

typedef struct NAME {
    Allocator a;

    u8* meta;
    ENTRY* entries;

    u32 load;
    u32 cap;

    USR_DATA;
} NAME;


#define MIN_CAP 16
#define GROUP_SIZE 8


#define INS_UNIQ CAT(NAME, _ins_uniq)
static void INS_UNIQ(NAME* m, KEY key, VAL val) {
    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/8));
    const u64 mask = spread * META_EMPTY;


    for (u32 i = 0; i < m->cap; i++) {
        u64 meta = ((u64*)m->meta)[probe];
        u64 res = getMask(meta, mask);

        //must probe next
        while (res) {
            u32 id = (__builtin_ctzll(res)/8);
            if (m->meta[probe * 8 + id] == META_EMPTY) {
                m->meta[probe * 8 + id] = h2;
                m->entries[probe * 8 + id].key = key;
                m->entries[probe * 8 + id].val = val;

                m->load++;
                return;
            }

            res &= res - 1;
        }

#ifdef TRI_PROBE
        probe = (probe + step) % (m->cap/8);
        step++;
#else
        probe = (probe + 1) % (m->cap/8);
#endif
    }
    panic();
}

#define RESIZE CAT(NAME, _resize)
void RESIZE(NAME* m, u32 size) {
    u32 s = 8;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    ENTRY* oldvals = m->entries; 

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, (m->cap/8) * sizeof(u64));
    m->entries = Alloc(m->a, m->cap * sizeof(m->entries[0]));
    memset(m->meta, META_EMPTY, m->cap);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] == META_EMPTY) continue;
        if (oldmeta[i] == META_DEL) continue;
        INS_UNIQ(m, oldvals[i].key, oldvals[i].val);
    }

    Free(m->a, oldmeta, oldcap);
    Free(m->a, oldvals, oldcap * sizeof(m->entries[0]));

}


void CAT(NAME, _ins)(NAME* m, KEY key, VAL val) {
    if (m->load * 8 >= m->cap * 7) {
        RESIZE(m, MIN_CAP);
    }

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/8));

    const u64 mask = spread * h2;
    const u64 emask = spread * META_EMPTY;
    const u64 dmask = spread * META_DEL;

    for (u32 i = 0; i < m->cap; i++) {
        u64 meta = ((u64*)m->meta)[probe];
        u64 res = getMask(meta, mask);

        //must probe next
        while (res) {
            u32 id = (__builtin_ctzll(res)/8);
            if (m->meta[probe * 8 + id] == h2 &&
                EQ(m->entries[probe * 8 + id].key, key)) {
                m->entries[probe * 8 + id].val = val;
                return;
            }

            res &= res - 1;
        }

        res = getMask(meta, emask);
        if (res) {
            res |= getMask(meta, dmask);

            while (res) {
                u32 id = (__builtin_ctzll(res)/8);
                if (m->meta[probe * 8 + id] == META_EMPTY ||
                    m->meta[probe * 8 + id] == META_DEL) {

                    ON_INS(m, key, val);

                    m->meta[probe * 8 + id] = h2;
                    m->entries[probe * 8 + id].key = key;
                    m->entries[probe * 8 + id].val = val;

                    m->load++;
                    return;
                }

                res &= res - 1;
            }
        }

#ifdef TRI_PROBE
        probe = (probe + step) % (m->cap/8);
        step++;
#else
        probe = (probe + 1) % (m->cap/8);
#endif
    }
    panic();

}


VAL* CAT(NAME, _get)(NAME* m, KEY key) {
    if (!m->cap) return NULL;

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/8));

    const u64 mask = spread * h2;
    const u64 emask = spread * META_EMPTY;

    for (u32 i = 0; i < m->cap; i++) {
        u64 meta = ((u64*)m->meta)[probe];
        u64 res = getMask(meta, mask);

        //must probe next
        while (res) {
            u32 id = (__builtin_ctzll(res)/8);
            if (m->meta[probe * 8 + id] == h2 && EQ(m->entries[probe * 8 + id].key, key)) {
                return &m->entries[probe * 8 + id].val;
            }

            res &= res - 1;
        }

        res = getMask(meta, emask);
        if (res) {
            return NULL;
        }

#ifdef TRI_PROBE
        probe = (probe + step) % (m->cap/8);
        step++;
#else
        probe = (probe + 1) % (m->cap/8);
#endif
    }
    panic();
}

void CAT(NAME, _del)(NAME* m, KEY key) {
    if (!m->cap) return; 

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/8));

    const u64 mask = spread * h2;
    const u64 emask = spread * META_EMPTY;

    for (u32 i = 0; i < m->cap; i++) {
        u64 meta = ((u64*)m->meta)[probe];
        u64 res = getMask(meta, mask);

        //must probe next
        while (res) {
            u32 id = (__builtin_ctzll(res)/8);
            if (m->meta[probe * 8 + id] == h2 && EQ(m->entries[probe * 8 + id].key, key)) {
                m->meta[probe * 8 + id] = META_DEL;
                ON_DEL(m, m->entries[probe * 8 + id].key, m->entries[probe * 8 + id].val);
                return;
            }

            res &= res - 1;
        }

        res = getMask(meta, emask);
        if (res) {
            return;
        }

#ifdef TRI_PROBE
        probe = (probe + step) % (m->cap/8);
        step++;
#else
        probe = (probe + 1) % (m->cap/8);
#endif
    }
    panic();
}

void CAT(NAME, _clear)(NAME* m) {
    m->load = 0;

    for (u32 i = 0; i < m->cap; i++) {
        if (m->meta[i] != META_EMPTY &&
            m->meta[i] != META_DEL) {
            ON_CLEAR(m, m->entries[i].key, m->entries[i].val);
        }
        m->meta[i] = META_EMPTY;
    }
}

void CAT(NAME, _free)(NAME* m) {
    for (u32 i = 0; i < m->cap; i++) {
        if (m->meta[i] != META_EMPTY &&
            m->meta[i] != META_DEL) {
            ON_CLEAR(m, m->entries[i].key, m->entries[i].val);
        }
    }


    Free(m->a, m->meta, m->cap);
    Free(m->a, m->entries, m->cap * sizeof(m->entries[0]));

    m->meta = NULL;
    m->entries = NULL;

    m->load = 0;
    m->cap = 0;
}

#undef GROUP_SIZE
#undef NAME
#undef KEY
#undef VAL
#undef MIN_SIZE
#undef HASH
#undef EQ
#undef ENTRY_NAME
#undef INS
#undef INS_UNIQ
#undef RESIZE
#undef ON_INS
#undef ON_DEL
#undef USR_DATA
