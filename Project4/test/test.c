#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "../assignment2/src/mythreads.h"

void thr(void *arg) {
    printf("thread\n");
}

int main(int argc, char *argv[]) {

    int timeout = atoi(argv[1]);
    mythreads_init(timeout);
    
    char buf;
    mythr_t thread;
    mythreads_create(&thread, thr, NULL);
    int ret = scanf("%c", &buf);
    printf("%c %d %d", buf, ret, EOF);
    mythreads_join(&thread);
    mythreads_destroy(&thread);

    mythreads_exit();

    return 0;
}