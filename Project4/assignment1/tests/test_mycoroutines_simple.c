#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/mycoroutines.h"

void coroutine1(void *arg);

void coroutine2(void *arg);

int main(const int argc, char *argv[]) {

    co_t main, co1, co2;
    mycoroutine_init(&main);

    const int res1 = mycoroutine_create(&co1, coroutine1, &co2);
    if (res1 == -1)
        printf("Failed to create coroutine 1: %d\n", res1);

    const int res2 = mycoroutine_create(&co2, coroutine2, &main);
    if (res2 == -1)
        printf("Failed to create coroutine 2: %d\n", res2);

    mycoroutine_switchto(&co1);
    mycoroutine_destroy(&co1);
    mycoroutine_destroy(&co2);
    mycoroutine_destroy(&main);
    return 0;
}

void coroutine1(void *arg) {
    for (int i = 0; i < 10; i++) {
        printf("1: i = %d\n", i);
    }

    mycoroutine_switchto((co_t *) arg);
}

void coroutine2(void *arg) {
    for (int i = 0; i < 10; i++) {
        printf("2: i = %d\n", i);
    }

    mycoroutine_switchto((co_t *) arg);
}
