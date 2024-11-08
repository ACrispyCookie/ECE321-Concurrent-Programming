#include <stdio.h>
#include <pthread.h>
#include "../../assignment1/src/mysem.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#define SEAT_NUM 5

void create_train();
void create_passenger();
void *train_thread(void *arg);
void *passenger_thread(void *arg);

typedef struct thread_info {
    pthread_t ptid;
} thread_info_t;

int passenger_count = 0;
int passengers_waiting = 0;
mysem_t wait_train;
mysem_t wait_passengers;
mysem_t mtx;
mysem_t ride;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of >", argv[0]);
        return -1;
    }
    const int N = atoi(argv[1]);

    // initialize semaphores
    mysem_init(&wait_passengers, 0);
    mysem_init(&wait_train, 0);
    mysem_init(&mtx, 1);
    mysem_init(&ride, 0);

    // add train thread
    create_train();

    while(1) {
        double sleep_time;
        scanf("%lf", &sleep_time);
        sleep(sleep_time);
        for (int i=0; i<N; i++)
            create_passenger();
    }
    
    return 0;
}

void create_train() {
    thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));

    int rc = pthread_create(&(info->ptid), NULL, train_thread, info);
    if (!rc) 
        printf("Successfully created train thread %ld\n", info->ptid);
}

void create_passenger() {
    thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));

    int rc = pthread_create(&(info->ptid), NULL, passenger_thread, info);
    if (!rc) 
        printf("Successfully created passenger thread %ld\n", info->ptid);
}

void *train_thread(void *arg) {
    
    while (1) {

        mysem_down(&mtx);
        // N or more passengers waiting
        if (passengers_waiting >= SEAT_NUM) {
            // unblock N of them
            for (int i = 0; i < SEAT_NUM; i++) {
                passengers_waiting--;
                printf("Train woke up passenger %d.\n", i);
                mysem_up(&wait_train);
            }
        }
        // less than N passengers waiting
        else if (passengers_waiting < SEAT_NUM && passengers_waiting > 0) {
            while (passengers_waiting > 0) {
                passengers_waiting--;
                printf("Train woke up a passenger.\n");
                mysem_up(&wait_train);
            }
        }
        mysem_up(&mtx);
        // wait until train is full
        printf("Train waiting for passengers.\n");
        mysem_down(&wait_passengers);

        /***during this code no new passengers should get in train ***/
        // ride
        printf("Train will go for a ride.\n");
        sleep(5);

        // let all passengers get out
        mysem_down(&mtx);
        for (int i=0; i < SEAT_NUM; i++) {
            passenger_count--;
            printf("Passenger %d left train.\n", i);
            mysem_up(&ride);
        }
        mysem_up(&mtx);
        /*******************/
    }

    return NULL;
}

void *passenger_thread(void *arg) {
    thread_info_t *info = (thread_info_t *) arg;
    pthread_t ptid = info->ptid;

    mysem_down(&mtx);
    // train on a ride or currently emptying
    if (passenger_count <= SEAT_NUM) {
        passengers_waiting++;
        mysem_up(&mtx); // ama allaksei cpu kai kanei >2 up to treno xanei up
        printf("Passenger %ld is waiting for train.\n", ptid); 
        mysem_down(&wait_train);
    }

    // mporei na adeiazei to treno kai na 
    // erthei epibaths , ara tha kollhsei edw  
    mysem_down(&mtx);
    // get into train
    passenger_count++;
    if (passenger_count == SEAT_NUM) {
        // let train start
        printf("N-th passenger %ld woke up train.\n", ptid);
        mysem_up(&wait_passengers);
    }
    mysem_up(&mtx);

    // enjoy ride
    printf("Passenger %ld will go for a ride.\n", ptid);
    mysem_down(&ride);

    return NULL;
}