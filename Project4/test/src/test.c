#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../assignment2/src/mythreads.h"

#define N 500

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
    // mythr_t *thr = (mythr_t *) malloc(N * sizeof(mythr_t));
    mythr_t thr[N];

    for (int i = 0; i < N; i++) {
        printf("Creating thread %p\n", &thr[i]);
        mythreads_create(&thr[i], thr1, &thr[i]);
        int j = 0;
        while (j < 10000000)
            j++;
    }

    for (int i = 0; i < N; i++) {
        printf("Waiting thread %p\n", &thr[i]);
        mythreads_join(&thr[i]);
        printf("Destroying thread %p\n", &thr[i]);
        mythreads_destroy(&thr[i]);
    }
    // free(thr);
    mythreads_exit();
    return 0;
}

void thr1(void *arg) {
    printf("Hello from thread %p!\n", arg);
}
