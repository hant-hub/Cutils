#include <core/cutils.h>
#include <ds/ds.h>
#include <pthread.h>
#include <unistd.h>

#include <stdatomic.h>
#include <time.h>

#define SEM_MAX 3

static atomic_int x = 0;
static atomic_int count = 0;

static Semaphore s = {0};
static pthread_barrier_t b = {};

void* custom_worker(void* arg) {
    //writer
    pthread_barrier_wait(&b);
    for (u32 i = 0; i < 5000; i++) {
        SemDown(&s);
        atomic_fetch_add(&count, 1);
        atomic_fetch_add(&x, 1);
        usleep(1000);
        assert(x <= SEM_MAX);  
        atomic_fetch_add(&x, -1);
        SemUp(&s);
    }
    return NULL;
}

#define THREAD_COUNT 8 

int main() {

    logfile->buffered = FALSE;
    logfile->thread_safe = TRUE;

    SemSet(&s, SEM_MAX);

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
