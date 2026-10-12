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
    struct ENTRY_NAME* next;
    
    KEY key;
    VAL val;
} ENTRY_NAME;

typedef struct NAME {
    Allocator a;

    ENTRY_NAME** buckets;

    u32 load;
    u32 cap;

    USR_DATA;
} NAME;


#define MIN_CAP 16

#define INS_UNIQ CAT(NAME, _ins_uniq)
static void INS_UNIQ(NAME* m, KEY key, VAL val) {
    u64 hash = FNVHash64((u8*)&key, sizeof(key));
    u32 probe = hash % m->cap;
    ENTRY_NAME* e = m->buckets[probe];
    ENTRY_NAME* n = Alloc(m->a, sizeof(ENTRY_NAME));
    n->key = key;
    n->val = val;
    n->next = e;
    m->buckets[probe] = n;
}

#define RESIZE CAT(NAME, _resize)
void RESIZE(NAME* m, u32 size) {
    u32 s = 1;
    while (m->load >= s) s <<= 1;

    u32 oldcap = m->cap;
    ENTRY_NAME** oldvals = m->buckets;

    m->cap = s;
    m->buckets = Alloc(m->a, m->cap * sizeof(m->buckets[0]));
    memset(m->buckets, 0, m->cap * sizeof(m->buckets[0]));

    for (u32 i = 0; i < oldcap; i++) {
        if (!oldvals[i]) continue;
        ENTRY_NAME* e = oldvals[i];
        while (e) {
            INS_UNIQ(m, e->key, e->val);
            e = e->next;
        }
    }

    Free(m->a, oldvals, oldcap * sizeof(m->buckets[0]));
}


void CAT(NAME, _ins)(NAME* m, KEY key, VAL val) {
    if (m->load >= m->cap) {
        //rehash at 100% load factor
        RESIZE(m, MIN_CAP);
    }

    u64 hash = FNVHash64((u8*)&key, sizeof(key));
    u32 probe = hash % m->cap;
    ENTRY_NAME* e = m->buckets[probe];

    //traverse
    ENTRY_NAME* cur = e;
    while (cur) {
        if (cur->key == key) {
            cur->val = val;
            return;
        }
        cur = cur->next;
    }

    ON_INS(key, val);

    m->load++;
    ENTRY_NAME* n = Alloc(m->a, sizeof(ENTRY_NAME));
    n->key = key;
    n->val = val;
    n->next = e;
    m->buckets[probe] = n;
}

VAL* CAT(NAME, _get)(NAME* m, KEY key) {
    if (!m->cap) return NULL;
    u64 hash = FNVHash64((u8*)&key, sizeof(key));
    u32 probe = hash % m->cap;
    ENTRY_NAME* e = m->buckets[probe];

    //traverse
    ENTRY_NAME* cur = e;
    while (cur) {
        if (cur->key == key) {
            return &cur->val;
        }
        cur = cur->next;
    }
    
    return NULL;
}


void CAT(NAME, _del)(NAME* m, KEY key) {
    if (!m->cap) return;
    u64 hash = FNVHash64((u8*)&key, sizeof(key));
    u32 probe = hash % m->cap;
    ENTRY_NAME* e = m->buckets[probe];

    //traverse
    ENTRY_NAME* prev = NULL;
    ENTRY_NAME* cur = e;
    while (cur) {
        if (cur->key == key) {
            ON_DEL(cur->key, cur->val);
            if (prev) {
                prev->next = cur->next;
            } else {
                m->buckets[probe] = cur->next;
            }

            m->load--;
            Free(m->a, cur, sizeof(ENTRY_NAME));
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    
    return;
}

#define CLEAR CAT(NAME, _clear)
void CLEAR(NAME* m) {

    for (u32 i = 0; i < m->cap; i++) {
        ENTRY_NAME* e = m->buckets[i];
        if (!e) continue;
        while (e->next) {
            ENTRY_NAME* next = e->next;
            ON_CLEAR(m, e->key, e->val);
            Free(m->a, e, sizeof(ENTRY_NAME));
            e = next;
        }
        Free(m->a, e, sizeof(ENTRY_NAME));
        m->buckets[i] = NULL;
    }
    m->load = 0;
}

void CAT(NAME, _free)(NAME* m) {
    CLEAR(m);
    Free(m->a, m->buckets, m->cap * sizeof(m->buckets[0]));
    m->buckets = NULL;
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
