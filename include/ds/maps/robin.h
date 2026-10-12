#ifndef NAME
#error Must set map name
#define NAME robin
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

#define META CAT(NAME, _meta)
typedef struct META {
    KEY key;
    u32 d;
} META;

typedef struct NAME {
    Allocator a;

    META* keys;
    VAL* vals;

    u32 load;
    u32 cap;

    USR_DATA;
} NAME;

#define MIN_CAP 16

#define INS_UNIQ CAT(NAME, _ins_uniq)
static void INS_UNIQ(NAME* m, KEY key, VAL val) {
    u64 hash = HASH(key); 
    u32 probe = hash & (m->cap - 1);

    u32 dist = 0;
    KEY tempk = key;
    VAL tempv = val;

    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[probe].d == -1) {
            //empty
            m->keys[probe].d = dist;
            m->keys[probe].key = tempk;
            m->vals[probe] = tempv;
            return;
        }
        
        if (m->keys[probe].d < dist) {
            //swap
            KEY tk = m->keys[probe].key;
            u32 td = m->keys[probe].d;
            VAL tv = m->vals[probe];

            m->keys[probe].key = tempk;
            m->keys[probe].d = dist;
            m->vals[probe] = tempv;

            tempk = tk;
            dist = td;
            tempv = tv;
        }

        dist++;
        probe = (probe + 1) & (m->cap - 1);
    }

    panic();


}

#define RESIZE CAT(NAME, _resize)
void RESIZE(NAME* m, u32 size) {
    u32 s = MIN_CAP;
    while (m->load * 8 >= s * 7 || s < size) s <<= 1;

    u32 oldcap = m->cap; 
    META* oldmeta = m->keys;
    VAL* oldvals = m->vals;

    m->cap = s;
    m->keys = Alloc(m->a, m->cap * sizeof(m->keys[0]));
    m->vals = Alloc(m->a, m->cap * sizeof(m->vals[0]));

    for (u32 i = 0; i < m->cap; i++) {
        m->keys[i].d = -1;
    }

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i].d == -1) continue;
        INS_UNIQ(m, oldmeta[i].key, oldvals[i]);
    }

    Free(m->a, oldmeta, oldcap * sizeof(oldmeta[0]));
    Free(m->a, oldvals, oldcap * sizeof(oldvals[0]));
}

void CAT(NAME, _ins)(NAME* m, KEY key, VAL val) {
    if (m->load * 8 >= m->cap * 7) {
        RESIZE(m, 0);
    }

    u64 hash = HASH(key); 
    u32 probe = hash & (m->cap - 1);

    u32 dist = 0;
    KEY tempk = key;
    VAL tempv = val;

    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[probe].d == -1) {
            //empty

            if (EQ(tempk, key)) {
                ON_INS(m, tempk, val);
            }

            m->load++;
            m->keys[probe].d = dist;
            m->keys[probe].key = tempk;
            m->vals[probe] = tempv;
            return;
        }

        if (EQ(m->keys[probe].key, key)) {
            //match
            m->vals[probe] = val;
            return;
        }
        
        if (m->keys[probe].d < dist) {
            //swap

            if (EQ(tempk, key)) {
                ON_INS(m, tempk, val);
            }

            KEY tk = m->keys[probe].key;
            u32 td = m->keys[probe].d;
            VAL tv = m->vals[probe];

            m->keys[probe].key = tempk;
            m->keys[probe].d = dist;
            m->vals[probe] = tempv;

            tempk = tk;
            dist = td;
            tempv = tv;
        }

        dist++;
        probe = (probe + 1) & (m->cap - 1);
    }

    panic();
}

u32* CAT(NAME, _get)(NAME* m, KEY key) {
    if (!m->cap || !m->load) return NULL;
    u64 hash = HASH(key); 
    u32 probe = hash & (m->cap - 1);

    u32 dist = 0;
    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[probe].d == -1) {
            return NULL;
        }

        if (EQ(m->keys[probe].key, key)) {
            //match
            return &m->vals[probe];
        }
        
        if (m->keys[probe].d < dist) {
            return NULL;
        }

        dist++;
        probe = (probe + 1) & (m->cap - 1);
    }
    panic();
}

void CAT(NAME, _del)(NAME* m, KEY key) {
    if (!m->cap || !m->load) return;
    u64 hash = HASH(key); 
    u32 probe = hash & (m->cap - 1);

    u32 dist = 0;

    //find
    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[probe].d == -1) {
            return;
        }

        if (EQ(m->keys[probe].key, key)) {
            ON_DEL(m, m->keys[probe].key, m->vals[probe]);
            break;
        }
        
        if (m->keys[probe].d < dist) {
            return;
        }

        dist++;
        probe = (probe + 1) & (m->cap - 1);
    }

    //back shift
    for (u32 i = 0; i < m->cap; i++) {
        u32 next = (probe + 1) & (m->cap - 1);
        //either next is empty or exactly on the spot
        if (m->keys[next].d == -1 || m->keys[next].d == 0) {
            m->keys[probe].d = -1;
            return;
        }

        m->keys[probe].d = m->keys[next].d - 1;
        m->keys[probe].key = m->keys[next].key;
        m->vals[probe] = m->vals[next];

        probe = (probe + 1) & (m->cap - 1);
    }

    panic();
}

void CAT(NAME, _clear)(NAME* m) {
    m->load = 0;
    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[i].d != -1) ON_CLEAR(m, m->keys[i].key, m->vals[i]);
        m->keys[i].d = -1;
    }
}

void CAT(NAME, _free)(NAME* m) {
    for (u32 i = 0; i < m->cap; i++) {
        if (m->keys[i].d != -1) ON_CLEAR(m, m->keys[i].key, m->vals[i]);
    }

    Free(m->a, m->keys, m->cap * sizeof(m->keys[0]));
    Free(m->a, m->vals, m->cap * sizeof(m->vals[0]));
    m->cap = 0;
    m->load = 0;
    m->keys = 0;
    m->vals = 0;
}

#undef GROUP_SIZE
#undef ON_INS
#undef ON_DEL
#undef NAME
#undef KEY
#undef VAL
#undef MIN_SIZE
#undef ENTRY_NAME
#undef META
#undef HASH
#undef EQ
#undef INS
#undef INS_UNIQ
#undef RESIZE
#undef USR_DATA
