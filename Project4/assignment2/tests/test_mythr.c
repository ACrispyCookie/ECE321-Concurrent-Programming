#include "../src/mythreads.h"
#include <stdio.h>

int counter = 0;

void test(void *) {
    printf("Hi from test thread!\n");
    counter = 5;
    while(counter != 6);
    printf("Second point\n");
    counter = 10;
    printf("Goodbye from test thread!\n");
}

int main(int argc, char *argv[]) {
    if(mythreads_init() == -1) {
        printf("Error while initializing threads!\n");
    }

    mythr_t thr1;
    thr1.id = -1;
    printf("Creating thread...\n");
    // if(mythreads_create(&thr1, test, NULL) == -1) {
    //     printf("Error while creating thread...\n");
    // }
    printf("Created thread!\n");

    while(counter != 5);
    printf("First point\n");
    counter = 6;
    while(counter != 10);
    printf("Thread exited!\n");
    return 0;
}