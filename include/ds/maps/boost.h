
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

#define ENTRY_NAME CAT(NAME, _entry)
typedef struct ENTRY_NAME {
    KEY key;
    VAL val;
} ENTRY_NAME;

#include "fixed.h"

#define GROUP_SIZE 7
#define MIN_CAP (7*1)

#define GROUP_NAME CAT(NAME, _group)
typedef union GROUP_NAME {
    struct {
        u8 overflow;
        u8 meta[GROUP_SIZE];
    };
    u8 slots[GROUP_SIZE + 1];
    u64 align;
} GROUP_NAME;

typedef struct NAME {
    Allocator a;

    GROUP_NAME* meta;
    ENTRY_NAME* entries;

    u32 load;
    u32 cap;

    USR_DATA;
} NAME;

#define INS_UNIQ CAT(NAME, _ins_uniq)
static inline void INS_UNIQ(NAME* m, KEY key, VAL val) {

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);
    u8 h3 = getH3(hash);

    const u64 emask = META_EMPTY * spread;

    u32 probe = h1 % (m->cap/GROUP_SIZE);
    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        GROUP_NAME* g = &m->meta[probe];
        GROUP_NAME g1 = m->meta[probe];
        g1.overflow = -1;
        u64 meta = g1.align;
        
        //find matches
        u64 res = getMask(meta, emask);

        //iterate
        while (res) {
            u32 id = __builtin_ctzll(res)/8 - 1;
            if (g->meta[id] == META_EMPTY) {
                //insert
                m->load++;
                g->meta[id] = h2;
                m->entries[probe * GROUP_SIZE + id] = (ENTRY_NAME){
                    .key = key,
                    .val = val
                };
                return;
            }
            res &= res - 1;
        }

        //set overflow
        g->overflow |= 1 << h3;

        //linear probe
        probe = (probe + 1) % (m->cap/GROUP_SIZE);
    }
    panic();
}

#define RESIZE CAT(NAME, _resize)
void RESIZE(NAME* m, u32 size) {
    u32 s = GROUP_SIZE; //grow in groups of 7
    while (m->load * 8 >= s * 7 || s <= size) s <<= 1; //powers of 2

    u32 oldcap = m->cap;
    GROUP_NAME* oldmeta = m->meta;
    ENTRY_NAME* oldvals = m->entries;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, sizeof(m->meta[0]) * (m->cap/GROUP_SIZE));
    m->entries = Alloc(m->a, sizeof(m->entries[0]) * m->cap);

    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        m->meta[i].overflow = 0;
        memset(m->meta[i].meta, META_EMPTY, GROUP_SIZE);
    }

    for (u32 i = 0; i < oldcap/GROUP_SIZE; i++) {
        for (u32 j = 0; j < GROUP_SIZE; j++) {
            if (oldmeta[i].meta[j] == META_EMPTY) continue;
            u32 id = i * GROUP_SIZE + j;
            INS_UNIQ(m, oldvals[id].key, oldvals[id].val);
        }
    }

    Free(m->a, oldmeta, sizeof(oldmeta[0]) * (oldcap/GROUP_SIZE));
    Free(m->a, oldvals, sizeof(oldvals[0]) * oldcap);
}


void CAT(NAME, _ins)(NAME* m, KEY key, VAL val) {
    if (m->load * 8 >= m->cap * 7) {
        RESIZE(m, MIN_CAP);
    }

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);
    u8 h3 = getH3(hash);


    const u64 emask = META_EMPTY * spread;
    const u64 hmask = h2 * spread;

    u32 p = -1;
    u32 s = -1;

    u32 probe = h1 % (m->cap/GROUP_SIZE);
    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        GROUP_NAME* g = &m->meta[probe];
        GROUP_NAME g1 = m->meta[probe];
        g1.overflow = META_SENTINEL;

        u64 res = getMask(g1.align, hmask);
        while (res) {
            u32 id = __builtin_ctzll(res)/8 - 1;
            ENTRY_NAME* e = &m->entries[probe * GROUP_SIZE + id];
            if (g->meta[id] == h2 && EQ(e->key, key)) {
                e->val = val;
                return;
            }
            res &= res - 1;
        }

        res = getMask(g1.align, emask);
        if (res) {
            if (p == -1) {
                while (res) {
                    u32 id = __builtin_ctzll(res)/8 - 1;
                    if (id < 7 && g->meta[id] == META_EMPTY) {
                        p = probe;
                        s = id;
                        break;
                    }
                    res &= res - 1;
                }
            }

            if ((p != -1) && !(g->overflow & (1 << h3))) {
                break;
            }
        }

        g->overflow |= 1 << h3;
        probe = (probe + 1) % (m->cap / GROUP_SIZE);
    }

    if (p != -1) {
        ON_INS(m, key, val);

        m->meta[p].meta[s] = h2;
        m->load++;

        m->entries[p * GROUP_SIZE + s].key = key;
        m->entries[p * GROUP_SIZE + s].val = val;
        return;
    }

    panic();
}

VAL* CAT(NAME, _get)(NAME* m, KEY key) {
    if (!m->cap || !m->load) return NULL;
    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);
    u8 h3 = getH3(hash);

    const u64 emask = META_EMPTY * spread;
    const u64 hmask = h2 * spread;

    u32 probe = h1 % (m->cap/GROUP_SIZE);
    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        GROUP_NAME* g = &m->meta[probe];
        GROUP_NAME g1 = m->meta[probe];
        g1.overflow = META_SENTINEL;

        u64 res = getMask(g1.align, hmask);

        while (res) {
            u32 id = __builtin_ctzll(res)/8 - 1;
            ENTRY_NAME* e = &m->entries[probe * GROUP_SIZE + id];
            if (g->meta[id] == h2 && EQ(e->key, key)) {
                return &e->val;
            }
            res &= res - 1;
        }

        res = getMask(g1.align, emask);
        if (res) {
            if (!(g->overflow & (1 << h3))) {
                return NULL;
            }
        }

        g->overflow |= 1 << h3;
        probe = (probe + 1) % (m->cap / GROUP_SIZE);
    }

    return NULL;
}

void CAT(NAME, _del)(NAME* m, KEY key) {
    if (!m->cap || !m->load) return;
    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);
    u8 h3 = getH3(hash);

    const u64 emask = META_EMPTY * spread;
    const u64 hmask = h2 * spread;

    u32 probe = h1 % (m->cap/GROUP_SIZE);
    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        GROUP_NAME* g = &m->meta[probe];
        GROUP_NAME g1 = m->meta[probe];
        g1.overflow = META_SENTINEL;

        u64 res = getMask(g1.align, hmask);

        while (res) {
            u32 id = __builtin_ctzll(res)/8 - 1;
            ENTRY_NAME* e = &m->entries[probe * GROUP_SIZE + id];
            if (g->meta[id] == h2 && EQ(e->key, key)) {
                ON_DEL(m, e->key, e->val);
                g->meta[id] = META_EMPTY;
                return;
            }
            res &= res - 1;
        }

        res = getMask(g1.align, emask);
        if (res) {
            if (!(g->overflow & (1 << h3))) {
                return;
            }
        }

        g->overflow |= 1 << h3;
        probe = (probe + 1) % (m->cap / GROUP_SIZE);
    }

    return;
}

void CAT(NAME, _clear)(NAME* m) {
    m->load = 0;

    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        m->meta[i].overflow = 0;
        for (u32 j = 0; j < GROUP_SIZE; j++) {
            if (m->meta[i].meta[j] != META_EMPTY &&
                m->meta[i].meta[j] != META_DEL) {
                ENTRY_NAME* e = &m->entries[i * GROUP_SIZE + j];
                ON_CLEAR(m, e->key, e->val);
            }
            m->meta[i].meta[j] = META_EMPTY;
        }
    }
}

void CAT(NAME, _free)(NAME* m) {

    for (u32 i = 0; i < m->cap/GROUP_SIZE; i++) {
        for (u32 j = 0; j < GROUP_SIZE; j++) {
            if (m->meta[i].meta[j] != META_EMPTY &&
                m->meta[i].meta[j] != META_DEL) {
                ENTRY_NAME* e = &m->entries[i * GROUP_SIZE + j];
                ON_CLEAR(m, e->key, e->val);
            }
        }
    }

    Free(m->a, m->meta, sizeof(m->meta[0]) * (m->cap/GROUP_SIZE));
    Free(m->a, m->entries, sizeof(m->entries[0]) * m->cap);

    m->cap = 0;
    m->load = 0;
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
