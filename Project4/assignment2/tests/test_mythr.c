#include "../src/mythreads.h"
#include <stdio.h>

int counter = 0;

void test(void *) {
    printf("Thread started!\n");
    while (counter < 10) {
        printf("Counter is %d\n", counter);
        counter++;
        if (counter == 1)
            mythreads_yield();
        else
            mythreads_sleep(1);
    }
    printf("Thread exiting...\n");
}

int main(int argc, char *argv[]) {
    if(mythreads_init() == -1) {
        printf("Error while initializing threads!\n");
    }

    mythr_t thr1;
    thr1.id = -1;
    printf("Creating thread...\n");
    if(mythreads_create(&thr1, test, NULL) == -1) {
        printf("Error while creating thread...\n");
    }
    printf("Created thread!\n");

    while(counter != 10) {
        //printf("Waiting for counter\n");
    }
    mythreads_sleep(10);
    printf("Thread exited!\n");

    return 0;
}