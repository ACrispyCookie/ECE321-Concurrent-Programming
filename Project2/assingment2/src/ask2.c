#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "../../assingment1/mysem.h"

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
    int id;
    mysem_t *main_sem;
    mysem_t *curr_sem;
    long long int value_to_process;
    bool blocked;
    bool terminate;
} worker_t;

/*
    Checks whether the workers in the given array,
    have all terminated or currently terminating.

    Parameters:
    worker_t *workers - the array of workers to check.
    int N - the size of the array workers.
    Returns:
    1, if all workers have terminated or currently terminating or
    0, if there is at least one worker that hasn't terminated.
*/
int check_terminated_workers(const worker_t *workers, int N);

/*
    Finds an available worker in the given array of workers.

    Parameters:
    worker_t *workers - the array of workers to search.
    int N - the size of the array workers.
    Returns:
    -1, if there isn't an available worker in the array or
    an integer that belongs to the id of the available worker found.
*/
int get_available_worker(const worker_t *workers, int N);

/*
    Creates an array workers with the given
    size and stores it at the given pointer.

    Parameters:
    worker_t **workers - the pointer to the array of workers.
    int N - the size of the array to create.
*/
void create_workers(worker_t **workers, int N, mysem_t **sems);

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

void create_semaphores(mysem_t **sems, const int N, const int value);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of workers>\n", argv[0]);
        return -1;
    }
    const int N = atoi(argv[1]);
    long long int input;

    mysem_t *sems;
    create_semaphores(&sems, N+1, 0);
    worker_t *workers;
    create_workers(&workers, N, &sems);

    while (1) {
        const int read_result = scanf("%lld", &input);
        if (read_result == EOF)
            break;
        
        // down Main sem
        printf("Main: BLOCKED himself. Main sem is %d\n", semctl(sems->id, 0, GETVAL));
        mysem_down(sems);
        int available_workers_id = get_available_worker(workers, N);
        // wake up available worker
        int res = mysem_up(sems + available_workers_id + 1);
        if (!res)
            printf("MAIN: lost up call on sem %d\n", available_workers_id);
        else
            printf("Main: Wake up worker #%d. It's sem is %d\n", available_workers_id, semctl((sems+available_workers_id+1)->id, 0, GETVAL));

        worker_t *worker = workers + available_workers_id;
        worker->value_to_process = input;
    }

    for (int i = 0; i < N; i++)
        workers[i].terminate = true;

    // wake up all workers to terminate
    for (int i = 1; i < N+1; i++) {
        mysem_up(&(sems[i]));
    }
    
    // block yourself untill all workers wake you up 
    for (int i = 1; i < N+1; i++) {
        mysem_down(sems);
    }

    free(workers);
    free(sems);
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;
    while(1) {
        self->blocked = true;
        // wake up Main
        int res = mysem_up(self->main_sem);
        if (!res)
            printf("Worker #%d: lost up call on Main sem\n", self->id);
        else
            printf("Worker #%d: Wake up Main. Main sem is %d\n", self->id, semctl(self->main_sem->id, 0, GETVAL));

        // block your self
        printf("Worker #%d: BLOCKED himself. Curr sem is %d\n", self->id, semctl(self->curr_sem->id, 0, GETVAL));
        mysem_down(self->curr_sem);
        if (self->terminate) 
            break;

        printf("Worker #%d: %lld is %sprime\n", self->id, self->value_to_process, is_prime(self->value_to_process) ? "" : "not ");
    }

    mysem_up(self->main_sem);
    return NULL;
}

int get_available_worker(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].blocked)
            return i;
    }
    return -1;
}

void create_workers(worker_t **workers, const int N, mysem_t **sems) {
    *workers = malloc(N * sizeof(worker_t));

    pthread_t thread;
    for (int i = 0; i < N; i++) {
        (*workers)[i].id = i;
        (*workers)[i].value_to_process = 0;
        (*workers)[i].terminate = false;
        (*workers)[i].blocked = false;
        (*workers)[i].main_sem = *sems;
        (*workers)[i].curr_sem = *sems + (*workers)[i].id + 1;
        const int res = pthread_create(&thread, NULL, run_worker, &(*workers)[i]);
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

void create_semaphores(mysem_t **sems, const int N, const int value) {
    *sems = malloc(N * sizeof(mysem_t));

    for (int i = 0; i < N; i++) {
        (*sems)[i].initialized = 0;
        (*sems)[i].id = semget(IPC_PRIVATE, 1, S_IRWXU);
        mysem_init(&(*sems)[i], value);
    }
}
