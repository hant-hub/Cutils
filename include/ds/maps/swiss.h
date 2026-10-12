
#ifndef NAME
#error Must set map name
#define NAME swiss
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
#include <immintrin.h>

#define GROUP_SIZE 16
#define MIN_SIZE GROUP_SIZE

#define ENTRY_NAME CAT(NAME, _entry)
typedef struct ENTRY_NAME {
    KEY key;
    VAL val;
} ENTRY_NAME;

typedef struct NAME {
    Allocator a;
    
    __m128i* groups;
    u8* meta;
    
    ENTRY_NAME* entries;

    u32 load;
    u32 cap;

    USR_DATA;
} NAME;

#include "fixed.h"

#define INS_UNIQ CAT(NAME, _ins_uniq)
static void INS_UNIQ(NAME* m, KEY key, VAL val) {
    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/16));
    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8((char)META_EMPTY);
        __m128i meta = _mm_load_si128(&m->groups[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            u32 id = (__builtin_ctz((u32)res));
            m->meta[probe * GROUP_SIZE + id] = h2;
            m->entries[probe * GROUP_SIZE + id].key = key;
            m->entries[probe * GROUP_SIZE + id].val = val;
            m->load++;
            return;
        }

        probe = (probe + 1) % (m->cap/16);
    }
    panic();
}

#define RESIZE CAT(NAME, _resize)
void RESIZE(NAME* m, u32 size) {

    u32 s = 16;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    ENTRY_NAME* oldvals = m->entries;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, (m->cap/16) * sizeof(m->groups[0]));
    m->entries = Alloc(m->a, m->cap * sizeof(m->entries[0]));

    m->groups = (__m128i*)m->meta;
    memset(m->meta, META_EMPTY, m->cap);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] == META_EMPTY) continue;
        if (oldmeta[i] == META_DEL) continue;
        INS_UNIQ(m, oldvals[i].key, oldvals[i].val);
    }

    Free(m->a, oldvals, oldcap * sizeof(m->entries[0]));
    Free(m->a, oldmeta, oldcap * sizeof(m->meta[0]));
}

void CAT(NAME, _ins)(NAME* m, KEY key, VAL val) {
    if (m->load * 8 >= m->cap * 7) {
        RESIZE(m, MIN_SIZE);
    }

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/16));
    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_load_si128(&m->groups[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));
            ENTRY_NAME* s = &m->entries[probe * GROUP_SIZE + id];

            if (EQ(s->key, key)) {
                s->val = val;
                return;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8((char)META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            mask = _mm_set1_epi8((char)META_DEL);
            res |= _mm_movemask_epi8(_mm_cmpeq_epi8(meta, mask));

            u32 id = (__builtin_ctz((u32)res));

            m->meta[probe * GROUP_SIZE + id] = h2;
            ENTRY_NAME* s = &m->entries[probe * GROUP_SIZE + id];

            ON_INS(m, key, val);

            s->key = key;
            s->val = val;
            m->load++;
            return;
        }

        probe = (probe + 1) % (m->cap/16);
    }
    panic();
}

u32* CAT(NAME, _get)(NAME* m, KEY key) {
    if (!m->cap) return NULL;

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/16));
    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_load_si128(&m->groups[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));
            ENTRY_NAME* s = &m->entries[probe * GROUP_SIZE + id];

            if (EQ(s->key, key)) {
                return &s->val;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8((char)META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            return NULL;
        }

        probe = (probe + 1) % (m->cap/16);
    }
    panic();
}

void CAT(NAME, _del)(NAME* m, KEY key) {
    if (!m->cap) return;

    u64 hash = HASH(key);
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 % (m->cap/16));
    for (u32 i = 0; i < m->cap; i++) {

        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_load_si128(&m->groups[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));
            ENTRY_NAME* s = &m->entries[probe * GROUP_SIZE + id];

            if (EQ(s->key, key)) {
                ON_DEL(m, s->key, s->val);
                m->meta[probe * GROUP_SIZE + id] = META_DEL;
                return;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8((char)META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            return;
        }

        probe = (probe + 1) % (m->cap/16);
    }
    panic();
}

void CAT(NAME, _clear)(NAME* m) {
    for (u32 i = 0; i < m->cap; i++) {
        if (m->meta[i] != META_DEL &&
            m->meta[i] != META_EMPTY) {
            ON_CLEAR(m, m->entries[i].key, m->entries[i].val);
        }
        m->meta[i] = META_EMPTY;
    }

}


void CAT(NAME, _free)(NAME* m) {
    for (u32 i = 0; i < m->cap; i++) {
        if (m->meta[i] != META_DEL &&
            m->meta[i] != META_EMPTY) {
            ON_CLEAR(m, m->entries[i].key, m->entries[i].val);
        }
    }


    Free(m->a, m->meta,    (m->cap) * sizeof(m->meta[0]));
    Free(m->a, m->entries, (m->cap) * sizeof(m->entries[0]));

    m->meta = NULL;
    m->groups = NULL;
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
