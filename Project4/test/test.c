#include <stdio.h>
#include "../assignment2/src/mythreads.h"

static int counter = 0;

void thr(void *arg) {
    printf("thread start\n");
    for (; counter < 10000; counter++);
}

int main(int argc, char *argv[]) {

    mythreads_init();
    
    mythr_t thread;
    mythreads_create(&thread, thr, NULL);
    while(counter < 10000);
    printf("done\n");
    mythreads_destroy(&thread);

    mythreads_exit();

    return 0;
}