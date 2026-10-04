#include <core/cutils.h>

int main() {
    DynArena d = DynArenaCreate(GlobalAllocator, 1);

    int* ints1 = DynArenaAlloc(&d, 10 * sizeof(int));
    for (u32 i = 0; i < 10; i++) { ints1[i] = i; }

    int* ints2 = DynArenaAlloc(&d, 10 * sizeof(int));
    for (u32 i = 0; i < 10; i++) { ints2[i] = i; }

    for (u32 i = 0; i < 10; i++) {
        assert(ints1[i] == i);
        assert(ints2[i] == i);
    }

    DynArenaDestroy(d);
}
