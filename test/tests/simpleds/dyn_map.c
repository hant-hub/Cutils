#define CU_IMPL
#include <core/cutils.h>
#include <ds/ds.h>

DefHashMapDecl(imap, u32, u32);
DefHashMapImpl(imap, u32, u32);

DefStringMapDecl(simap, u32);
DefStringMapImpl(simap, u32);

int main() {
    imap map = {GlobalAllocator};
    u32* test = imapGet(&map, 3);
    imapReserve(&map, 32);

    for (u32 i = 1; i <= 28; i++) { imapIns(&map, 2 * i, i); }
    for (u32 i = 1; i <= 28; i++) {
        u32 *val = imapGet(&map, 2 * i);
        assert(val && *val == i);
    }
    imapDel(&map, 10);
    for (u32 i = 1; i <= 28; i++) {
        u32 *val = imapGet(&map, 2 * i);
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

    simapDel(&smap, sstring("test5"));
    assert(!simapGet(&smap, sstring("test5")));

    simapFree(&smap);
    imapFree(&map);
}
