#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../assignment2/src/mythreads.h"

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
    mythr_t thread;
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
    worker_t template - worker_t struct that contains shared variables for all workers.
    Returns:
    1 on success or -1 on fail
*/
int create_workers(worker_t **workers, const int N, worker_t template);

/*
    The function to run on each worker thread.
    Updates the status of the worker and waits to receive
    new commands from the master.

    Parameters: 
    void *arg - a pointer to the worker_t struct that belongs to this thread.
    Returns: 
*/
void run_worker(void *arg);

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

    //Initialize variables for threads
    const int N = atoi(argv[1]);
    mysem_t main_sleep, worker_queue;
    mythreads_init();
    mythreads_sem_create(&main_sleep, 0);
    mythreads_sem_create(&worker_queue, 0);
    enum master_command command = PROCESS_VALUE;
    long long int value_to_process = 0;

    //Initialize template for threads
    worker_t template;
    template.command = &command;
    template.main_sleep = &main_sleep;
    template.worker_queue = &worker_queue;
    template.value_to_process = &value_to_process;

    //Initialize variables for main
    long long int input;
    worker_t **workers = malloc(N * sizeof(worker_t *));
    if(workers == NULL || create_workers(workers, N, template) == -1) {
        printf("Error while creating workers!\n");
    }

    while(1) {
        const int read_result = scanf("%lld", &input);
        if (read_result == EOF)
            break;
        value_to_process = input;

        //Wake up next worker and wait until it starts working
        printf("Master: Waking up next worker in queue!\n");
        int ret = mythreads_sem_up(&worker_queue);
        if (!ret)
            printf("Lost up call on main on worker_queue\n");
        printf("Master: Waiting for the worker to start working...\n");
        mythreads_sem_down(&main_sleep);
    }

    //Change command to terminate and start waking workers
    command = TERMINATE;
    for (int i = 0; i < N; i++) {
        //Wake up next worker and wait until it starts terminating
        printf("Master: Waking up next worker in queue to terminate!\n");
        int ret = mythreads_sem_up(&worker_queue);
        if (!ret)
            printf("Lost up call on main on worker_queue\n");
        printf("Master: Waiting for the worker to start terminating...\n");
        mythreads_sem_down(&main_sleep);
    }

    //Wait for worker threads to terminate and free all memory
    for (int i = 0; i < N; i++) {
        mythreads_join(&(workers[i]->thread));
        mythreads_destroy(&(workers[i]->thread));
        free(workers[i]);
    }
    free(workers);
    mythreads_sem_destroy(&main_sleep);
    mythreads_sem_destroy(&worker_queue);
    mythreads_exit();
    return 0;
}

void run_worker(void *arg) {
    worker_t *self = arg;

    while(1) {
        //Wait until the master wakes me up and copy command and value to work on.
        printf("Worker #%p: Waiting on workers queue...\n", &self->thread);
        mythreads_sem_down(self->worker_queue);
        printf("Worker #%p: Woke up by master!\n", &self->thread);
        long long int value = *self->value_to_process;
        enum master_command command = *self->command;
        
        //Wake up main and then break from the while loop
        if (command  == TERMINATE) {
            printf("Worker #%p: Terminating and woke up master!\n", &self->thread);
            int ret = mythreads_sem_up(self->main_sleep);
            if (!ret)
                printf("Lost up call on main_sleep\n");
            break;
        }

        //Wake up main and then process the given value
        printf("Worker #%p: Started working and woke up master!\n", &self->thread);
        int ret = mythreads_sem_up(self->main_sleep);
        if (!ret)
            printf("Lost up call on main_sleep\n");
        printf("Worker #%p: %lld is %sprime\n", &self->thread, value, is_prime(value) ? "" : "not ");
    }
}

int create_workers(worker_t **workers, const int N, worker_t template) {
    for (int i = 0; i < N; i++) {
        worker_t *worker = malloc(sizeof(worker_t));
        if (worker == NULL) 
            return -1;
        workers[i] = worker;
        worker->command = template.command;
        worker->main_sleep = template.main_sleep;
        worker->value_to_process = template.value_to_process;
        worker->worker_queue = template.worker_queue;
        mythreads_create(&(worker->thread), run_worker, worker);
    }
    return 1;
}

int is_prime(const long long int N) {
    for (long long int i = 2; i < N; i++) {
        if (N % i == 0) {
            return 0;
        }
    }
    return 1;
}