#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

enum main_command {
    WAIT,
    PROCESS_VALUE,
    TERMINATE
};

enum worker_state {
    AVAILABLE,
    WORKING,
    TERMINATING
};

typedef struct worker {
    int id;
    enum worker_state state;
    enum main_command command;
    long long int value_to_process;
} worker_t;

int check_terminated_workers(const worker_t *workers, int N);
int check_available_worker(const worker_t *workers, int N);
int create_workers(worker_t **workers, int N);
void *run_worker(void *arg);
int is_prime(long long int N);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ask2 <number of workers>\n");
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
        int available_worker = check_available_worker(workers, N);
        while(available_worker == -1) {
            available_worker = check_available_worker(workers, N);
        }

        // notify worker to process value and wait for it to start processing
        workers[available_worker].value_to_process = input;
        workers[available_worker].command = PROCESS_VALUE;
        // wait for the worker to start working
        while (workers[available_worker].state != WORKING) {}
    }

    for (int i = 0; i < N; i++)
        workers[i].command = TERMINATE;
    // wait for all workers to terminate
    while(!check_terminated_workers(workers, N)) {}

    free(workers);
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;
    while(1) {
        self->state = AVAILABLE;
        while (self->command == WAIT) {}
        if (self->command == TERMINATE) break;
        // reset command before setting the state of the worker
        // so that the master doesn't get his commands overwritten by the worker
        self->command = WAIT;
        self->state = WORKING;
        printf("Worker #%d: %lld is %sprime\n", self->id, self->value_to_process, is_prime(self->value_to_process) ? "" : "not ");
    }
    self->state = TERMINATING;
    return NULL;
}

int check_terminated_workers(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].state != TERMINATING)
            return 0;
    }
    return 1;
}

int check_available_worker(const worker_t *workers, const int N) {
    for (int i = 0; i < N; i++) {
        if (workers[i].state == AVAILABLE)
            return i;
    }
    return -1;
}

int create_workers(worker_t **workers, const int N) {
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
    return 0;
}

int is_prime(const long long int N) {
    for (long long int i = 2; i < N; i++) {
        if (N % i == 0) {
            return 0;
        }
    }
    return 1;
}
