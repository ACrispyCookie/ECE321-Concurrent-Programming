#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../assignment_1/src/mysem.h"
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
    int id;
    mysem_t worker_queue;
    mysem_t m_wait_available;
    mysem_t w_wait_command;
    bool waiting_for_command;
    enum master_command command;
    long long int value_to_process;
} worker_t;

/*
    Creates an array workers with the given
    size and stores it at the given pointer.

    Parameters:
    worker_t **workers - the pointer to the array of workers.
    int N - the size of the array to create.
*/
void create_workers(worker_t **workers, const int N, const mysem_t worker_queue, const mysem_t m_wait_available, const mysem_t w_wait_command);

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
    mysem_t m_wait_available, w_wait_command, worker_queue;
    m_wait_available.id = -1; w_wait_command.id = -1; worker_queue.id = -1;
    mysem_init(&m_wait_available, 0);
    mysem_init(&worker_queue, 0);
    mysem_init(&w_wait_command, 0);
    create_workers(&workers, N, worker_queue, m_wait_available, w_wait_command);

    while (1) {
        const int read_result = scanf("%lld", &input);
        if (read_result == EOF)
            break;

        // wait until you find an available worker
        mysem_up(&worker_queue);
        mysem_down(&m_wait_available);
        // notify worker to process value and wait for it to start processing
        int available_worker_id = get_available_worker(workers, N);
        worker_t *available_worker = workers + available_worker_id;
        available_worker->value_to_process = input;
        available_worker->command = PROCESS_VALUE;
        mysem_up(&w_wait_command);
    }

    for (int i = 0; i < N; i++) {
        mysem_up(&worker_queue);
        mysem_down(&m_wait_available);
        workers[i].command = TERMINATE;
        mysem_up(&w_wait_command);
    }

    free(workers);
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;
    while(1) {
        self->waiting_for_command = false;
        mysem_down(&self->worker_queue);
        self->waiting_for_command = true;
        mysem_up(&self->m_wait_available);
        mysem_down(&self->w_wait_command);
        
        if (self->command == TERMINATE)
            break;
        printf("Worker #%d: %lld is %sprime\n", self->id, self->value_to_process, is_prime(self->value_to_process) ? "" : "not ");
    }
    return NULL;
}

int get_available_worker(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].waiting_for_command)
            return i;
    }
    return -1;
}

void create_workers(worker_t **workers, const int N, const mysem_t worker_queue, const mysem_t m_wait_available, const mysem_t w_wait_command) {
    *workers = malloc(N * sizeof(worker_t));

    pthread_t thread;
    for (int i = 0; i < N; i++) {
        (*workers)[i].id = i;
        (*workers)[i].worker_queue = worker_queue;
        (*workers)[i].m_wait_available = m_wait_available;
        (*workers)[i].w_wait_command = w_wait_command;
        (*workers)[i].value_to_process = 0;
        (*workers)[i].waiting_for_command = false;
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