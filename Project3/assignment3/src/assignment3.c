#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    RED,
    BLUE,
    NOT_ASSIGNED
} color;


typedef struct car_monitor
{
    int N;
    int TOTAL_LIMIT;
    pthread_mutex_t lock;
    pthread_cond_t queue[2];
    color bridge;
    int on_bridge;
    int cars_waiting[2];
    int total_cars[2];
} car_monitor_t;


typedef struct car_info
{
    int id;
    pthread_t thread;
    color team;
    car_monitor_t *monitor;
} car_info_t;


void *car(void *arg);
void create_car(car_info_t **car_array, color team, int id, car_monitor_t *monitor); 
void enter_bridge(int id, color team, car_monitor_t *monitor);
void exit_bridge(int id, color team, car_monitor_t *monitor);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of max cars>\n", argv[0]);
        return -1;
    }

    //Initialize thread variables
    car_monitor_t monitor;
    monitor.N = atoi(argv[1]);
    monitor.TOTAL_LIMIT = monitor.N;
    monitor.bridge = NOT_ASSIGNED;
    monitor.on_bridge = 0;
    monitor.cars_waiting[0] = 0;
    monitor.cars_waiting[1] = 0;
    monitor.total_cars[0] = 0;
    monitor.total_cars[1] = 0;
    pthread_mutex_init(&(monitor.lock), NULL);
    pthread_cond_init(&(monitor.queue[0]), NULL);
    pthread_cond_init(&(monitor.queue[1]), NULL);

    //Initialize main variables
    car_info_t **cars_array = NULL;
    int car_count = 0;

    color r = RED; color b = BLUE;
    while(1) {
        double sleep_time;
        char to_add;
        int ret = scanf("%lf %c", &sleep_time, &to_add);
        if (ret == EOF) 
            break;
        sleep(sleep_time);

        car_count++;
        cars_array = realloc(cars_array, car_count * sizeof(car_info_t *));
        if (to_add == 'r') create_car(cars_array, r, car_count - 1, &monitor);
        else create_car(cars_array, b, car_count - 1, &monitor);
    }

    for (int i = 0; i < car_count; i++) {
        pthread_join(cars_array[i]->thread, NULL);
        free(cars_array[i]);
    }
    free(cars_array);

    pthread_mutex_destroy(&(monitor.lock));
    pthread_cond_destroy(&(monitor.queue[0]));
    pthread_cond_destroy(&(monitor.queue[1]));
    
    return 0;
}

void create_car(car_info_t **cars_array, color team, int id, car_monitor_t *monitor) {
    car_info_t *car_info = malloc(sizeof(car_info_t));
    car_info->id = id;
    car_info->team = team;
    car_info->monitor = monitor;
    cars_array[id] = car_info;

    pthread_create(&(car_info->thread), NULL, car, car_info);
}

void enter_bridge(int id, color team, car_monitor_t *monitor) {
    pthread_mutex_lock(&monitor->lock);
    color bridge = monitor->bridge;
    int N = monitor->N;
    int TOTAL_LIMIT = monitor->TOTAL_LIMIT;
    int *on_bridge = &monitor->on_bridge;
    int *cars_waiting = &monitor->cars_waiting[team];
    int *total_cars = &monitor->total_cars[team];
    pthread_cond_t *queue = &(monitor->queue[team]);

    // If the bridge isn't assigned to any color
    if (bridge == NOT_ASSIGNED) {
        printf("%d, %c: # ENTERING # I was first on bridge! Claiming bridge...\n", id, team ? 'b' : 'r');
        monitor->bridge = team;
    // If the bridge is taken by another color or full (or another car is waking up)
    } else if (bridge != team || *on_bridge == N || *cars_waiting > 0) {
        printf("%d, %c: # ENTERING # Bridge is either full or different color! Waiting...\n", id, team ? 'b' : 'r');
        (*cars_waiting)++;
        pthread_cond_wait(queue, &monitor->lock);
        (*cars_waiting)--;
    }

    // Logistics
    (*on_bridge)++;
    (*total_cars)++;

    int opposite_cars_waiting = monitor->cars_waiting[!team];
    // If another car of my color is waiting and can fit in the bridge without violating the total limit of 
    // cars of a certain color that can pass at once 
    if (*on_bridge < N && *cars_waiting > 0 && (*total_cars < TOTAL_LIMIT || !opposite_cars_waiting)) {
        printf("%d, %c: # ENTERING # Waking another car from my color...\n", id, team ? 'b' : 'r');
        pthread_cond_signal(queue);
    }

    pthread_mutex_unlock(&monitor->lock);
}

void exit_bridge(int id, color team, car_monitor_t *monitor) {
    pthread_mutex_lock(&monitor->lock);
    int TOTAL_LIMIT = monitor->TOTAL_LIMIT;
    int cars_waiting = monitor->cars_waiting[team];
    int opposite_cars_waiting = monitor->cars_waiting[!team];
    int *on_bridge = &monitor->on_bridge;
    int *total_cars = &monitor->total_cars[team];
    pthread_cond_t *queue = &(monitor->queue[team]);
    pthread_cond_t *opposite_queue = &(monitor->queue[!team]);
    
    // Logistics
    (*on_bridge)--;

    // If the limit of total cars of one color hasn't been reached
    if (*total_cars < TOTAL_LIMIT) {
        if (cars_waiting) {
            printf("%d, %c: # EXITING # Total limit not reached. Waking up waiting car of my color.\n", id, team ? 'b' : 'r');
            pthread_cond_signal(queue);
        } else if (*on_bridge == 0) {
            *total_cars = 0;
            if (opposite_cars_waiting) {
                printf("%d, %c: # EXITING # Total limit not reached. I am last on bridge and there are cars on the opposite side.\n", id, team ? 'b' : 'r');
                monitor->bridge = !team;
                pthread_cond_signal(opposite_queue);
            } else {
                printf("%d, %c: # EXITING # Total limit not reached. I am last on bridge and there is no one left.\n", id, team ? 'b' : 'r');
                monitor->bridge = NOT_ASSIGNED;
            }
        }
    } else if (opposite_cars_waiting) {
        if (*on_bridge == 0) {
            printf("%d, %c: # EXITING # Total limit reached. Waking up waiting car of the opposite color.\n", id, team ? 'b' : 'r');
            *total_cars = 0;
            monitor->bridge = !team;
            pthread_cond_signal(opposite_queue);
        }
    } else if (cars_waiting) {
        printf("%d, %c: # EXITING # Total limit reached but there is no one on the other side.\n", id, team ? 'b' : 'r');
        pthread_cond_signal(queue);
    } else if (*on_bridge == 0) {
        printf("%d, %c: # EXITING # Total limit reached but there is no one left.\n", id, team ? 'b' : 'r');
        monitor->bridge = NOT_ASSIGNED;
        *total_cars = 0;
    }

    
    pthread_mutex_unlock(&monitor->lock);
}

void *car(void *arg) {
    car_info_t *info = arg;
    car_monitor_t *monitor = info->monitor;
    color team = info->team;
    int id = info->id;
    char t = info->team ? 'b' : 'r';

    printf("%d, %c: # ENTERING # Trying to access bridge...\n", id, t); 
    enter_bridge(id, team, monitor);
    printf("%d, %c: # ENTERING # Starting to cross the bridge...\n", id, t);
    sleep(5);
    printf("%d, %c: # EXITING # Reached the end of the bridge!\n", id, t);
    exit_bridge(id, team, monitor);
    printf("%d, %c: # EXITING # Exited the bridge!\n", id, t);

    return NULL;
}