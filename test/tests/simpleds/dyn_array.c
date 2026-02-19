#define CU_IMPL
#define DS_IMPL
#include <core/cutils.h>
#include <ds/ds.h>

DefDynArrayDecl(IntArr, int);
DefDynArrayImpl(IntArr, int);

int main() {

    IntArr ints = {GlobalAllocator};

    IntArrReserve(&ints, 10);
    assert(ints.cap == 10);

    for (u32 i = 0; i < 10; i++) IntArrPush(&ints, i);
    for (u32 i = 0; i < 10; i++) assert(ints.data[i] == i);

    assert(IntArrBack(&ints) == 9);

    IntArrResize(&ints, 20);
    assert(ints.cap == 20);
    assert(ints.size == 20);

    IntArrIns(&ints, 4, 10);
    assert(ints.data[4] == 10);
    for (u32 i = 5; i < 10; i++) assert(ints.data[i] == i - 1);

    IntArrDel(&ints, 4);
    for (u32 i = 0; i < 10; i++) assert(ints.data[i] == i);

    IntArrResize(&ints, 0);
    IntArrExt(&ints, (int[]){0, 1, 2, 3}, 4);
    for (u32 i = 0; i < 4; i++) assert(ints.data[i] == i);

    IntArrFree(&ints);
}
