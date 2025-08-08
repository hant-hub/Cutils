#define CU_IMPL
#include <cutils.h>


int main() {

    u32* a = Alloc(GlobalAllocator, 100 * sizeof(u32));

    for (u32 i = 0; i < 100; i++) {
        a[i] = i;
    }

    for (u32 i = 0; i < 100; i++) {
        assert(a[i] == i);
    }

    a = Realloc(GlobalAllocator, a, 100 * sizeof(u32), 200 * sizeof(u32));

    for (u32 i = 100; i < 200; i++) {
        a[i] = i;
    }

    for (u32 i = 0; i < 200; i++) {
        assert(a[i] == i);
    }

    Free(GlobalAllocator, a, 100*sizeof(u32));

}
