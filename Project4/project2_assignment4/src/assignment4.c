#include <stdio.h>
#include "../../assignment2/src/mythreads.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

//Struct passed as argument to both the train and the passenger threads
typedef struct thread_info {
    mythr_t id;
    int max_passengers;
    bool *park_closed;
    int *passengers_waiting;
    mysem_t *mtx;
    mysem_t *train_wait;
    mysem_t *wait_passenger_action;
    mysem_t *passenger_queue;
    mysem_t *ride_queue;
} thread_info_t;

/*
    Creates and starts a new passenger.

    Parameters: 
    thread_info_t *passenger - The argument that will be used by the new passenger.
    thread_info_t template - The standard values to be copied to the passenger parameter.
*/
void create_passenger(thread_info_t *passenger, thread_info_t template);

/*
    Creates and starts a new train.

    Parameters: 
    thread_info_t *train - The argument that will be used by the new train.
    thread_info_t template - The standard values to be copied to the train parameter.
*/
void create_train(thread_info_t *train, thread_info_t template);

/*
    Function run by train threads. Initially waits for the queue
    to be complete, boards all passengers waiting in queue and starts the ride.
    After the ride is done it signals the passengers to get off. If there are
    still enough passengers waiting in queue it starts a new ride.

    Parameters: 
    void *arg - A struct that contains all the variables and semaphores needed for a train.
*/
void train_thread(void *arg);

/*
    Function run by passenger threads. Initially he joins the
    passenger queue and the N-th passenger signals the train to start boarding.
    After the passenger gets boarded he waits until the ride is done and then gets off
    when the train tells him to.

    Parameters: 
    void *arg - A struct that contains all the variables and semaphores needed for a passenger.
*/
void passenger_thread(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of max passengers>\n", argv[0]);
        return -1;
    }

    //Initialize variables for train and passengers
    const int N = atoi(argv[1]);
    int passengers_waiting = 0;
    bool park_closed = false;
    mysem_t mtx, train_wait, wait_passenger_action, passenger_queue, ride_queue;
    mythreads_init();
    mythreads_sem_create(&mtx, 1);
    mythreads_sem_create(&train_wait, 0);
    mythreads_sem_create(&wait_passenger_action, 0);
    mythreads_sem_create(&passenger_queue, 0);
    mythreads_sem_create(&ride_queue, 0);

    //Create a template struct to pass to each thread
    thread_info_t template;
    template.max_passengers = N;
    template.park_closed = &park_closed;
    template.passengers_waiting = &passengers_waiting;
    template.mtx = &mtx;
    template.train_wait = &train_wait;
    template.wait_passenger_action = &wait_passenger_action;
    template.passenger_queue = &passenger_queue;
    template.ride_queue = &ride_queue;

    //Initialize variables for main and create train
    thread_info_t train;
    thread_info_t **passengers = NULL;
    int passenger_count = 0;
    create_train(&train, template);

    while(1) {
        //Read input to create next passenger
        double sleep_time;
        int rc = scanf("%lf", &sleep_time);
        if (rc == EOF)
            break;
        mythreads_sleep(sleep_time);

        //Initialize memory for new passenger
        passenger_count++;
        thread_info_t **new_passengers = realloc(passengers, passenger_count * sizeof(thread_info_t *));
        if (new_passengers == NULL) {
            printf("Error creating new passengers!\n");
            break;
        }
        passengers = new_passengers;
        thread_info_t *passenger = malloc(sizeof(thread_info_t));
        if (passenger == NULL) {
            printf("Error creating new passengers!\n");
            break;
        }
        passengers[passenger_count - 1] = passenger;

        //Create and start new passenger
        create_passenger(passengers[passenger_count - 1], template);
    }

    //Wait for passengers to terminate
    for (int i = 0; i < passenger_count; i++) {
        mythreads_join(&(passengers[i]->id));
        mythreads_destroy(&(passengers[i]->id));
        free(passengers[i]);
    }
    free(passengers);

    //Close train
    park_closed = true;
    if(!mythreads_sem_up(&train_wait)) {
        printf("Missed up while trying to close train!\n");
    }
    mythreads_join(&(train.id));
    mythreads_destroy(&(train.id));

    //Destroy semaphores
    mythreads_sem_destroy(&mtx);
    mythreads_sem_destroy(&train_wait);
    mythreads_sem_destroy(&wait_passenger_action);
    mythreads_sem_destroy(&passenger_queue);
    mythreads_sem_destroy(&ride_queue);
    mythreads_exit();
    
    return 0;
}

void passenger_thread(void *arg) {
    //Setup variables from argument
    thread_info_t *info = (thread_info_t *) arg;
    mythr_t id = info->id;
    int max_passengers = info->max_passengers;
    int *passengers_waiting = info->passengers_waiting;
    mysem_t *mtx = info->mtx;
    mysem_t *passenger_queue = info->passenger_queue;
    mysem_t *ride_queue = info->ride_queue;
    mysem_t *train_wait = info->train_wait;
    mysem_t *wait_passenger_action = info->wait_passenger_action;

    //Increase waiting passengers
    mythreads_sem_down(mtx);
    printf("Passenger %p: New passenger!\n", &id);
    (*passengers_waiting)++;

    //Wake the train up if you need to
    if (*passengers_waiting == max_passengers) {
        printf("Passenger %p: Enough passengers! Waking up train!\n", &id);
        if(!mythreads_sem_up(train_wait)) {
            printf("Passenger %p: Missed up while trying to wake up train!\n", &id);
        }
    }
    if(!mythreads_sem_up(mtx)) {
        printf("Passenger %p: Missed up while trying to up mtx!\n", &id);
    }

    //If we didn't use the wait_passenger_action semaphore the train could
    //wake up multiple passengers that may haven't yet joined the queue.
    //That could result in lost up calls which are avoided by making the
    //train wait for each passenger that it wakes up to actually board.
    mythreads_sem_down(passenger_queue);
    if(!mythreads_sem_up(wait_passenger_action)) {
        printf("Passenger %p: Missed up while trying to acknowledge boarding!\n", &id);
    }

    //You're in the ride!
    printf("Passenger %p: WOOOOOOOOO!\n", &id);
    mythreads_sem_down(ride_queue);

    //The ride is over!
    if(!mythreads_sem_up(wait_passenger_action)) {
        printf("Passenger %p: Missed up while trying to acknowledge get off!\n", &id);
    }
}

void train_thread(void *arg) {
    thread_info_t *info = (thread_info_t *) arg;
    mythr_t id = info->id;
    int max_passengers = info->max_passengers;
    bool *park_closed = info->park_closed;
    int *passengers_waiting = info->passengers_waiting;
    mysem_t *mtx = info->mtx;
    mysem_t *passenger_queue = info->passenger_queue;
    mysem_t *ride_queue = info->ride_queue;
    mysem_t *train_wait = info->train_wait;
    mysem_t *wait_passenger_action = info->wait_passenger_action;

    
    printf("Train %p: Train is now open!\n", &id);
    while (1) {
        //Wait until max_passengers arrive
        mythreads_sem_down(train_wait);
        if(*park_closed)
            break;

        //Wake max_passengers and wait for each one to board
        printf("Train %p: Passengers are ready! Starting the ride!\n", &id);
        for (int i = 0; i < max_passengers; i++) {
            if(!mythreads_sem_up(passenger_queue)) {
                printf("Passenger %p: Missed up while trying to board next passenger!\n", &id);
            }
            mythreads_sem_down(wait_passenger_action);
        }
        
        //Start the ride
        mythreads_sleep(10);
        
        //Ride is over. Wake passengers on board to leave
        printf("Train %p: Ride is done!\n", &id);
        for (int i = 0; i < max_passengers; i++) {
            if(!mythreads_sem_up(ride_queue)) {
                printf("Passenger %p: Missed up while trying to remove on-board passenger!\n", &id);
            }
            mythreads_sem_down(wait_passenger_action);
        }

        //Count how many passengers are left after the ride.
        //If there are enough passengers for a new ride, start a new one.
        mythreads_sem_down(mtx);
        *passengers_waiting -= max_passengers;
        printf("Train %p: There are %d passengers waiting...\n", &id, *passengers_waiting);
        if (*passengers_waiting >= max_passengers) {
            printf("Train %p: Enough passengers to start new ride!\n", &id);
            if(!mythreads_sem_up(train_wait)) {
                printf("Passenger %p: Missed up while trying to start new ride!\n", &id);
            }
        }
        if(!mythreads_sem_up(mtx)) {
            printf("Passenger %p: Missed up while trying to up mtx!\n", &id);
        }
    }
    printf("Train %p: Train is now closed!\n", &id);
}

void create_train(thread_info_t *train, thread_info_t template) {
    //Setup argument for the train thread
    train->max_passengers = template.max_passengers;
    train->park_closed = template.park_closed;
    train->passengers_waiting = template.passengers_waiting;
    train->mtx = template.mtx;
    train->train_wait = template.train_wait;
    train->wait_passenger_action = template.wait_passenger_action;
    train->passenger_queue = template.passenger_queue;
    train->ride_queue = template.ride_queue;

    //Create the train thread
    int rc = mythreads_create(&(train->id), train_thread, train);
    if (rc == -1) 
        printf("Error while creating train thread %p\n", &(train->id));
}

void create_passenger(thread_info_t *passenger, thread_info_t template) {
    //Setup argument for the passenger thread
    passenger->max_passengers = template.max_passengers;
    passenger->park_closed = template.park_closed;
    passenger->passengers_waiting = template.passengers_waiting;
    passenger->mtx = template.mtx;
    passenger->train_wait = template.train_wait;
    passenger->wait_passenger_action = template.wait_passenger_action;
    passenger->passenger_queue = template.passenger_queue;
    passenger->ride_queue = template.ride_queue;

    //Create the passenger thread
    int rc = mythreads_create(&(passenger->id), passenger_thread, passenger);
    if (rc == -1) 
        printf("Error while creating passenger thread %p\n", &(passenger->id));
}