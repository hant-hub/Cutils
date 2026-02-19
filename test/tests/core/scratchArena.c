#include <core/cutils.h>

int main() {
    ArenaAllocator a = ArenaCreate(GlobalAllocator, 40 * sizeof(int));

    int* ints1 = ArenaAlloc(&a, 10 * sizeof(int));
    for (u32 i = 0; i < 10; i++) {
        ints1[i] = i;
    }

    int* ints2 = ArenaAlloc(&a, 10  * sizeof(int));
    for (u32 i = 0; i < 10; i++) {
        ints2[i] = i;
    }

    for (u32 i = 0; i < 10; i++) {
        assert(ints1[i] == i);
        assert(ints2[i] == i);
    }

    u64 base = ArenaGetPos(a);
    debuglog("Base: %l", base);
    int* ints3 = ArenaAllocZero(&a, 10 * sizeof(int)); 
    assert(ints3);
    for (u32 i = 0; i < 10; i++) {
        assert(ints3[i] == 0);
    }

    ArenaSetPos(&a, base);
    u64 next = ArenaGetPos(a);
    assert(next == base);
    debuglog("Reset: %l", next);

    ints3 = ArenaAllocZero(&a, 10 * sizeof(int)); 
    assert(ints3);
    for (u32 i = 0; i < 10; i++) {
        assert(ints3[i] == 0);
    }

    ArenaDestroy(a);

    ScratchArena sc = ScratchArenaGet(NULL);

    char* chars = ArenaAlloc(&sc.arena, 10); 
    for (u32 i = 0; i < 10; i++) {
        chars[i] = i;
    }

    for (u32 i = 0; i < 10; i++) {
        assert(chars[i] == i);
    }

    ArenaExtendLast(&sc.arena, 10);

    for (u32 i = 10; i < 20; i++) {
        chars[i] = i;
    }

    for (u32 i = 0; i < 20; i++) {
        assert(chars[i] == i);
    }


    ScratchArenaEnd(sc);
}
