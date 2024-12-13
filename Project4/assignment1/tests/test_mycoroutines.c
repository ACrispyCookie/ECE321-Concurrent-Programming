#include <stdio.h>
#include <stdlib.h>
#include "../src/mycoroutines.h"


void add(void *arg);
void subtract(void *arg);
static int N = 0;
static int n = 0;

int main(int argc, char *argv[]) {
    co_t main, t1, t2;
    main.id = -1; t1.id = -1; t2.id = -1;

    if (argc != 2) {
        printf("Usage: %s <number of operations>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    printf("Before threads: %d\n", n);

    mycoroutine_init(&main);
    mycoroutine_create(&t2, subtract, &main);
    mycoroutine_create(&t1, add, &t2);
    mycoroutine_switchto(&t1);

    //Returns
    mycoroutine_destroy(&main);
    mycoroutine_destroy(&t1);
    mycoroutine_destroy(&t2);
    printf("After threads: %d\n", n);
    return 0;
}

void add(void *arg) {
    for (int i = 0; i < N; i++) {
        ++n;
    }
    mycoroutine_switchto(arg);
}

void subtract(void *arg) {
    for (int i = 0; i < N; i++) {
        --n;
    }
    mycoroutine_switchto(arg);
}