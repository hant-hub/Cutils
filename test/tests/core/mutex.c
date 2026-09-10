#include <core/cutils.h>
#include <ds/ds.h>
#include <pthread.h>
#include <unistd.h>

#include <stdatomic.h>
#include <time.h>

static u32 x = 0;
static atomic_int count = 0;

static Mutex m = {0};
static pthread_barrier_t b = {};
static pthread_mutex_t pm = PTHREAD_MUTEX_INITIALIZER;

void* custom_worker(void* arg) {

    printlog("Thread\n");
    pthread_barrier_wait(&b);
    for (u32 i = 0; i < 5000; i++) {
        MutexLock(&m);
        atomic_fetch_add(&count, 1);
        x = x + 1;
        x = x - 1;
        assert(x == 0);
        MutexUnlock(&m);
    }

    return NULL;
}

void* pthread_worker(void* arg) {

    printlog("Thread\n");
    pthread_barrier_wait(&b);
    for (u32 i = 0; i < 5000; i++) {
        pthread_mutex_lock(&pm);
        atomic_fetch_add(&count, 1);
        x = x + 1;
        x = x - 1;
        assert(x == 0);
        pthread_mutex_unlock(&pm);
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
        pthread_create(&threads[i], NULL, custom_worker, NULL);
    }

    clock_t start = clock();

    pthread_barrier_wait(&b);

    void* ret = NULL;
    for (u32 i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], &ret);
    }

    clock_t diff_custom = clock() - start;


    for (u32 i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, pthread_worker, NULL);
    }

    start = clock();

    pthread_barrier_wait(&b);

    ret = NULL;
    for (u32 i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], &ret);
    }

    clock_t diff_pthread = clock() - start;

    printlog("Custom took: %d ms\n", (diff_custom * 1000)/ CLOCKS_PER_SEC);
    printlog("pthread took: %d ms\n", (diff_pthread * 1000) / CLOCKS_PER_SEC);

    printlog("Var: %d\n", x);
    printlog("Count: %d\n", count);
}
