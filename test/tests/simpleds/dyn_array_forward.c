#define CU_IMPL
#define DS_IMPL
#include <core/cutils.h>
#include <ds/ds.h>

int main() {

    dynArray(u32) ints = {GlobalAllocator};

    for (u32 i = 0; i < 10; i++) { dynPush(ints, i); }
    for (u32 i = 0; i < 10; i++) { assert(ints.data[i] == i); }

    assert(dynBack(ints) == 9);

    dynReserve(ints, 20);
    assert(ints.cap == 20);

    dynResize(ints, 30);
    for (u32 i = 10; i < 30; i++) { assert(ints.data[i] == 0); }
    dynResize(ints, 10);
    for (u32 i = 0; i < 10; i++) { ints.data[i] = -(i + 1); };

    dynIns(ints, 6, 0); 
    {
        u32 expected[] = {-1, -2, -3, -4, -5, -6, 0, -7, -8, -9, -10};
        arrAssert(ints.data, expected, ARRAY_SIZE(expected));
    }

    dynDel(ints, 6);
    {
        u32 expected[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
        arrAssert(ints.data, expected, ARRAY_SIZE(expected));
    }

    u32 vals[] = {1, 2, 3};
    dynExt(ints, vals, 3);
    {
        u32 expected[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, 1, 2, 3};
        arrAssert(ints.data, expected, ARRAY_SIZE(expected));
    }

    debuglog("ints: idx val\n");
    for (u32 i = 0; i < ints.size; i++) {
        debuglog("\t%d\t%d", i, ints.data[i]);
    }



    dynFree(ints);
}
