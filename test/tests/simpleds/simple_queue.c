#define CU_IMPL
#define DS_IMPL
#include <core/cutils.h>
#include <ds/ds.h>

#define DefQueueDecl(name, type)                                               \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        u32 first;                                                             \
        type *data;                                                            \
    } name

#define DefQueueImpl(name, type)                                               \
    name name##Create(Allocator a, u32 max_size) {                             \
        name q = {                                                             \
            .a = a,                                                            \
            .cap = max_size,                                                   \
        };                                                                     \
        q.data = Alloc(a, max_size * sizeof(type));                            \
        return q;                                                              \
    }                                                                          \
    void name##Destroy(name *q) {                                              \
        Free(q->a, q->data, q->cap * sizeof(type));                            \
        q->cap = 0;                                                            \
    }                                                                          \
    bool8 name##Empty(name *q) { return q->size == 0; }                        \
    bool8 name##Full(name *q) { return q->size == q->cap; }                    \
    bool8 name##Push(name *q, type item) {                                     \
        if (name##Full(q)) {                                                   \
            return FALSE;                                                      \
        }                                                                      \
        u32 slot = (q->first + q->size) % q->cap;                              \
        q->data[slot] = item;                                                  \
        q->size++;                                                             \
        return TRUE;                                                           \
    }                                                                          \
    bool8 name##Pop(name *q, type *out) {                                      \
        if (name##Empty(q)) {                                                  \
            return FALSE;                                                      \
        }                                                                      \
        *out = q->data[q->first];                                              \
        q->first = (q->first + 1) % q->cap;                                    \
        q->size--;                                                             \
        return TRUE;                                                           \
    }

DefQueueDecl(Queue, int);
DefQueueImpl(Queue, int);

DefQueueDecl(Queuel, u64);
DefQueueImpl(Queuel, u64);

int main() {

    {
        // Basic Queue
        Queue q = QueueCreate(GlobalAllocator, 3);
        printlog("Empty: %d\n", QueueEmpty(&q));
        printlog("Full: %d\n", QueueFull(&q));

        assert(!QueueFull(&q));
        assert(QueueEmpty(&q));

        QueuePush(&q, 0);
        QueuePush(&q, 1);
        QueuePush(&q, 2);

        arrAssert(q.data, ((int[]){0, 1, 2}), q.cap);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        int val;
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

        arrAssert(q.data, ((int[]){3, 4, 2}), q.cap);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        printlog("Empty: %d\n", QueueEmpty(&q));
        printlog("Full: %d\n", QueueFull(&q));

        assert(QueueFull(&q));
        assert(!QueueEmpty(&q));

        QueueDestroy(&q);
    }

    {
        // Basic Queue
        Queuel q = QueuelCreate(GlobalAllocator, 3);
        printlog("Empty: %d\n", QueuelEmpty(&q));
        printlog("Full: %d\n", QueuelFull(&q));

        assert(!QueuelFull(&q));
        assert(QueuelEmpty(&q));

        QueuelPush(&q, 0);
        QueuelPush(&q, 1);
        QueuelPush(&q, 2);

        arrAssert(q.data, ((u64[]){0, 1, 2}), q.cap);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        u64 val;
        QueuelPop(&q, &val);
        printlog("Popped: %d\n", val);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        QueuelPop(&q, &val);
        printlog("Popped: %d\n", val);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        QueuelPush(&q, 3);
        QueuelPush(&q, 4);

        arrAssert(q.data, ((u64[]){3, 4, 2}), q.cap);

        printlog("Queue (%d, %d): ", q.size, q.cap);
        for (u32 i = 0; i < q.size; i++) {
            u32 slot = (q.first + i) % q.cap;
            printlog("(%d, %d) ", slot, q.data[slot]);
        }
        printlog("\n");

        printlog("Empty: %d\n", QueuelEmpty(&q));
        printlog("Full: %d\n", QueuelFull(&q));

        assert(QueuelFull(&q));
        assert(!QueuelEmpty(&q));

        QueuelDestroy(&q);
    }
}
