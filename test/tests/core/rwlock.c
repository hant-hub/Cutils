#include <core/cutils.h>
#include <ds/ds.h>
#include <pthread.h>
#include <unistd.h>

#include <stdatomic.h>
#include <time.h>

static u32 x = 0;
static atomic_int count = 0;

static RWLock m = {0};
static pthread_barrier_t b = {};

void* custom_worker(void* arg) {
    u32 tid = (u32)(u64)arg;

    if (tid % 6 == 0) {
        //writer
        pthread_barrier_wait(&b);
        for (u32 i = 0; i < 5000; i++) {
            RWLockWrite(&m);
            atomic_fetch_add(&count, 1);
            x = x + 1;
            x = x - 1;
            assert(x == 0);
            RWUnlockWrite(&m);
        }
    } else {
        //reader
        pthread_barrier_wait(&b);
        for (u32 i = 0; i < 5000; i++) {
            RWLockRead(&m);
            assert(x == 0);
            RWUnlockRead(&m);
        }
    }

    return NULL;
}

#define THREAD_COUNT 8 

int main() {

    logfile->buffered = FALSE;
    logfile->thread_safe = TRUE;

    pthread_barrier_init(&b, NULL, THREAD_COUNT + 1);
    pthread_t threads[THREAD_COUNT] = {0};
    for (u32 i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, custom_worker, (void*)(u64)i);
    }


    clock_t start = clock();
    pthread_barrier_wait(&b);

    void* ret = NULL;
    for (u32 i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], &ret);
    }

    clock_t diff = clock() - start;

    printlog("lock took: %d ms\n", (diff * 1000) / CLOCKS_PER_SEC);

    printlog("Var: %d\n", x);
    printlog("Count: %d\n", count);
}
