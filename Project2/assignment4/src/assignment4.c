#include <stdio.h>
#include <pthread.h>
#include "../../assignment1/src/mysem.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

void create_train();
void create_passenger();
void *train_thread(void *arg);
void *passenger_thread(void *arg);

typedef struct thread_info {
    pthread_t ptid;
    int max_passengers;
} thread_info_t;

int passengers_onboard = 0;
int passengers_waiting = 0;
mysem_t wait_train;
mysem_t wait_passengers;
mysem_t mtx;
mysem_t mtx2;
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
    create_train(N);

    while(1) {
        double sleep_time;
        scanf("%lf", &sleep_time);
        sleep(sleep_time);
        create_passenger(N);
    }
    
    return 0;
}

void create_train(int max_passengers) {
    thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));
    info->max_passengers = max_passengers;

    int rc = pthread_create(&(info->ptid), NULL, train_thread, info);
    if (!rc) 
        printf("Successfully created train thread %ld\n", info->ptid);
}

void create_passenger(int max_passengers) {
    thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));
    info->max_passengers = max_passengers;

    int rc = pthread_create(&(info->ptid), NULL, passenger_thread, info);
    if (!rc) 
        printf("Successfully created passenger thread %ld\n", info->ptid);
}

void *train_thread(void *arg) {
    thread_info_t *info = (thread_info_t *) arg;
    int max_passengers = info->max_passengers;

    while (1) {

        mysem_down(&mtx);
        // N or more passengers waiting
        if (passengers_waiting >= max_passengers) {
            // unblock N of them
            for (int i = 0; i < max_passengers; i++) {
                passengers_waiting--;
                printf("Train woke up passenger %d.\n", i);
                mysem_up(&wait_train);
            }
        }
        // less than N passengers waiting
        else if (passengers_waiting < max_passengers && passengers_waiting > 0) {
            while (passengers_waiting > 0) {
                passengers_waiting--;
                printf("Train woke up a passenger.\n");
                mysem_up(&wait_train);
            }
        }
        mysem_up(&mtx);
        printf("Train waiting for passengers.\n");
        // wait until train is full
        mysem_down(&wait_passengers);

        /***during this code no new passengers should get in train ***/
        // ride
        printf("Train will go for a ride.\n");
        sleep(5);
        printf("Train finished ride.\n");

        // let all passengers get out
        mysem_down(&mtx);
        for (int i=0; i < max_passengers; i++) {
            passengers_onboard--;
            printf("Train told passenger %d to leave.\n", i);
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
    int max_passengers = info->max_passengers;

    mysem_down(&mtx2);
    mysem_down(&mtx);
    // train on a ride 
    if (passengers_onboard == max_passengers) {
        passengers_waiting++;
        mysem_up(&mtx); // xwris mtx2 race condition edw 
        printf("Passenger %ld is waiting for train.\n", ptid); 
        mysem_down(&wait_train);

        mysem_down(&mtx2);
    }
    else 
        mysem_up(&mtx);

    mysem_down(&mtx);
    // get into train
    passengers_onboard++;
    if (passengers_onboard == max_passengers) {
        // let train start
        printf("%d-th passenger %ld woke up train.\n", max_passengers, ptid);
        mysem_up(&wait_passengers);
    }
    mysem_up(&mtx);

    // enjoy ride
    printf("Passenger %ld is on board train.\n", ptid);
    mysem_down(&ride);

    return NULL;
}