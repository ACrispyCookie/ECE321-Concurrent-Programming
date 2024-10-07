#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

enum main_signal {
    WAIT,
    PROCESS_VALUE,
    TERMINATE
};

typedef struct main_command {
    enum main_signal signal;
    long long int process_value;
    int id;
} main_to_worker_signal;

typedef enum worker_signal {
    AVAILABLE,
    WORKING,
    TERMINATING
} worker_to_main_signal;

main_to_worker_signal *main_signals;
worker_to_main_signal *worker_signals;

int check_terminated_workers(int N);
int check_available_worker(int N);
int create_workers(int N);
void *run_worker(void *arg);
int is_prime(long long int N);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ask2 <number of workers>\n");
        return -1;
    }
    const int N = atoi(argv[1]);
    long long int input;

    create_workers(N);

    while (1) {
        scanf("%lld", &input);
        if (input == -1)
            break;

        // wait until you find an available worker
        int available_worker = check_available_worker(N);
        while(available_worker == -1) {
            available_worker = check_available_worker(N);
        }

        // notify worker to process value and wait for it to start processing
        main_signals[available_worker].process_value = input;
        main_signals[available_worker].signal = PROCESS_VALUE;
        // edw yparxei periptwsh na parei ton cpu to worker prin prolavei h main na allaksei to notification se wait
        // ara mporei to worker na elegxei to idio value polles fores mexri na parei ksana to cpu h main
        while (worker_signals[available_worker] != WORKING) {}
        main_signals[available_worker].signal = WAIT;
    }

    for (int i = 0; i < N; i++)
        main_signals[i].signal = TERMINATE;
    while(!check_terminated_workers(N)) {}

    free(worker_signals);
    free(main_signals);
    return 0;
}

int check_terminated_workers(const int N) {
    for (int i = 0; i < N; i++) {
        if (worker_signals[i] != TERMINATING)
            return 0;
    }
    return 1;
}

int check_available_worker(const int N) {
    for (int i = 0; i < N; i++) {
        if (worker_signals[i] == AVAILABLE)
            return i;
    }
    return -1;
}

int create_workers(const int N) {
    main_signals = malloc(N * sizeof(main_to_worker_signal));
    worker_signals = malloc(N * sizeof(worker_to_main_signal));

    for (int i = 0; i < N; i++) {
        main_signals[i].signal = WAIT;
        main_signals[i].process_value = 0;
        main_signals[i].id = i;
        pthread_create(NULL, NULL, run_worker, &main_signals[i].id);
    }
    return 0;
}

void *run_worker(void *arg) {
    const int id = *(int *) arg;
    while(1) {
        worker_signals[id] = AVAILABLE;
        while (main_signals[id].signal == WAIT) {}
        if (main_signals[id].signal == TERMINATE) break;
        worker_signals[id] = WORKING;
        printf("%lld is %sprime\n", main_signals[id].process_value, is_prime(main_signals[id].process_value) ? "" : "not ");
    }
    worker_signals[id] = TERMINATING;
    return NULL;
}

int is_prime(const long long int N) {
    for (long long int i = 2; i < N; i++) {
        if (N % i == 0) {
            return 0;
        }
    }
    return 1;
}
