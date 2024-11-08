#include <stdio.h>
#include <stdlib.h>
#include "../src/mysem.h"

int n = 0;

void *add(void *arg);
void *subtract(void *arg);
static int N = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of operations>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
#ifdef USE_SEMAPHORE
    mysem_t sem1; sem1.id = -1;
    int ret1 = mysem_init(&sem1, 1);
    if (ret1 == 1) 
        printf("Semaphore successfully initiated to 1\n");
#endif

    pthread_t t1, t2;
    printf("Before threads: %d\n", n);
#ifdef USE_SEMAPHORE
    pthread_create(&t1, NULL, add, &sem1);
    pthread_create(&t2, NULL, subtract, &sem1);
#else
    pthread_create(&t1, NULL, add, NULL);
    pthread_create(&t2, NULL, subtract, NULL);
#endif
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("After threads: %d\n", n);

#ifdef USE_SEMAPHORE
    mysem_destroy(&sem1);
#endif

    return 0;
}

void *add(void *arg) {
#ifdef USE_SEMAPHORE
    mysem_t *sem = arg;
    mysem_down(sem);
#endif
    for (int i = 0; i < N; i++) {
        ++n;
    }
#ifdef USE_SEMAPHORE
    int ret = mysem_up(sem);
    if (!ret)
        printf("Lost up call on main on semaphore\n");
#endif
    return NULL;
}

void *subtract(void *arg) {
#ifdef USE_SEMAPHORE
    mysem_t *sem = arg;
    mysem_down(sem);
#endif
    for (int i = 0; i < N; i++) {
        --n;
    }
#ifdef USE_SEMAPHORE
    int ret = mysem_up(sem);
    if (!ret)
        printf("Lost up call on main on semaphore\n");
#endif
    return NULL;
}