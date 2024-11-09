#include "../../assignment1/src/mysem.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct car_info
{
    pthread_t id;
    int team;
} car_info_t;

int N;
int cars[2] = {0};
int total_cars[2] = {0};
int waiting[2] = {0};
mysem_t mtx[2]; 
mysem_t q[2];
mysem_t bridge_access;

void *car(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of max cars>\n", argv[0]);
        return -1;
    }
    N = atoi(argv[1]);

    (mtx[0]).id = -1; (mtx[1]).id = -1;
    (q[0]).id = -1; (q[1]).id = -1;
    bridge_access.id = -1;

    mysem_init(&mtx[0], 1);
    mysem_init(&mtx[1], 1);
    mysem_init(&(q[0]), 1);
    mysem_init(&(q[1]), 1);
    mysem_init(&bridge_access, 1);

    pthread_t t1, t2, t3, t4, t5;
    int r = 0; int b = 1;
    car_info_t info1; info1.team = r; info1.id = 1;
    car_info_t info2; info2.team = r; info2.id = 2;
    car_info_t info3; info3.team = b; info3.id = 3;
    car_info_t info4; info4.team = b; info4.id = 4;
    car_info_t info5; info5.team = b; info5.id = 5;
    pthread_create(&t1, NULL, car, &info1);
    pthread_create(&t2, NULL, car, &info2);
    //pthread_create(&t3, NULL, car, &info3);
    //pthread_create(&t4, NULL, car, &info4);
    //pthread_create(&t5, NULL, car, &info5);

    // while(1) {
    //     double sleep_time;
    //     char to_add;
    //     scanf("%lf", &sleep_time);
    //     sleep(sleep_time);
    //     scanf("%c", &to_add);
    //     if (to_add == 'r') //add red car
    //     else //add blue car
    // }
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    //pthread_join(t3, NULL);
    //pthread_join(t4, NULL);
    //pthread_join(t5, NULL);
    
    return 0;
}

void *car(void *args) {
    car_info_t *info = (car_info_t *) args;
    int i = info->team;
    
    printf("%ld: Start\n", info->id);
    mysem_down(&q[i]);
    mysem_down(&mtx[i]);
    printf("%ld: Down team\n", info->id); 
    // accessing bridge
    if (cars[i] == 0) {
        printf("%ld: Was first\n", info->id);
        waiting[i] = 1;
        mysem_down(&bridge_access);
        waiting[i] = 0;
        printf("%ld: Got first\n", info->id);
    }
    cars[i]++;
    total_cars[i]++;
    if (cars[i] < N && (total_cars < N || !waiting[!i])) {
        printf("%ld: Up next\n", info->id);
        mysem_up(&q[i]);
    }
    printf("%ld: Up team\n", info->id);
    mysem_up(&mtx[i]);

    printf("%ld: Crossing\n", info->id);
    sleep(3);
    printf("%ld: Done crossing\n", info->id);

    mysem_down(&mtx[i]);
    printf("%ld: Down team leaving\n", info->id);
    cars[i]--;
    if (cars[i] == N - 1 && !waiting[!i]) {
        printf("%ld: Waking next in bridge\n", info->id);
        mysem_up(&q[i]);
    } else if (cars[i] == 0) {
        printf("%ld: Was last\n", info->id);
        mysem_up(&bridge_access);
        if (total_cars[i] >= N) {
            total_cars[i] = 0;
            printf("%ld: Wake first\n", info->id);
            mysem_up(&q[i]);
        }
    }
    printf("%ld: Up team leaving\n", info->id);
    mysem_up(&mtx[i]);

    return NULL;
}