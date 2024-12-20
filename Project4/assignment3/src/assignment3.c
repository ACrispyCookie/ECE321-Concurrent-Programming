#include "../../assignment2/src/mythreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct thread_data {
    int id;
    mysem_t *mtx, *writers_queue, *readers_queue;
    int *writers_waiting, *readers_waiting;
    int *writers, *readers;
} thread_data_t;


void reader(void *arg);
void writer(void *arg);
thread_data_t *clone(thread_data_t data, int id);

static char file[32] = {'\0'};

int main(int argc, char *argv[]) {
    int slice = atoi(argv[1]);
    mysem_t mtx, writers_queue, readers_queue;
    int writers_waiting = 0, readers_waiting = 0;
    int writers = 0, readers = 0;
    mythreads_init(slice);
    mythreads_sem_create(&mtx, 1);
    mythreads_sem_create(&writers_queue, 0);
    mythreads_sem_create(&readers_queue, 0);

    thread_data_t data;
    data.mtx = &mtx;
    data.writers_queue = &writers_queue;
    data.readers_queue = &readers_queue;
    data.readers_waiting = &readers_waiting;
    data.writers_waiting = &writers_waiting;
    data.readers = &readers;
    data.writers = &writers;
    
    mythr_t *threads = NULL;
    thread_data_t **thread_data = NULL;
    int thread_count = 0;
    while(1) {
        double sleep_time;
        char to_add;
        int ret = mythreads_scanf("%lf %c", &sleep_time, &to_add);
        if (ret == EOF) {
            printf("here\n");
            break;
        }
        mythreads_sleep(sleep_time);

        thread_count++;
        threads = realloc(threads, thread_count * sizeof(mythr_t));
        thread_data = realloc(thread_data, thread_count * sizeof(thread_data_t *));
        thread_data_t *cloned_data = clone(data, thread_count);
        thread_data[thread_count - 1] = cloned_data;
        if (to_add == 'r') mythreads_create(&(threads[thread_count - 1]), reader, cloned_data);
        else mythreads_create(&(threads[thread_count - 1]), writer, cloned_data);
    }

    for (int i = 0; i < thread_count; i++) {
        mythreads_join(&(threads[i]));
        mythreads_destroy(&(threads[i]));
        free(thread_data[i]);
    }
    free(thread_data);
    free(threads);
    
    mythreads_sem_destroy(&mtx);
    mythreads_sem_destroy(&writers_queue);
    mythreads_sem_destroy(&readers_queue);
    mythreads_exit();

    return 0;
}

thread_data_t *clone(thread_data_t data, int id) {
    thread_data_t *clone = (thread_data_t *) malloc(sizeof(thread_data_t));
    clone->mtx = data.mtx;
    clone->readers_queue = data.readers_queue;
    clone->writers_queue = data.writers_queue;
    clone->readers_waiting = data.readers_waiting;
    clone->writers_waiting = data.writers_waiting;
    clone->readers = data.readers;
    clone->writers = data.writers;
    clone->id = id;
    
    return clone;
}

void reader(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;
    int id = data->id;
    mysem_t *mtx = data->mtx;
    mysem_t *readers_queue = data->readers_queue;
    mysem_t *writers_queue = data->writers_queue;
    int *writers_waiting = data->writers_waiting;
    int *readers_waiting = data->readers_waiting;
    int *writers = data->writers; 
    int *readers = data->readers;

    printf("Reader %d: New reader!\n", id);
    mythreads_sem_down(mtx);
    if (*writers_waiting + *writers > 0) {
        (*readers_waiting)++;
        mythreads_sem_up(mtx);
        mythreads_sem_down(readers_queue);
        if (*readers_waiting > 0) {
            (*readers_waiting)--;
            (*readers)++;
            mythreads_sem_up(readers_queue);
        } else {
            mythreads_sem_up(mtx);
        }
    } else {
        (*readers)++;
        mythreads_sem_up(mtx);
    }

    printf("Reader %d: Read the string \"%s\"\n", id, file);
    mythreads_sleep(2);
    printf("Reader %d: Done!\n", id);

    mythreads_sem_down(mtx);
    (*readers)--;
    if (*readers == 0 && *writers_waiting > 0) {
        (*writers_waiting)--;
        (*writers)++;
        mythreads_sem_up(writers_queue);
    }
    mythreads_sem_up(mtx);

}

void writer(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;
    int id = data->id;
    mysem_t *mtx = data->mtx;
    mysem_t *readers_queue = data->readers_queue;
    mysem_t *writers_queue = data->writers_queue;
    int *writers_waiting = data->writers_waiting;
    int *readers_waiting = data->readers_waiting;
    int *writers = data->writers; 
    int *readers = data->readers;

    printf("Writer %d: New writer!\n", id);
    mythreads_sem_down(mtx);
    if (*readers + *writers > 0) {
        (*writers_waiting)++;
        mythreads_sem_up(mtx);
        mythreads_sem_down(writers_queue);
    } else {
        (*writers)++;
        mythreads_sem_up(mtx);
    }

    printf("Writer %d: Writing to the file \"%d\"\n", id, id);
    mythreads_sleep(2);
    sprintf(file, "%d", id);
    printf("Writer %d: Done!\n", id);

    mythreads_sem_down(mtx);
    (*writers)--;
    if (*readers_waiting > 0) {
        (*readers_waiting)--; 
        (*readers)++; 
        mythreads_sem_up(readers_queue);
    } else {
        if (*writers_waiting > 0) {
            (*writers_waiting)--; 
            (*writers)++; 
            mythreads_sem_up(writers_queue);
        }
        mythreads_sem_up(mtx);
    }

}