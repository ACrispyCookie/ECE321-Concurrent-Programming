#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../assignment1/src/mysem.h"
#include <pthread.h>

/*
    Enum describing the possible commands
    the master can send to the workers
*/
enum master_command {
    PROCESS_VALUE,
    TERMINATE
};

/*
    Struct that describes a worker.

    Fields:
    int id - The id of the worker.
    enum worker_state state - The current state of the worker.
    enum master_command command - The last command the master has send to the worker.
    long long int value_to_process - The value that must be processed by the worker 
                                     to determine if it is a prime number or not.
*/
typedef struct worker {
    pthread_t thread;
    mysem_t *worker_queue;
    mysem_t *main_sleep;
    enum master_command *command;
    long long int *value_to_process;
} worker_t;

/*
    Creates an array workers with the given
    size and stores it at the given pointer.

    Parameters:
    worker_t **workers - the pointer to the array of workers.
    int N - the size of the array to create.
*/
void create_workers(worker_t **workers, const int N, mysem_t *worker_queue, mysem_t *main_sleep, enum master_command *command, long long int *value_to_process);

/*
    The function to run on each worker thread.
    Updates the status of the worker and waits to receive
    new commands from the master.

    Parameters: 
    void *arg - a pointer to the worker_t struct that belongs to this thread.
    Returns: 
    NULL every time since the return value is always ignored.
*/
void *run_worker(void *arg);

/*
    Checks if a number is prime using
    a very naive approach.

    Parameters: 
    long long int N - the number to check
    Returns:
    1, if the number N is prime.
    0, if the number N is not prime.
*/
int is_prime(long long int N);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of workers>\n", argv[0]);
        return -1;
    }
    const int N = atoi(argv[1]);
    long long int input;

    worker_t *workers;
    mysem_t main_sleep, worker_queue;
    main_sleep.id = -1; worker_queue.id = -1;
    mysem_init(&main_sleep, 0);
    mysem_init(&worker_queue, 0);
    enum master_command command = PROCESS_VALUE;
    long long int value_to_process = 0;

    create_workers(&workers, N, &worker_queue, &main_sleep, &command, &value_to_process);

    while(1) {
        const int read_result = scanf("%lld", &input);
        if (read_result == EOF)
            break;
        value_to_process = input;

        printf("Main woke up next worker.\n");
        /* tell next worker to start */
        int ret = mysem_up(&worker_queue);
        if (!ret)
            printf("Lost up call on main on worker_queue\n");
        /* Wait worker to start working */
        printf("Main waiting for worker to start.\n");
        mysem_down(&main_sleep);
    }

    command = TERMINATE;
    for (int i = 0; i < N; i++) {
        printf("Main woke up next worker.\n");
        /* tell next worker to start */
        int ret = mysem_up(&worker_queue);
        if (!ret)
            printf("Lost up call on main on worker_queue\n");
        /* wait for worker to indicate termination */
        printf("Main waiting for worker to terminate.\n");
        mysem_down(&main_sleep);
    }

    /* Free workers memory and destroy semaphores */
    free(workers);
    mysem_destroy(&main_sleep);
    mysem_destroy(&worker_queue);
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;

    while(1) {
        printf("Worker #%ld waiting main.\n", self->thread);
        /* wait main to wake you up */
        mysem_down(self->worker_queue);
        printf("Worker #%ld woke up.\n", self->thread);
        long long int value = *self->value_to_process;
        enum master_command command = *self->command;
        
        if (command  == TERMINATE) {
            printf("Worker #%ld: terminating...\n", self->thread);
            /* tell main that you are terminating */
            int ret = mysem_up(self->main_sleep);
            if (!ret)
                printf("Lost up call on main on main_sleep\n");
            break;
        }

        printf("Worker #%ld started working.\n", self->thread);
        /* tell main you are starting work */
        int ret = mysem_up(self->main_sleep);
        if (!ret)
            printf("Lost up call on main on main_sleep\n");
        printf("Worker #%ld: %lld is %sprime\n", self->thread, value, is_prime(value) ? "" : "not ");
    }

    return NULL;
}

void create_workers(worker_t **workers, const int N, mysem_t *worker_queue, mysem_t *main_sleep, enum master_command *command, long long int *value_to_process) {
    *workers = malloc(N * sizeof(worker_t));

    for (int i = 0; i < N; i++) {
        (*workers)[i].worker_queue = worker_queue;
        (*workers)[i].main_sleep = main_sleep;
        (*workers)[i].command = command;
        (*workers)[i].value_to_process = value_to_process;
        const int res = pthread_create(&((*workers)[i].thread), NULL, run_worker, &(*workers)[i]);
        if (res)
            printf("Failed to create worker %d: %d", i, res);
    }
}

int is_prime(const long long int N) {
    for (long long int i = 2; i < N; i++) {
        if (N % i == 0) {
            return 0;
        }
    }
    return 1;
}