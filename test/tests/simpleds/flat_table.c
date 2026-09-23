#define CU_IMPL
#include <core/cutils.h>
#include <ds/ds.h>
#include <time.h>

DefHashMapDecl(imap, u32, u32);
DefHashMapImpl(imap, u32, u32);


#define NUM_INSERTS 10000000

#define MIN_SIZE 32 
#define GROUP_SIZE 8 

typedef struct flat_imap_group {
    union {
        u8 meta[GROUP_SIZE];
        u64 align;
    };
    u32 keys[GROUP_SIZE];
} flat_imap_group;

typedef struct flat_imap {
    Allocator a;
    flat_imap_group* groups;
    u32* vals;

    u32 load;
    u32 cap;
} flat_imap;

typedef enum flat_imap_meta {
    META_EMPTY = 0b10000000,
    META_DEL   = 0b11111110,
    META_FULL  = 0b01111111,
} flat_imap_meta;

void flat_imap_free(flat_imap* m) {
    Free(m->a, m->groups, m->cap * sizeof(m->groups[0]));
    m->cap = 0;
}

static u64 getH1(u64 h) {
    return (h >> 7);
}

static u8 getH2(u64 h) {
    return (h & META_FULL);
}

static const u64 spread = 0x0101010101010101;
static const u64 final_mask = 0x8080808080808080;

static inline u64 getMask(u64 meta, u64 mask) {
    const u64 matches = meta ^ mask;
    u64 res = (matches - spread) & ~matches;
    return res & final_mask;
}

#define HASH FNVHash64

static void flat_imap_ins_uniq(flat_imap* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 emask = META_EMPTY * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group* g = &m->groups[probe];

        u64 empty = getMask(g->align, emask);

        //must probe next
        if (empty) {
            u8 id = (__builtin_ctzll(empty)/8);
            g->meta[id] = h2;
            g->keys[id] = key;
            m->vals[probe * GROUP_SIZE + id] = val;

            m->load++;
            return;
        }

        //linear probe
        probe = (probe + 1) % m->cap;
    }
    panic();
}

void flat_imap_rehash(flat_imap* m, u32 size) {
    u32 s = 1;
    while (s * GROUP_SIZE <= size) s <<= 1;
    size = s;

    u32 oldcap = m->cap;
    flat_imap_group* oldgroups = m->groups;
    u32* oldvals = m->vals;

    m->cap = s;
    m->groups = Alloc(m->a, sizeof(m->groups[0]) * m->cap); 
    m->vals   = Alloc(m->a, sizeof(m->vals[0]) * GROUP_SIZE * m->cap);

    for (u32 i = 0; i < m->cap; i++) {
        memset(&m->groups[i].align, META_EMPTY, GROUP_SIZE);
    }

    for (u32 i = 0; i < oldcap; i++) {
        flat_imap_group* g = &m->groups[i];
        for (u32 j = 0; j < GROUP_SIZE; j++) {
            if ((g->meta[j] & META_EMPTY) == 0) {
                flat_imap_ins_uniq(m, g->keys[j], oldvals[i * GROUP_SIZE + j]);
            }
        }
    }

    Free(m->a, oldgroups, sizeof(oldgroups[0]) * oldcap); 
    Free(m->a, oldvals, sizeof(oldvals[0]) * oldcap);
}

void flat_imap_ins(flat_imap* m, u32 key, u32 val) {

    //printlog("(load, cap) %d %d\n", m->load, m->cap * GROUP_SIZE * 0.7);
    //90% max load factor
    if (m->load >= m->cap * GROUP_SIZE * 0.9) {
        if (!m->cap) m->cap = MIN_SIZE;
        flat_imap_rehash(m, m->cap * GROUP_SIZE * 2);
    }

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 hmask = h2 * spread;
    const u64 emask = META_EMPTY * spread;
    const u64 dmask = META_DEL * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group* g = &m->groups[probe];

        u64 res = getMask(g->align, hmask);

        //check matches
        while (res) {
            //matches
            u8 id = (__builtin_ctzll(res)/8);
            if (memcmp(&key, &g->keys[id], sizeof(key)) == 0) {
                m->vals[probe * GROUP_SIZE + id] = val;
                return;
            }
            res &= res - 1;
        }

        u64 empty = getMask(g->align, emask);
        //must probe next
        if (empty) {
            u64 deleted = getMask(g->align, dmask);

            u8 id = (__builtin_ctzll(deleted | empty)/8);
            g->meta[id] = h2;
            g->keys[id] = key;
            m->vals[probe * GROUP_SIZE + id] = val;

            m->load++;
            return;
        }

        //linear probe
        probe = (probe + 1) % m->cap;
    }

    panic();
}

u32* flat_imap_get(flat_imap* m, u32 key) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 hmask = h2 * spread;
    const u64 emask = META_EMPTY * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group* g = &m->groups[probe];

        u64 res = getMask(g->align, hmask);
        //check matches
        while (res) {
            //matches
            u8 id = (__builtin_ctzll(res)/8);
            if (memcmp(&key, &g->keys[id], sizeof(key)) == 0) {
                return &m->vals[probe * GROUP_SIZE + id];
            }
            res &= res - 1;
        }

        u64 empty = getMask(g->align, emask);
        //must probe next
        if (empty) {
            return NULL;
        }

        //quadratic probe
        probe = (probe + 1) % m->cap;
    }

    panic();
}

int main() {
    logfile->buffered = FALSE;
    //benchmark imap
    printlog("Flat imap (template + SIMD probing)\n");
    clock_t f_new;
    {
        flat_imap map = {
            .a = GlobalAllocator
        };

        //flat_imap_reserve(&map, NUM_INSERTS);


        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);
        f_new = diff;

        //printlog("\t Group Miss: %d\n", group_miss);
        //printlog("\t Failed CMP: %d\n", failed_cmp);

        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins(&map, i, i);
        }
        diff = clock() - start;
        printlog("\t Insert(Hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        //get (hit)
        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_get(&map, i);
        }
        diff = clock() - start;
        printlog("\t Get(Hit):   \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        //get (miss)
        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_get(&map, i + NUM_INSERTS);
        }
        diff = clock() - start;
        printlog("\t Get(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////delete (miss)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    flat_imap_del(&map, i + NUM_INSERTS);
        //}
        //diff = clock() - start;
        //printlog("\t Delete(miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////delete (hit)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    flat_imap_del(&map, i);
        //}
        //diff = clock() - start;
        //printlog("\t Delete(hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);


        flat_imap_free(&map);
    }

    printlog("Default Map:\n");
    clock_t f_old;
    {
        imap map = {.a = GlobalAllocator};
        //imapReserve(&map, NUM_INSERTS); 

        //insert (miss)
        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            imapIns(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);
        f_old = diff;

        //insert (hit)
        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            imapIns(&map, i, i);
        }
        diff = clock() - start;
        printlog("\t Insert(Hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        //get (hit)
        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            imapGet(&map, i);
        }
        diff = clock() - start;
        printlog("\t Get(Hit):   \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        //get (miss)
        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            imapGet(&map, i + NUM_INSERTS);
        }
        diff = clock() - start;
        printlog("\t Get(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////delete (miss)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    imapDel(&map, i + NUM_INSERTS);
        //}
        //diff = clock() - start;
        //printlog("\t Delete(miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////delete (hit)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    imapDel(&map, i);
        //}
        //diff = clock() - start;
        //printlog("\t Delete(hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        imapFree(&map);
    }

    u32 speed = (f_old - f_new) * 100;
    speed /= f_old;
    printlog("Speedup: %d%%\n", speed);
    return 0;
}
