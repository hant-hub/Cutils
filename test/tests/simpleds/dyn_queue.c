#include <core/cutils.h>

#define DefDynQueueDecl(name, type)                                            \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        u32 first;                                                             \
        type* data;                                                            \
    } name;

#define DefDynQueueImpl(name, type)                                            \
    name name##Create(Allocator a) { return (name){.a = a}; }                  \
    void name##Destroy(name* q) {                                              \
        if (!q->data)                                                          \
            return;                                                            \
        Free(q->a, q->data, q->cap);                                           \
    }                                                                          \
    bool8 name##Empty(name* q) { return q->size == 0; }                        \
    bool8 name##Full(name* q) { return q->size >= q->cap; }                    \
    void name##Push(name* q, u32 item) {                                       \
        if (name##Full(q)) {                                                   \
            u32 oldcap = q->cap;                                               \
            q->cap = q->cap ? q->cap * 2 : 1;                                  \
                                                                               \
            u32* new = Alloc(q->a, q->cap * sizeof(q->data[0]));               \
            if (!new) {                                                        \
                q->cap = oldcap;                                               \
                return;                                                        \
            }                                                                  \
                                                                               \
            for (u32 i = 0; i < q->size; i++) {                                \
                new[i] = q->data[(q->first + i) % oldcap];                     \
            }                                                                  \
                                                                               \
            Free(q->a, q->data, oldcap * sizeof(q->data[0]));                  \
            q->data = new;                                                     \
            q->first = 0;                                                      \
        }                                                                      \
                                                                               \
        u32 slot = (q->first + q->size) % q->cap;                              \
        q->data[slot] = item;                                                  \
        q->size++;                                                             \
    }                                                                          \
    bool8 name##Pop(name* q, u32* out) {                                       \
        if (name##Empty(q)) {                                                  \
            return FALSE;                                                      \
        }                                                                      \
        *out = q->data[q->first];                                              \
        q->first = (q->first + 1) % q->cap;                                    \
        q->size--;                                                             \
        return TRUE;                                                           \
    }

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
