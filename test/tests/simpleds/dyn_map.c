#define CU_IMPL
#include <core/cutils.h>
#include <ds/ds.h>

DefHashMapDecl(imap, u32, u32);
DefHashMapImpl(imap, u32, u32);

DefStringMapDecl(simap, u32);
DefStringMapImpl(simap, u32);

#define NAME rmap
#define KEY SString
#define VAL u32
#define HASH(k) FNVHash64((u8*)k.data, k.len)
#define EQ(k1, k2) Sstrcmp(k1, k2)
#define ON_INS(m, k, v) k = Sstrdup(GlobalAllocator, k)
#define ON_DEL(m, k, v) Free(GlobalAllocator, k.data, k.len)
#define ON_CLEAR(m, k, v) Free(GlobalAllocator, k.data, k.len)
#include <ds/maps/swiss.h>


int main() {
    imap map = {GlobalAllocator};
    u32* test = imapGet(&map, 3);
    assert(!test);
    imapReserve(&map, 32);

    for (u32 i = 1; i <= 28; i++) { imapIns(&map, 2 * i, i); }
    for (u32 i = 1; i <= 28; i++) {
        u32* val = imapGet(&map, 2 * i);
        assert(val && *val == i);
    }
    imapDel(&map, 10);
    for (u32 i = 1; i <= 28; i++) {
        u32* val = imapGet(&map, 2 * i);
        if (2 * i == 10) {
            assert(!val);
            continue;
        }
        assert(val && *val == i);
    }

    debuglog("map:");
    for (u32 i = 0; i < map.cap; i++) {
        if (map.keys[i].m == -1)
            debuglog("\tempty");
        else
            debuglog("\t(%d %d) %d", map.keys[i].k, map.keys[i].m, map.vals[i]);
    }

    for (u32 i = 1; i <= 28; i++) { imapDel(&map, 2 * i); }
    assert(map.size == 0);

    debuglog("map:");
    for (u32 i = 0; i < map.cap; i++) {
        if (map.keys[i].m == -1)
            debuglog("\tempty");
        else
            debuglog("\t(%d %d) %d", map.keys[i].k, map.keys[i].m, map.vals[i]);
    }

    simap smap = {GlobalAllocator};

    simapIns(&smap, sstring("test"), 1);
    simapIns(&smap, sstring("test"), 2);
    simapIns(&smap, sstring("test1"), 2);
    simapIns(&smap, sstring("test2"), 2);
    simapIns(&smap, sstring("test3"), 2);
    simapIns(&smap, sstring("test4"), 2);
    simapIns(&smap, sstring("test5"), 2);
    simapIns(&smap, sstring("test6"), 2);
    simapIns(&smap, sstring("test7"), 2);
    simapIns(&smap, sstring("test8"), 2);
    simapIns(&smap, sstring("test9"), 2);
    simapIns(&smap, sstring("test10"), 2);
    simapIns(&smap, sstring("static/something"), 3);

    debuglog("smap");
    for (u32 i = 0; i < smap.cap; i++) {
        if (smap.keys[i].m == -1)
            debuglog("\tempty");
        else
            debuglog("\t(%s %d) %d", smap.keys[i].k, smap.keys[i].m,
                     smap.vals[i]);
    }

    assert(*simapGet(&smap, sstring("test")) == 2);
    assert(*simapGet(&smap, sstring("test1")) == 2);
    assert(*simapGet(&smap, sstring("static/something")) == 3);

    simapDel(&smap, sstring("test5"));
    assert(!simapGet(&smap, sstring("test5")));

    simapFree(&smap);

    simap m = {
        GlobalAllocator
    };
    SString strs[] = {
        sstring("t1"),
        sstring("t2"),
        sstring("t3")
    };

    //test hidden validity concerns 
    //This should work
    {
        ScratchArena sc = ScratchArenaGet(NULL);

        for (u32 i = 0; i < ARRAY_SIZE(strs); i++) {
            i8* data = ArenaAlloc(sc.arena, strs[i].len);
            memcpy(data, strs[i].data, strs[i].len);
            simapIns(&m, (SString){strs[i].len, data}, i);
            memset(data, -1, strs[i].len);
        }

        ScratchArenaEnd(sc);
    }

    for (u32 i = 0; i < ARRAY_SIZE(strs); i++) {
        u32* v = simapGet(&m, strs[i]);
        assert(v);
        assert(*v == i);
    }

    //test Robin map
    rmap r = {
        GlobalAllocator
    };


    rmap_ins(&r, sstring("test"), 1);
    rmap_ins(&r, sstring("test"), 2);
    rmap_ins(&r, sstring("test1"), 2);
    rmap_ins(&r, sstring("test2"), 2);
    rmap_ins(&r, sstring("test3"), 2);
    rmap_ins(&r, sstring("test4"), 2);
    rmap_ins(&r, sstring("test5"), 2);
    rmap_ins(&r, sstring("test6"), 2);
    rmap_ins(&r, sstring("test7"), 2);
    rmap_ins(&r, sstring("test8"), 2);
    rmap_ins(&r, sstring("test9"), 2);
    rmap_ins(&r, sstring("test10"), 2);
    rmap_ins(&r, sstring("static/something"), 3);

    assert(*rmap_get(&r, sstring("test")) == 2);
    assert(*rmap_get(&r, sstring("test1")) == 2);
    assert(*rmap_get(&r, sstring("static/something")) == 3);

    rmap_del(&r, sstring("test5"));
    assert(!rmap_get(&r, sstring("test5")));

    rmap_free(&r);


    simapFree(&m);
    imapFree(&map);
}
