#include <cutils.h>


int main() {

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
        u32 base = st->size;
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

        StackSet(st, base);
        u32* test3 = StackAlloc(st, 100 * sizeof(u32));

        for (u32 i = 0; i < 100; i++) {
            test3[i] = i;
        }

        for (u32 i = 0; i < 100; i++) {
            assert(test3[i] == i);
        }

    }

    StackDestroy(GlobalAllocator, st);
}
