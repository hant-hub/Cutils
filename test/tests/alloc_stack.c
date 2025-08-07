#include <cutils.h>


int main() {

    //abstract
    Allocator a = StackAllocatorCreate(GlobalAllocator, 200 * sizeof(u32));

    {
        u32* test = Alloc(a, 100 * sizeof(u32));
        u32* test2 = Alloc(a, 100 * sizeof(u32));

        for (u32 i = 0; i < 100; i++) {
            test[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            test2[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            assert(test[i] == i);
        }
        for (u32 i = 0; i < 100; i++) {
            assert(test2[i] == i);
        }
    }

    StackAllocatorReset(a);

    {
        u32* test = Alloc(a, 100 * sizeof(u32));
        u32* test2 = Alloc(a, 100 * sizeof(u32));

        for (u32 i = 0; i < 100; i++) {
            test[i] = 100 - i;
        }

        for (u32 i = 0; i < 100; i++) {
            test2[i] = 100 - i;
        }

        for (u32 i = 0; i < 100; i++) {
            assert(test[i] == 100 - i);
        }
        for (u32 i = 0; i < 100; i++) {
            assert(test2[i] == 100 - i);
        }
    }

    StackAllocatorFree(GlobalAllocator, a);


    //concrete
    StackAllocator st = StackCreate(GlobalAllocator, 200 * sizeof(u32));

    {
        u32* test = StackAlloc(st, 100 * sizeof(u32));
        u32* test2 = StackAlloc(st, 100 * sizeof(u32));

        for (u32 i = 0; i < 100; i++) {
            test[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            test2[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            assert(test[i] == i);
        }
        for (u32 i = 0; i < 100; i++) {
            assert(test2[i] == i);
        }
    }

    StackReset(st);

    {
        u32* test = StackAlloc(st, 100 * sizeof(u32));
        u32* test2 = StackAlloc(st, 100 * sizeof(u32));

        for (u32 i = 0; i < 100; i++) {
            test[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            test2[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            assert(test[i] == i);
        }
        for (u32 i = 0; i < 100; i++) {
            assert(test2[i] == i);
        }
    }

    StackDestroy(GlobalAllocator, st);
}
