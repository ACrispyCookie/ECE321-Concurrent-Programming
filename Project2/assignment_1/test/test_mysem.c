#include <stdio.h>
#include "../src/mysem.h"

#define N 1000000
int n = 0;

void *add_one(void *arg);
void *subtract_one(void *arg);

int main(int argc, char *argv[]) {
    mysem_t sem1, sem2, sem3;
    sem1.id = -1;
    sem2.id = -1;
    sem3.id = -1;
    int ret1 = mysem_init(&sem1, 1);
    int ret2 = mysem_init(&sem2, 0);
    int ret3 = mysem_init(&sem3, 0);

    pthread_t t1, t2;
    printf("%d, %d, %d\n", ret1, ret2, ret3);
    printf("before: %d\n", n);
    pthread_create(&t1, NULL, add_one, &sem1);
    pthread_create(&t2, NULL, subtract_one, &sem1);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("after: %d\n", n);

    mysem_destroy(&sem1);
    mysem_destroy(&sem2);
    mysem_destroy(&sem3);

    return 0;
}

void *add_one(void *arg) {
    mysem_t *sem = arg;
    mysem_down(sem);
    for (int i = 0; i < N; i++) {
        ++n;
    }
    mysem_up(sem);
    return NULL;
}

void *subtract_one(void *arg) {
    mysem_t *sem = arg;
    mysem_down(sem);
    for (int i = 0; i < N; i++) {
        --n;
    }
    mysem_up(sem);
    return NULL;
}