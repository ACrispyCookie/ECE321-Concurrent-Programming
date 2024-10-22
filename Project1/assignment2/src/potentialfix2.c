#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
    Enum describing the possible commands
    the master can send to the workers
*/
enum master_command {
    WAIT,
    PROCESS_VALUE
};

/*
    Enum describing the possible states
    a worker can be at any time
*/
enum worker_state {
    AVAILABLE,
    WORKING,
    TERMINATING
};


int TERMINATE = 0;
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
    enum worker_state state;
    enum master_command command;
    long long int value_to_process;
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
void create_workers(worker_t **workers, int N);

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
    create_workers(&workers, N);

    while (1) {
        const int read_result = scanf("%lld", &input);
        if (read_result == EOF)
            break;

        // wait until you find an available worker
        int available_worker_id = get_available_worker(workers, N);
        while(available_worker_id == -1) {
            available_worker_id = get_available_worker(workers, N);
        }

        worker_t *worker = workers + available_worker_id;
        // notify worker to process value and wait for it to start processing
        worker->value_to_process = input;
        worker->command = PROCESS_VALUE;
        // wait for the worker to start working
        while (worker->state == AVAILABLE && worker->command == PROCESS_VALUE) {}
    }

    TERMINATE = 1;
    // wait for all workers to terminate
    while(!check_terminated_workers(workers, N)) {}

    free(workers);
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;
    while(1) {
        self->state = AVAILABLE;
        while (self->command == WAIT) {
            if (TERMINATE == 1) {
                self->state = TERMINATING;
                break;
            }
        }

        /*
        Resetting the command of the master before setting the state of the worker
        so that the worker doesn't repeat the processing of the same value
        and the master doesn't get its command overwritten.
        */

        self->state = WORKING;
        self->command = WAIT;
        printf("Worker #%d: %lld is %sprime\n", self->id, self->value_to_process, is_prime(self->value_to_process) ? "" : "not ");
    }
    return NULL;
}

int check_terminated_workers(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].state != TERMINATING)
            return 0;
    }
    return 1;
}

int get_available_worker(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].state == AVAILABLE)
            return i;
    }
    return -1;
}

void create_workers(worker_t **workers, const int N) {
    *workers = malloc(N * sizeof(worker_t));

    pthread_t thread;
    for (int i = 0; i < N; i++) {
        (*workers)[i].id = i;
        (*workers)[i].state = AVAILABLE;
        (*workers)[i].value_to_process = 0;
        (*workers)[i].command = WAIT;
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
