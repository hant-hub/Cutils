#include <core/cutils.h>

static void PrintBlockData(BlockAllocator b) {
    MemBlock* curr = b->head;
    printlog("Block Data:\n");
    while (curr) {
        printlog("\t Block: %l\n", curr->size);
        curr = curr->next;
    }
}

int main() {
    logfile->buffered = FALSE;
    BlockAllocator ba = BlockCreate(GlobalAllocator, 1000 * sizeof(u32));


    u32* ptrs[100] = {0};

    //PrintBlockData(ba);
    for (u32 i = 0; i < 100; i++) {
        ptrs[i] = BlockAlloc(ba, sizeof(u32));
        *ptrs[i] = i;
    }


    //PrintBlockData(ba);
    for (u32 i = 0; i < 100; i++) { assert(*ptrs[i] == i); }

    for (u32 i = 0; i < 100; i++) {
        BlockFree(ba, ptrs[i]);
    }
    //PrintBlockData(ba);

    PrintBlockData(ba);
    u32* a1 = BlockAlloc(ba, sizeof(u32));
    u32* a2 = BlockAlloc(ba, sizeof(u32));

    PrintBlockData(ba);
    BlockFree(ba, a1);
    PrintBlockData(ba);
    BlockFree(ba, a2);
    PrintBlockData(ba);

    //a = Realloc(GlobalAllocator, a, 100 * sizeof(u32), 200 * sizeof(u32));

    //for (u32 i = 100; i < 200; i++) { a[i] = i; }
    //for (u32 i = 0; i < 200; i++) { assert(a[i] == i); }

    //Free(GlobalAllocator, a, 100 * sizeof(u32));
    BlockDestroy(ba);
}
