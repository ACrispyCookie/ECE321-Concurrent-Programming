#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../assignment2/src/mythreads.h"

#define N 1000

/*
    The function run by the first thread created that reads the
    original file and writes its contents to the first pipe. After that
    it reads bytes from the second pipe and writes it to the second copy of 
    the original file.

    Parameters:
    void *arg - A pointer to the copier struct.
    Returns:
    NULL every time since the return value is always ignored.
*/
void thr1(void *arg);

int main(const int argc, char *argv[]) {
    mythreads_init();
    mythr_t thrs[N];

    for (int i = 0; i < N; i++) {
        printf("Creating thread %d\n", i);
        mythreads_create(&(thrs[i]), thr1, NULL);
        // printf("Waiting thread %d\n", i);
        // mythreads_join(&thrs[i]);
        // printf("Destroying thread %d\n", i);
        // mythreads_destroy(&thrs[i]);
    }

    for (int i = 0; i < N; i++) {
        printf("Waiting thread %p\n", &(thrs[i]));
        mythreads_join(&(thrs[i]));
        printf("Destroying thread %d\n", i);
        mythreads_destroy(&(thrs[i]));
    }
    mythreads_exit();
    return 0;
}

void thr1(void *arg) {
    printf("Hello from thread!\n");
}
