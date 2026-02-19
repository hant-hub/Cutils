#define CU_IMPL
#include <core/cutils.h>
#include <ds/ds.h>


PoolDecl(pool, u32);
PoolImpl(pool, u32);

int main() {

    pool *p = &(pool){
        .a = GlobalAllocator,
    };

    u32 one = poolAlloc(p);
    u32 two = poolAlloc(p);
    u32 three = poolAlloc(p);

    p->slots[one] = 1;
    p->slots[two] = 2;
    p->slots[three] = 3;

    poolFree(p, two);

    debuglog("slots");
    for (u32 i = 0; i < p->cap; i++) { debuglog("\t%d %d", i, p->slots[i]); }

    debuglog("freelist");
    for (u32 i = 0; i < p->freesize; i++) {
        debuglog("\t%d %d", i, p->freelist[i]);
    }

    poolDestroy(p);

    return 0;
}
