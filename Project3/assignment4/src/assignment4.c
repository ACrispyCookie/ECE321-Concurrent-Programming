#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

//Struct that defines the monitor used for synchronization
typedef struct monitor {
    bool park_closed;
    bool train_running;
    int passengers_waiting;
    int passengers_onboard;
    int max_passengers;
    pthread_mutex_t lock;
    pthread_cond_t train_wait;
    pthread_cond_t boarding_queue;
    pthread_cond_t ride_queue;
} monitor_t;
 

//Struct passed as argument to both the train and the passenger threads
typedef struct thread_info {
    pthread_t id;
    monitor_t *monitor;
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
    void *arg - A struct that contains all the variables and the monitor needed for a train.

    Returns: Always NULL
*/
void *train_thread(void *arg);

/*
    Function run by passenger threads. Initially he joins the
    passenger queue and the N-th passenger signals the train to start boarding.
    After the passenger gets boarded he waits until the ride is done and then gets off
    when the train tells him to.

    Parameters: 
    void *arg - A struct that contains all the variables and the monitor needed for a passenger.

    Returns: Always NULL
*/
void *passenger_thread(void *arg);

void passenger_board(monitor_t *monitor, pthread_t id);
void passenger_unboard(monitor_t *monitor, pthread_t id);
bool train_fill(monitor_t *monitor, pthread_t id);
void train_empty(monitor_t *monitor, pthread_t id);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of max passengers>\n", argv[0]);
        return -1;
    }

    //Initialize variables for train and passengers
    monitor_t monitor;
    const int N = atoi(argv[1]);
    monitor.passengers_waiting = 0;
    monitor.passengers_onboard = 0;
    monitor.max_passengers = N;
    monitor.park_closed = false;
    monitor.train_running = false;
    pthread_mutex_init(&(monitor.lock), NULL);
    pthread_cond_init(&(monitor.train_wait), NULL);
    pthread_cond_init(&(monitor.boarding_queue), NULL);
    pthread_cond_init(&(monitor.ride_queue), NULL);


    //Create a template struct to pass to each thread
    thread_info_t template;
    template.monitor = &monitor;

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
        sleep(sleep_time);

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
        pthread_join(passengers[i]->id, NULL);
        free(passengers[i]);
    }
    free(passengers);

    //Close train
    monitor.park_closed = true;
    pthread_cond_signal(&monitor.train_wait);
    pthread_join(train.id, NULL);

    //Destroy mutexes and condition variables
    pthread_mutex_destroy(&(monitor.lock));
    pthread_cond_destroy(&(monitor.train_wait));
    pthread_cond_destroy(&(monitor.boarding_queue));
    pthread_cond_destroy(&(monitor.ride_queue));
    
    return 0;
}

void *passenger_thread(void *arg) {
    //Setup variables from argument
    thread_info_t *info = (thread_info_t *) arg;
    pthread_t id = info->id;
    monitor_t *monitor = info->monitor;

    printf("Passenger %ld: New passenger! Boarding...\n", id);
    passenger_board(monitor, id);
    
    //You're in the ride!
    printf("Passenger %ld: WOOOOOOOOO!\n", id);
    
    //Wait until the ride is over to unboard!
    passenger_unboard(monitor, id);
    printf("Passenger %ld: Unboarded!\n", id);

    return NULL;
}

void *train_thread(void *arg) {
    thread_info_t *info = (thread_info_t *) arg;
    pthread_t id = info->id;
    monitor_t *monitor = info->monitor;

    
    printf("Train %ld: Train is now open!\n", id);
    while (1) {
        //Wait for max_passengers to arrive and then wake them up
        printf("Train %ld: Waiting for max passengers and waking them up!\n", id);
        bool closed = train_fill(monitor, id);
        if (closed)
            break;
        
        //Start the ride
        sleep(1);
        
        //Ride is over. Wake passengers on board to leave
        printf("Train %ld: Ride is done! Emptying train...\n", id);
        train_empty(monitor, id);

    }
    printf("Train %ld: Train is now closed!\n", id);
    return NULL;
}

void create_train(thread_info_t *train, thread_info_t template) {
    //Setup argument for the train thread
    train->monitor = template.monitor;

    //Create the train thread
    int rc = pthread_create(&(train->id), NULL, train_thread, train);
    if (rc) 
        printf("Error while creating train thread %ld\n", train->id);
}

void create_passenger(thread_info_t *passenger, thread_info_t template) {
    //Setup argument for the passenger thread
    passenger->monitor = template.monitor;

    //Create the passenger thread
    int rc = pthread_create(&(passenger->id), NULL, passenger_thread, passenger);
    if (rc) 
        printf("Error while creating passenger thread %ld\n", passenger->id);
}

void passenger_board(monitor_t *monitor, pthread_t id) {
    pthread_mutex_lock(&monitor->lock);
    
    //Increase waiting passengers
    monitor->passengers_waiting++;

    //Wait if train is not ready for ride
    if (monitor->train_running || monitor->passengers_waiting != monitor->max_passengers) {
        pthread_cond_wait(&monitor->boarding_queue, &monitor->lock);
        monitor->passengers_onboard++;
    } else {
        printf("Passenger %ld: Enough passengers! Waking up train!\n", id);
        monitor->passengers_onboard++;
        monitor->train_running = true;
        pthread_cond_signal(&monitor->train_wait);
    }

    pthread_mutex_unlock(&monitor->lock);
}

void passenger_unboard(monitor_t *monitor, pthread_t id) {
    pthread_mutex_lock(&monitor->lock);

    if (monitor->train_running)
        pthread_cond_wait(&monitor->ride_queue, &monitor->lock);
    
    // Wake train if it is empty
    monitor->passengers_onboard--;
    if (monitor->passengers_onboard == 0) {
        printf("Passenger %ld: I am last! Waking up train!\n", id);
        pthread_cond_signal(&monitor->train_wait);
    }
    
    pthread_mutex_unlock(&monitor->lock);
}

bool train_fill(monitor_t *monitor, pthread_t id) {
    pthread_mutex_lock(&monitor->lock);
    
    // Check if park is closed (No new passenger will arrive)
    if (monitor->park_closed) {
        pthread_mutex_unlock(&monitor->lock);
        return true;
    }

    //Wait if there aren't enough passengers for the ride
    if (monitor->passengers_waiting < monitor->max_passengers)
        pthread_cond_wait(&monitor->train_wait, &monitor->lock);

    //Wake up N-1 passengers
    for (int i = 0; i < monitor->max_passengers - 1; i++)
        pthread_cond_signal(&monitor->boarding_queue);
    
    pthread_mutex_unlock(&monitor->lock);
    return false;
}

void train_empty(monitor_t *monitor, pthread_t id) {
    pthread_mutex_lock(&monitor->lock);
    
    // Ride is done wake up passengers in the ride
    monitor->train_running = false;
    monitor->passengers_waiting -= monitor->max_passengers;
    for (int i = 0; i < monitor->max_passengers; i++)
        pthread_cond_signal(&monitor->ride_queue);
    
    // Wait until the train is empty
    if (monitor->passengers_onboard != 0)
        pthread_cond_wait(&monitor->train_wait, &monitor->lock);
    
    pthread_mutex_unlock(&monitor->lock);
}