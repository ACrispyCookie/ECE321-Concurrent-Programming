#include "../../assignment1/src/mysem.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct car_info
{
    int id;
    pthread_t thread;
    int team;
    int N;
    int *cars;
    int *total_cars;
    mysem_t *mtx; 
    mysem_t *q;
    mysem_t *bridge_access;
} car_info_t;


void *car(void *arg);
void create_car(car_info_t **car_array, int team, int N, int id, int *cars, int *total_cars, mysem_t *mtx, mysem_t *q, mysem_t *bridge_access);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of max cars>\n", argv[0]);
        return -1;
    }

    //Initialize thread variables
    int N = atoi(argv[1]);
    int cars[2] = {0};
    int total_cars[2] = {0};
    mysem_t mtx[2]; 
    mysem_t q[2];
    mysem_t bridge_access;

    (mtx[0]).id = -1; (mtx[1]).id = -1;
    (q[0]).id = -1; (q[1]).id = -1;
    bridge_access.id = -1;
    mysem_init(&mtx[0], 1);
    mysem_init(&mtx[1], 1);
    mysem_init(&(q[0]), 1);
    mysem_init(&(q[1]), 1);
    mysem_init(&bridge_access, 1);

    //Initialize main variables
    car_info_t **cars_array = NULL;
    int car_count = 0;

    int r = 0; int b = 1;
    while(1) {
        double sleep_time;
        char to_add;
        int ret = scanf("%lf %c", &sleep_time, &to_add);
        if (ret == EOF) 
            break;
        sleep(sleep_time);

        car_count++;
        cars_array = realloc(cars_array, car_count * sizeof(car_info_t *));
        if (to_add == 'r') create_car(cars_array, r, N, car_count - 1, &cars[r], &total_cars[r], &mtx[r], &q[r], &bridge_access);
        else create_car(cars_array, b, N, car_count - 1, &cars[b], &total_cars[b], &mtx[b], &q[b], &bridge_access);
    }

    for (int i = 0; i < car_count; i++) {
        pthread_join(cars_array[i]->thread, NULL);
        free(cars_array[i]);
    }
    free(cars_array);
    
    mysem_destroy(&mtx[0]);
    mysem_destroy(&mtx[1]);
    mysem_destroy(&(q[0]));
    mysem_destroy(&(q[1]));
    mysem_destroy(&bridge_access);
    
    return 0;
}

void create_car(car_info_t **cars_array, int team, int N, int id, int *cars, int *total_cars, mysem_t *mtx, mysem_t *q, mysem_t *bridge_access) {
    car_info_t *car_info = malloc(sizeof(car_info_t));
    car_info->id = id;
    car_info->bridge_access = bridge_access;
    car_info->q = q;
    car_info->mtx = mtx;
    car_info->cars = cars;
    car_info->total_cars = total_cars;
    car_info->team = team;
    car_info->N = N;
    cars_array[id] = car_info;

    pthread_create(&(car_info->thread), NULL, car, car_info);
}

void *car(void *arg) {
    car_info_t *info = arg;
    char t = info->team ? 'b' : 'r';
    int N = info->N;
    mysem_t *mtx = info->mtx;
    mysem_t *q = info->q;
    mysem_t *bridge_access = info->bridge_access;
    int *cars = info->cars;
    int *total_cars = info->total_cars;

    
    mysem_down(q);
    mysem_down(mtx);
    printf("%d, %c: Trying to access bridge...\n", info->id, t); 
    //Accessing bridge
    if (*cars == 0) {
        printf("%d, %c: I was first from my team!\n", info->id, t);
        mysem_down(bridge_access);
        printf("%d, %c: Got access to the bridge\n", info->id, t);
    }
    (*cars)++;
    (*total_cars)++;
    if (*cars < N && *total_cars < N) {
        printf("%d, %c: Waking my car behind me!\n", info->id, t);
        if(!mysem_up(q)) {
            printf("Missed an up while waking the next passenger up!\n");
        }
    }
    if(!mysem_up(mtx)) {
        printf("Missed an up on mtx!\n");   
    }

    printf("%d, %c: Starting to cross the bridge...\n", info->id, t);
    sleep(5);
    printf("%d, %c: Reached the end of the bridge!\n", info->id, t);

    mysem_down(mtx);
    (*cars)--;
    if ((*cars) == 0) {
        printf("%d, %c: I am the last car, opening the bridge to everyone!\n", info->id, t);
        if(!mysem_up(bridge_access)) {
            printf("Missed an up while trying to free the bridge!\n");
        };
        if ((*total_cars) >= N) {
            printf("%d, %c: Waking first one on my team!\n", info->id, t);
            (*total_cars) = 0;
            if(!mysem_up(q)) {
                printf("Missed an up while waking the first passenger up!\n");
            }
        }
    }
    if(!mysem_up(mtx)) {
        printf("Missed an up on mtx!\n");   
    }

    return NULL;
}