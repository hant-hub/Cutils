#define CU_IMPL
#include <core/cutils.h>
#include <ds/ds.h>
#include <time.h>

typedef struct imap {
    Allocator a;
    u32 cap;
    u32 size;
    struct {
        u32 k;
        i32 m;
    }* keys;
    u32* vals;
} imap;
void imapIns(imap*, u32, u32);
u32* imapGet(imap*, u32);
void imapDel(imap*, u32);
void imapClear(imap*);
void imapFree(imap*);

void imapIns(imap* map, u32 ky, u32 vl) {                                
    if (map->cap * MAX_LOAD <= map->size + 1) {                            
        u64 oldcap = map->cap;                                             
        map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      

        typeof(map->keys) oldkeys = map->keys;                             
        typeof(map->vals) oldvals = map->vals;                             

        map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));        
        map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));        

        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));            
        for (u32 i = 0; i < oldcap; i++) {                                 
            if (oldkeys[i].m == -1)                                        
                continue;                                                  
            u32 tempk = oldkeys[i].k;                                      
            u32 tempv = oldvals[i];                                        
            u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));              

            u32 idx = hash % map->cap;                                     
            u32 dist = 0;                                                  

            for (u64 i = 0; i < map->cap; i++) {                           
                if (map->keys[idx].m == -1) {                              
                    map->keys[idx].k = tempk;                              
                    map->keys[idx].m = dist;                               
                    map->vals[idx] = tempv;                                
                    break;                                                 
                }                                                          
                if (map->keys[idx].m < dist) {                             
                    u32 tmpkey = map->keys[idx].k;                         
                    u32 tmpval = map->vals[idx];                           
                    u32 tmpdist = map->keys[idx].m;                        

                    map->keys[idx].k = tempk;                              
                    map->keys[idx].m = dist;                               
                    map->vals[idx] = tempv;                                

                    tempk = tmpkey;                                        
                    tempv = tmpval;                                        
                    dist = tmpdist;                                        
                }                                                          

                if (memcmp(&map->keys[idx].k, &tempk, sizeof(u32)) == 0) { 
                    map->vals[idx] = tempv;                                
                    break;                                                 
                }                                                          
                idx = (idx + 1) % map->cap;                                
                dist++;                                                    
            }                                                              
        }                                                                  

        Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));              
        Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));              
    }                                                                      

    u32 tempk = ky;                                                        
    u32 tempv = vl;                                                        
    u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      

    u32 idx = hash % map->cap;                                             
    u32 dist = 0;                                                          

    for (u64 i = 0; i < map->cap; i++) {                                   
        if (map->keys[idx].m == -1) {                                      
            map->keys[idx].k = tempk;                                      
            map->keys[idx].m = dist;                                       
            map->vals[idx] = tempv;                                        
            map->size++;                                                   
            break;                                                         
        }                                                                  
        if (map->keys[idx].m < dist) {                                     
            u32 tmpkey = map->keys[idx].k;                                 
            u32 tmpval = map->vals[idx];                                   
            u32 tmpdist = map->keys[idx].m;                                

            map->keys[idx].k = tempk;                                      
            map->keys[idx].m = dist;                                       
            map->vals[idx] = tempv;                                        

            tempk = tmpkey;                                                
            tempv = tmpval;                                                
            dist = tmpdist;                                                
        }                                                                  

        if (memcmp(&map->keys[idx].k, &tempk, sizeof(u32)) == 0) {         
            map->vals[idx] = tempv;                                        
            break;                                                         
        }                                                                  
        idx = (idx + 1) % map->cap;                                        
        dist++;                                                            
    }                                                                      
}                                                                          
u32* imapGet(imap* map, u32 ky) {                                        
    if (!map->cap)                                                         
        return NULL;                                                       
    u32 tempk = ky;                                                        
    u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      

    u32 idx = hash % map->cap;                                             
    u32 dist = 0;                                                          

    for (u64 i = 0; i < map->cap; i++) {                                   
        if (map->keys[idx].m == -1) {                                      
            return NULL;                                                   
        }                                                                  
        if (map->keys[idx].m < dist) {                                     
            return NULL;                                                   
        }                                                                  

        if (memcmp(&map->keys[idx].k, &tempk, sizeof(u32)) == 0) {         
            return &map->vals[idx];                                        
        }                                                                  
        idx = (idx + 1) % map->cap;                                        
        dist++;                                                            
    }                                                                      

    return NULL;                                                           
}                                                                          
void imapDel(imap* map, u32 ky) {                                        

    u32 tempk = ky;                                                        

    u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                      
    u32 idx = hash % map->cap;                                             
    u32 dist = 0;                                                          

    for (u64 i = 0; i < map->cap; i++) {                                   
        if (map->keys[idx].m == -1) {                                      
            return;                                                        
        }                                                                  
        if (map->keys[idx].m < dist) {                                     
            return;                                                        
        }                                                                  

        if (memcmp(&map->keys[idx].k, &tempk, sizeof(u32)) == 0) {         
            map->size--;                                                   
            idx = (idx + 1) % map->cap;                                    
            break;                                                         
        }                                                                  
        idx = (idx + 1) % map->cap;                                        
        dist++;                                                            
    }                                                                      

    u32 prev = (idx + map->cap - 1) % map->cap;                            
    while (map->keys[idx].m != -1 && map->keys[idx].m > 0) {               
        map->keys[prev] = map->keys[idx];                                  
        map->keys[prev].m -= 1;                                            

        map->vals[prev] = map->vals[idx];                                  
        idx = (idx + 1) % map->cap;                                        
        prev = (idx + map->cap - 1) % map->cap;                            
    }                                                                      

    map->keys[prev].m = -1;                                                
}                                                                          
void imapReserve(imap* map, u64 size) {                                  
    u32 oldcap = map->cap;                                                 
    while (map->cap < size) {                                              
        map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      
    }                                                                      

    typeof(map->keys) oldkeys = map->keys;                                 
    typeof(map->vals) oldvals = map->vals;                                 

    map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));            
    map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));            

    memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                
    for (u32 i = 0; i < oldcap; i++) {                                     
        if (oldkeys[i].m == -1)                                            
            continue;                                                      

        u32 tempk = oldkeys[i].k;                                          
        u32 tempv = oldvals[i];                                            
        u32 hash = FNVHash32((u8*)&tempk, sizeof(tempk));                  

        u32 idx = hash % map->cap;                                         
        u32 dist = 0;                                                      

        for (u64 i = 0; i < map->cap; i++) {                               
            if (map->keys[idx].m == -1) {                                  
                map->keys[idx].k = tempk;                                  
                map->keys[idx].m = dist;                                   
                map->vals[idx] = tempv;                                    
                break;                                                     
            }                                                              
            if (map->keys[idx].m < dist) {                                 
                u32 tmpkey = map->keys[idx].k;                             
                u32 tmpval = map->vals[idx];                               
                u32 tmpdist = map->keys[idx].m;                            

                map->keys[idx].k = tempk;                                  
                map->keys[idx].m = dist;                                   
                map->vals[idx] = tempv;                                    

                tempk = tmpkey;                                            
                tempv = tmpval;                                            
                dist = tmpdist;                                            
            }                                                              

            if (memcmp(&map->keys[idx].k, &tempk, sizeof(u32)) == 0) {     
                map->vals[idx] = tempv;                                    
                break;                                                     
            }                                                              
            idx = (idx + 1) % map->cap;                                    
            dist++;                                                        
        }                                                                  
    }                                                                      

    Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));                  
    Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));                  
}                                                                          
void imapClear(imap* map) {                                              
    map->size = 0;                                                         
    memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                
}                                                                          
void imapFree(imap* map) {                                               
    Free(map->a, map->keys, map->cap * sizeof(map->keys[0]));              
    Free(map->a, map->vals, map->cap * sizeof(map->vals[0]));              
}


#define NUM_INSERTS 5000000

#define MIN_SIZE 32 
#define GROUP_SIZE 8 

typedef struct flat_imap_group_swar {
    union {
        u8 meta[GROUP_SIZE];
        u64 align;
    };
    u32 keys[GROUP_SIZE];
} flat_imap_group_swar;

typedef struct flat_imap_swar {
    Allocator a;
    flat_imap_group_swar* groups;
    u32* vals;

    u32 load;
    u32 cap;
} flat_imap_swar;

typedef enum flat_imap_meta : u8 {
    META_EMPTY    = 0b10000000,
    META_DEL      = 0b11111110,
    META_FULL     = 0b01111111,
    META_SENTINEL = 0b11111111,
} flat_imap_meta;

void flat_imap_free_swar(flat_imap_swar* m) {
    Free(m->a, m->groups, m->cap * sizeof(m->groups[0]));
    Free(m->a, m->vals, m->cap * sizeof(m->vals[0]));
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

static void flat_imap_ins_uniq_swar(flat_imap_swar* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 emask = META_EMPTY * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group_swar* g = &m->groups[probe];

        u64 empty = getMask(g->align, emask);

        //must probe next
        if (empty) {
            u32 id = (__builtin_ctzll(empty)/8);
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

void flat_imap_rehash_swar(flat_imap_swar* m, u32 size) {
    u32 s = 1;
    while (s * GROUP_SIZE <= size) s <<= 1;
    size = s;

    u32 oldcap = m->cap;
    flat_imap_group_swar* oldgroups = m->groups;
    u32* oldvals = m->vals;

    m->cap = s;
    m->load = 0;
    m->groups = Alloc(m->a, sizeof(m->groups[0]) * m->cap); 
    m->vals   = Alloc(m->a, sizeof(m->vals[0]) * GROUP_SIZE * m->cap);

    for (u32 i = 0; i < m->cap; i++) {
        memset(&m->groups[i].align, META_EMPTY, GROUP_SIZE);
    }

    for (u32 i = 0; i < oldcap; i++) {
        flat_imap_group_swar* g = &m->groups[i];
        for (u32 j = 0; j < GROUP_SIZE; j++) {
            if ((g->meta[j] & META_EMPTY) == 0) {
                flat_imap_ins_uniq_swar(m, g->keys[j], oldvals[i * GROUP_SIZE + j]);
            }
        }
    }

    Free(m->a, oldgroups, sizeof(oldgroups[0]) * oldcap); 
    Free(m->a, oldvals, sizeof(oldvals[0]) * oldcap);
}

void flat_imap_ins_swar(flat_imap_swar* m, u32 key, u32 val) {

    //printlog("(load, cap) %d %d\n", m->load, m->cap * GROUP_SIZE * 0.7);
    //90% max load factor
    if (m->load >= m->cap * GROUP_SIZE * 0.9) {
        if (!m->cap) m->cap = MIN_SIZE;
        flat_imap_rehash_swar(m, m->cap * GROUP_SIZE * 2);
    }

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 hmask = h2 * spread;
    const u64 emask = META_EMPTY * spread;
    const u64 dmask = META_DEL * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group_swar* g = &m->groups[probe];

        u64 res = getMask(g->align, hmask);

        //check matches
        while (res) {
            //matches
            u32 id = (__builtin_ctzll(res)/8);
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

            u32 id = (__builtin_ctzll(deleted | empty)/8);
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

u32* flat_imap_get_swar(flat_imap_swar* m, u32 key) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 hmask = h2 * spread;
    const u64 emask = META_EMPTY * spread;

    u32 probe = h1 & (m->cap - 1);
    for (u32 i = 0; i < m->cap; i++) {
        flat_imap_group_swar* g = &m->groups[probe];

        u64 res = getMask(g->align, hmask);
        //check matches
        while (res) {
            //matches
            u32 id = (__builtin_ctzll(res)/8);
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

#include <immintrin.h>

typedef struct flat_imap_simd_sep {
    Allocator a;
    u8* meta;
    u32* keys;
    u32* vals;

    u32 load;
    u32 cap;
} flat_imap_simd_sep;

void flat_imap_free_simd_sep(flat_imap_simd_sep* m) {
    Free(m->a, m->vals, m->cap * sizeof(m->vals[0]));
    Free(m->a, m->keys, m->cap * sizeof(m->keys[0]));
    Free(m->a, m->meta, m->cap * sizeof(m->meta[0]));
    m->cap = 0;
}

#define HASH FNVHash64

static void flat_imap_ins_uniq_simd_sep(flat_imap_simd_sep* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1)) / 16;
    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8(META_EMPTY);
        __m128i meta = _mm_load_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            u32 id = (__builtin_ctz((u32)res));
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            m->keys[probe + id] = key;
            m->vals[probe + id] = val;
            m->load++;
            return;
        }

        probe = (probe + 16) % (m->cap);
    }
    panic();
}

void flat_imap_resize_simd_sep(flat_imap_simd_sep* m, u32 size) {
    u32 s = 16;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    u32* oldvals = m->vals;
    u32* oldkeys = m->keys;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, m->cap + 16);
    m->keys = Alloc(m->a, m->cap * sizeof(m->keys[0]));
    m->vals = Alloc(m->a, m->cap * sizeof(m->vals[0]));

    memset(m->meta, META_EMPTY, m->cap);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] < META_SENTINEL) continue;
        flat_imap_ins_uniq_simd_sep(m, oldkeys[i], oldvals[i]);
    }

    Free(m->a, oldvals, oldcap * sizeof(m->vals[0]));
    Free(m->a, oldkeys, oldcap * sizeof(m->vals[0]));
    Free(m->a, oldmeta, oldcap * sizeof(m->vals[0]));
}

static u32 ins_count = 0;
static u32 miss_count = 0;

void flat_imap_ins_simd_sep(flat_imap_simd_sep* m, u32 key, u32 val) {
    if (m->load * 8 >= m->cap * 7) {
        flat_imap_resize_simd_sep(m, MIN_SIZE);
    }

    ins_count++;

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1));
    for (u32 i = 0; i < m->cap; i++) {

        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_loadu_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));

            if (m->keys[probe + id] == key) {
                m->vals[probe + id] = val;
                return;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8(META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            mask = _mm_set1_epi8(META_DEL);
            res |= _mm_movemask_epi8(_mm_cmpeq_epi8(meta, mask));

            u32 id = (__builtin_ctz((u32)res));
            if (probe >= m->cap - 16) probe -= m->cap - 16;
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            m->keys[probe + id] = key;
            m->vals[probe + id] = val;
            m->load++;
            return;
        }

        miss_count++;
        probe = (probe + 16) % (m->cap);
    }
    panic();
}

#include <immintrin.h>

struct slot {
    u32 key;
    u32 val;
};

typedef struct flat_imap_simd_sep2 {
    Allocator a;
    u8* meta;
    struct slot* slots;

    u32 load;
    u32 cap;
} flat_imap_simd_sep2;

void flat_imap_free_simd_sep2(flat_imap_simd_sep2* m) {
    Free(m->a, m->slots, m->cap * sizeof(m->slots[0]));
    Free(m->a, m->meta, m->cap * sizeof(m->meta[0]));
    m->cap = 0;
}

#define HASH FNVHash64

static void flat_imap_ins_uniq_simd_sep2(flat_imap_simd_sep2* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1)) / 16;
    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8(META_EMPTY);
        __m128i meta = _mm_load_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            u32 id = (__builtin_ctz((u32)res));
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            m->slots[probe + id].key = key;
            m->slots[probe + id].val = val;
            m->load++;
            return;
        }

        probe = (probe + 16) % (m->cap);
    }
    panic();
}

void flat_imap_resize_simd_sep2(flat_imap_simd_sep2* m, u32 size) {
    u32 s = 16;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    struct slot * oldvals = m->slots;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, m->cap + 16);
    m->slots = Alloc(m->a, m->cap * sizeof(m->slots[0]));

    memset(m->meta, META_EMPTY, m->cap);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] < META_SENTINEL) continue;
        flat_imap_ins_uniq_simd_sep2(m, oldvals[i].key, oldvals[i].val);
    }

    Free(m->a, oldvals, oldcap * sizeof(m->slots[0]));
    Free(m->a, oldmeta, oldcap * sizeof(m->meta[0]));
}

void flat_imap_ins_simd_sep2(flat_imap_simd_sep2* m, u32 key, u32 val) {
    if (m->load * 8 >= m->cap * 7) {
        flat_imap_resize_simd_sep2(m, MIN_SIZE);
    }

    ins_count++;

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1));
    for (u32 i = 0; i < m->cap; i++) {

        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_loadu_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));
            struct slot* s = &m->slots[probe + id];

            if (s->key == key) {
                s->val = val;
                return;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8(META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            mask = _mm_set1_epi8(META_DEL);
            res |= _mm_movemask_epi8(_mm_cmpeq_epi8(meta, mask));

            u32 id = (__builtin_ctz((u32)res));
            if (probe >= m->cap - 16) probe -= m->cap - 16;
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            struct slot* s = &m->slots[probe + id];
            s->key = key;
            s->val = val;
            m->load++;
            return;
        }

        miss_count++;
        probe = (probe + 16) % (m->cap);
    }
    panic();
}

#include <immintrin.h>

typedef struct flat_imap_simd_sep3 {
    Allocator a;
    u8* meta;
    struct slot* slots;

    u32 load;
    u32 cap;
} flat_imap_simd_sep3;

void flat_imap_free_simd_sep3(flat_imap_simd_sep3* m) {
    Free(m->a, m->slots, m->cap * sizeof(m->slots[0]));
    Free(m->a, m->meta, (m->cap + 8) * sizeof(m->meta[0]));
    m->cap = 0;
}

#define HASH FNVHash64

static void flat_imap_ins_uniq_simd_sep3(flat_imap_simd_sep3* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1)) / 16;

    for (u32 i = 0; i < m->cap; i++) {
        __m128i mask = _mm_set1_epi8(META_EMPTY);
        __m128i meta = _mm_load_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            u32 id = (__builtin_ctz((u32)res));
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            m->slots[probe + id].key = key;
            m->slots[probe + id].val = val;
            m->load++;
            return;
        }

        probe = (probe + 16) % (m->cap);
    }
    panic();
}

void flat_imap_resize_simd_sep3(flat_imap_simd_sep3* m, u32 size) {
    u32 s = 16;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    struct slot * oldvals = m->slots;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, m->cap + 16);
    m->slots = Alloc(m->a, m->cap * sizeof(m->slots[0]));

    memset(m->meta, META_EMPTY, m->cap);
    memset(&m->meta[m->cap], META_SENTINEL, 16);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] < META_SENTINEL) continue;
        flat_imap_ins_uniq_simd_sep3(m, oldvals[i].key, oldvals[i].val);
    }

    Free(m->a, oldvals, oldcap * sizeof(m->slots[0]));
    Free(m->a, oldmeta, oldcap * sizeof(m->meta[0]));
}

void flat_imap_ins_simd_sep3(flat_imap_simd_sep3* m, u32 key, u32 val) {
    if (m->load * 8 >= m->cap * 7) {
        flat_imap_resize_simd_sep3(m, MIN_SIZE);
    }

    ins_count++;

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    u32 probe = (h1 & (m->cap - 1));
    for (u32 i = 0; i < m->cap; i++) {

        __m128i mask = _mm_set1_epi8(h2);
        __m128i meta = _mm_loadu_si128((__m128i*)&m->meta[probe]);
        u16 res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));
        
        while (res) {
            u32 id = (__builtin_ctz((u32)res));
            struct slot* s = &m->slots[probe + id];

            if (s->key == key) {
                s->val = val;
                return;
            }

            res &= res - 1;
        }

        mask = _mm_set1_epi8(META_EMPTY);
        res = _mm_movemask_epi8(_mm_cmpeq_epi8(mask, meta));

        if (res) {
            mask = _mm_set1_epi8(META_DEL);
            res |= _mm_movemask_epi8(_mm_cmpeq_epi8(meta, mask));

            u32 id = (__builtin_ctz((u32)res));
            if (probe >= m->cap - 16) probe -= m->cap - 16;
            m->meta[probe + id] = h2;
            if (probe <= 16) m->meta[m->cap - 16 + id] = h2;
            struct slot* s = &m->slots[probe + id];
            s->key = key;
            s->val = val;
            m->load++;
            return;
        }

        miss_count++;
        probe = (probe + 16) % (m->cap);
    }
    panic();
}

#include <immintrin.h>

typedef struct flat_imap3 {
    Allocator a;
    u8* meta;
    struct slot* slots;

    u32 load;
    u32 cap;
} flat_imap3;

void flat_imap_free3(flat_imap3* m) {
    Free(m->a, m->slots, m->cap * sizeof(m->slots[0]));
    Free(m->a, m->meta, m->cap * sizeof(m->meta[0]));
    m->cap = 0;
}

#define HASH FNVHash64

static void flat_imap_ins_uniq3(flat_imap3* m, u32 key, u32 val) {
    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 emask = spread * META_EMPTY;

    u32 probe = (h1 & (m->cap - 1))/8;
    probe *= 8;
    for (u32 i = 0; i < m->cap; i++) {

        u64 res = getMask(*(u64*)&m->meta[probe], emask);
        while (res) {
            u32 id = (__builtin_ctzll(res)/8);
            if (m->meta[probe + id] != META_EMPTY) {
                res &= res - 1;
                continue;
            }
            m->meta[probe + id] = h2;
            m->slots[probe + id].key = key;
            m->slots[probe + id].val = val;
            m->load++;
            return;
        }

        probe = (probe + 8) % (m->cap);
    }
    panic();
}

void flat_imap_resize3(flat_imap3* m, u32 size) {
    u32 s = 8;
    while (s <= size || m->load * 8 >= s * 7) s <<= 1;

    u32 oldcap = m->cap;
    u8* oldmeta = m->meta;
    struct slot * oldvals = m->slots;

    m->cap = s;
    m->load = 0;
    m->meta = Alloc(m->a, m->cap);
    m->slots = Alloc(m->a, m->cap * sizeof(m->slots[0]));

    memset(m->meta, META_EMPTY, m->cap);

    for (u32 i = 0; i < oldcap; i++) {
        if (oldmeta[i] < META_SENTINEL) continue;
        flat_imap_ins_uniq3(m, oldvals[i].key, oldvals[i].val);
    }

    Free(m->a, oldvals, oldcap * sizeof(m->slots[0]));
    Free(m->a, oldmeta, oldcap * sizeof(m->meta[0]));
}

void flat_imap_ins3(flat_imap3* m, u32 key, u32 val) {
    if (m->load * 8 >= m->cap * 6) {
        flat_imap_resize3(m, MIN_SIZE);
    }

    u64 hash = HASH((u8*)&key, sizeof(key));
    u64 h1 = getH1(hash);
    u8 h2 = getH2(hash);

    const u64 mask = spread * h2;
    const u64 emask = spread * META_EMPTY;
    const u64 dmask = spread * META_DEL;

    u32 probe = (h1 & (m->cap - 1))/8;
    probe *= 8;
    for (u32 i = 0; i < m->cap; i++) {

        u64 res = getMask(*(u64*)&m->meta[probe], mask);
        while (res) {
            u32 id = (__builtin_ctz((u32)res)/8);
            struct slot* s = &m->slots[probe + id];

            if (!(m->meta[probe + id] & ~META_FULL) && s->key == key) {
                s->val = val;
                return;
            }

            res &= res - 1;
        }

        res = getMask(*(u64*)&m->meta[probe], emask);
        if (res) {
            res |= getMask((u64)&m->meta[probe], dmask);
            u32 id = (__builtin_ctz((u32)res)/8);
            m->meta[probe + id] = h2;
            struct slot* s = &m->slots[probe + id];
            s->key = key;
            s->val = val;
            m->load++;
            return;
        }

        probe = (probe + 8) % (m->cap);
    }
    panic();
}

int main() {
    logfile->buffered = FALSE;
    //benchmark imap
    printlog("Flat imap3 (template + SWAR probing + sep metadata)\n");
    {
        flat_imap3 map = {
            .a = GlobalAllocator
        };

        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins3(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins3(&map, i, i);
        }
        diff = clock() - start;
        printlog("\t Insert(Hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        flat_imap_free3(&map);
    }
    printlog("Flat imap3 (template + SIMD probing + sep metadata)\n");
    {
        flat_imap_simd_sep3 map = {
            .a = GlobalAllocator
        };

        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins_simd_sep3(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        printlog("\t Count, Miss: (%d %d)\n", ins_count, miss_count);
        printlog("\t Miss rate: %d%%\n", (miss_count * 100) / ins_count);
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        flat_imap_free_simd_sep3(&map);
    }
    printlog("Flat imap2 (template + SIMD probing + sep metadata)\n");
    {
        flat_imap_simd_sep2 map = {
            .a = GlobalAllocator
        };

        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins_simd_sep2(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        printlog("\t Count, Miss: (%d %d)\n", ins_count, miss_count);
        printlog("\t Miss rate: %d%%\n", (miss_count * 100) / ins_count);
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        flat_imap_free_simd_sep2(&map);
    }
    printlog("Flat imap (template + SIMD probing + sep metadata)\n");
    {
        flat_imap_simd_sep map = {
            .a = GlobalAllocator
        };

        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins_simd_sep(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        printlog("\t Count, Miss: (%d %d)\n", ins_count, miss_count);
        printlog("\t Miss rate: %d%%\n", (miss_count * 100) / ins_count);
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        flat_imap_free_simd_sep(&map);
    }
    printlog("Flat imap (template + SWAR probing)\n");
    clock_t f_new;
    {
        flat_imap_swar map = {
            .a = GlobalAllocator
        };

        //flat_imap_reserve(&map, NUM_INSERTS);


        clock_t start = clock();
        for (u32 i = 0; i < NUM_INSERTS; i++) {
            flat_imap_ins_swar(&map, i, i);
        }
        clock_t diff = clock() - start;
        printlog("\t Insert(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);
        f_new = diff;
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        //printlog("\t Group Miss: %d\n", group_miss);
        //printlog("\t Failed CMP: %d\n", failed_cmp);

        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    flat_imap_ins_swar(&map, i, i);
        //}
        //diff = clock() - start;
        //printlog("\t Insert(Hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////get (hit)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    flat_imap_get_swar(&map, i);
        //}
        //diff = clock() - start;
        //printlog("\t Get(Hit):   \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////get (miss)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    flat_imap_get_swar(&map, i + NUM_INSERTS);
        //}
        //diff = clock() - start;
        //printlog("\t Get(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

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


        flat_imap_free_swar(&map);
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
        u64 ns = (diff * 1000000000)/CLOCKS_PER_SEC;
        printlog("\t Time: %l ns\n", ns/NUM_INSERTS);

        ////insert (hit)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    imapIns(&map, i, i);
        //}
        //diff = clock() - start;
        //printlog("\t Insert(Hit): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////get (hit)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    imapGet(&map, i);
        //}
        //diff = clock() - start;
        //printlog("\t Get(Hit):   \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

        ////get (miss)
        //start = clock();
        //for (u32 i = 0; i < NUM_INSERTS; i++) {
        //    imapGet(&map, i + NUM_INSERTS);
        //}
        //diff = clock() - start;
        //printlog("\t Get(Miss): \t %l ms\n", (diff * 1000) / CLOCKS_PER_SEC);

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
