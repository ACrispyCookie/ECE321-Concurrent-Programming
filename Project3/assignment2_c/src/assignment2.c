#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

/*
    Enum describing the possible commands
    the master can send to the workers
*/
enum master_command {
    PROCESS_VALUE,
    TERMINATE
};

typedef struct worker_data {
    enum master_command command;
    long long int value_to_process;
} worker_data_t;

/*
    Struct that describes a monitor.

    Fields:
    int id - The id of the worker.
    enum worker_state state - The current state of the worker.
    enum master_command command - The last command the master has send to the worker.
    long long int value_to_process - The value that must be processed by the worker 
                                     to determine if it is a prime number or not.
*/
typedef struct monitor {
    int waiting, working;
    int num_of_workers;
    bool main_waiting, main_waiting_worker;
    worker_data_t *data;

    pthread_mutex_t lock;
    pthread_cond_t main_sleep;
    pthread_cond_t worker_queue;
} monitor_t;


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
    monitor_t *monitor;
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

void set_job(monitor_t *monitor);
worker_data_t get_job(monitor_t *monitor);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of workers>\n", argv[0]);
        return -1;
    }
    worker_data_t data;
    data.command = PROCESS_VALUE;

    //Initialize variables for monitor
    monitor_t monitor;
    const int N = atoi(argv[1]);
    monitor.num_of_workers = N;
    monitor.main_waiting = false;
    monitor.waiting = 0;
    monitor.working = N;
    monitor.data = &data;

    pthread_mutex_init(&(monitor.lock), NULL);
    pthread_cond_init(&(monitor.main_sleep), NULL);
    pthread_cond_init(&(monitor.worker_queue), NULL);

    //Initialize template for threads
    worker_t template;
    template.monitor = &monitor;

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
        data.value_to_process = input;

        //Wake up next worker and wait until it starts working
        printf("Master: Waking up next worker in queue and waiting for him to start working!\n");
        set_job(&monitor);
    }

    //Change command to terminate and start waking workers
    data.command = TERMINATE;
    data.value_to_process = -1;
    for (int i = 0; i < N; i++) {
        //Wake up next worker and wait until it starts terminating
        printf("Master: Waking up next worker in queue and waiting for him to terminate!\n");
        set_job(&monitor);
    }

    //Wait for worker threads to terminate and free all memory
    for (int i = 0; i < N; i++) {
        pthread_join(workers[i]->thread, NULL);
        free(workers[i]);
    }
    free(workers);
    pthread_mutex_destroy(&(monitor.lock));
    pthread_cond_destroy(&(monitor.main_sleep));
    pthread_cond_destroy(&(monitor.worker_queue));
    return 0;
}

void *run_worker(void *arg) {
    worker_t *self = arg;
    monitor_t *monitor = self->monitor;

    while(1) {
        //Wait until the master wakes me up and copy command and value to work on.
        printf("Worker #%ld: Waiting on for a job...\n", self->thread);
        worker_data_t job_data = get_job(monitor);
        printf("Worker #%ld: Got job!\n", self->thread);
        long long int value = job_data.value_to_process;
        enum master_command command = job_data.command;
        
        //Wake up main and then break from the while loop
        if (command == TERMINATE) {
            printf("Worker #%ld: Terminating...\n", self->thread);
            break;
        }

        //Wake up main and then process the given value
        printf("Worker #%ld: Started working!\n", self->thread);
        printf("Worker #%ld: %lld is %sprime\n", self->thread, value, is_prime(value) ? "" : "not ");
    }

    return NULL;
}

worker_data_t get_job(monitor_t *monitor) {
    pthread_mutex_lock(&monitor->lock);
    monitor->working--;
    if (monitor->main_waiting) {
        monitor->main_waiting = false;
        pthread_cond_signal(&monitor->main_sleep);
    } else {
        monitor->waiting++;
        pthread_cond_wait(&monitor->worker_queue, &monitor->lock);
    }

    worker_data_t data;
    data.command = monitor->data->command;
    data.value_to_process = monitor->data->value_to_process;
    monitor->working++;

    if (monitor->main_waiting_worker) {
        monitor->main_waiting_worker = false;
        pthread_cond_signal(&monitor->main_sleep);
    }

    pthread_mutex_unlock(&monitor->lock);
    return data;
}

void set_job(monitor_t *monitor) {
    pthread_mutex_lock(&monitor->lock);
    if (monitor->waiting > 0) {
        printf("Master: There are %d workers waiting\n", monitor->waiting);
        monitor->waiting--;
        pthread_cond_signal(&monitor->worker_queue);
    } else {
        printf("Master: There are no workers waiting, so going to sleep!\n");
        monitor->main_waiting = true;
        pthread_cond_wait(&monitor->main_sleep, &monitor->lock);
    }

    if (monitor->waiting + monitor->working != monitor->num_of_workers) {
        monitor->main_waiting_worker = true;
        pthread_cond_wait(&monitor->main_sleep, &monitor->lock);
    }
    pthread_mutex_unlock(&monitor->lock);
}

int create_workers(worker_t **workers, const int N, worker_t template) {
    for (int i = 0; i < N; i++) {
        worker_t *worker = malloc(sizeof(worker_t));
        if (worker == NULL) 
            return -1;
        workers[i] = worker;
        worker->monitor = template.monitor;
        pthread_create(&(worker->thread), NULL, run_worker, worker);
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