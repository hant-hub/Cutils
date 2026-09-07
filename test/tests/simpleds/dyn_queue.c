#include <core/cutils.h>
#include <ds/ds.h>

DefDynQueueDecl(Queue, u32);
DefDynQueueImpl(Queue, u32);

int main() {
    // Basic Queue
    logfile.buffered = FALSE;
    Queue q = QueueCreate(GlobalAllocator);
    printlog("Empty: %d\n", QueueEmpty(&q));
    printlog("Full: %d\n", QueueFull(&q));

    // assert(!QueueFull(&q));
    // assert(QueueEmpty(&q));

    QueuePush(&q, 0);
    QueuePush(&q, 1);
    QueuePush(&q, 2);

    printlog("Queue (%d, %d): ", q.size, q.cap);
    for (u32 i = 0; i < q.size; i++) {
        u32 slot = (q.first + i) % q.cap;
        printlog("(%d, %d) ", slot, q.data[slot]);
    }
    printlog("\n");

    arrAssert(q.data, ((u32[]){0, 1, 2}), 3);

    u32 val;
    QueuePop(&q, &val);
    printlog("Popped: %d\n", val);

    printlog("Queue (%d, %d): ", q.size, q.cap);
    for (u32 i = 0; i < q.size; i++) {
        u32 slot = (q.first + i) % q.cap;
        printlog("(%d, %d) ", slot, q.data[slot]);
    }
    printlog("\n");

    QueuePop(&q, &val);
    printlog("Popped: %d\n", val);

    printlog("Queue (%d, %d): ", q.size, q.cap);
    for (u32 i = 0; i < q.size; i++) {
        u32 slot = (q.first + i) % q.cap;
        printlog("(%d, %d) ", slot, q.data[slot]);
    }
    printlog("\n");

    QueuePush(&q, 3);
    QueuePush(&q, 4);
    QueuePush(&q, 5);
    QueuePush(&q, 6);

    // arrAssert(q.data, ((int[]){3, 4, 2}), 3);

    printlog("Queue (%d, %d): ", q.size, q.cap);
    for (u32 i = 0; i < q.size; i++) {
        u32 slot = (q.first + i) % q.cap;
        printlog("(%d, %d) ", slot, q.data[slot]);
    }
    printlog("\n");

    printlog("Empty: %d\n", QueueEmpty(&q));
    printlog("Full: %d\n", QueueFull(&q));

    // assert(QueueFull(&q));
    // assert(!QueueEmpty(&q));

    QueueDestroy(&q);

    return 0;
}
