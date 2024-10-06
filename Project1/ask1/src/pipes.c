#include "pipes.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct ring_buffer {
    int size;
    int read;
    int write;
    char *buffer;
} ring_buffer_t;

typedef struct pipe {
    int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
} pipe_t;

typedef struct pipeList {
    int size;
    pipe_t **pipes;
} pipeList_t;

pipeList_t *list = NULL;

int list_init();
pipe_t *list_get(int pipe_id);
int list_add(pipe_t *to_add);
int list_remove(int pipe_id);
int list_destroy();
int list_check_destroy();
int list_next_id();

int pipe_open(const int size) {
    pipe_t *new_pipe = malloc(sizeof(pipe_t));
    new_pipe->ring_buffer = malloc(sizeof(ring_buffer_t *));
    new_pipe->ring_buffer->buffer = malloc(size * sizeof(char));
    new_pipe->ring_buffer->size = size;
    new_pipe->ring_buffer->read = 0;
    new_pipe->ring_buffer->write = 0;
    new_pipe->open_to_write = 1;
    list_add(new_pipe);

    return new_pipe->id;
}

int pipe_write(int p, char c) {
    const pipe_t *to_write = list_get(p);
    if (to_write == NULL)
        return -1;

    return 1;
}

int pipe_writeDone(int p) {
    pipe_t *to_write = list_get(p);
    if (to_write == NULL)
        return -1;

    return ;
}

int pipe_read(int p, char *c) {
    pipe_t *to_write = list_get(p);
    if (to_write == NULL)
        return -1;

    return ;
}

int list_init() {
    list = malloc(sizeof(pipeList_t));
    if (list == NULL) {
        printf("Error while initializing pipe list\n");
        return -1;
    }

    list->size = 0;
    list->pipes = malloc(sizeof(pipe_t **));
    return 0;
}

int list_add(pipe_t *to_add) {
    if (list == NULL)
        list_init();
    pipe_t **new_pipes = realloc(list->pipes, (list->size + 1) * sizeof(pipe_t **));
    if (new_pipes == NULL)
        return -1;
    list->pipes = new_pipes;

    const int size = list->size;
    to_add->id = size;
    list->pipes[size] = to_add;
    ++list->size;
    return 0;
}

int list_remove(const int pipe_id) {
    if (list == NULL)
        return -1;

    pipe_t *to_remove = list->pipes[pipe_id];
    free(to_remove->ring_buffer->buffer);
    free(to_remove->ring_buffer);
    free(to_remove);
    list->pipes[pipe_id] = NULL;
    list_check_destroy();
    return 0;
}

pipe_t *list_get(const int pipe_id) {
    if (list->size <= pipe_id)
        return NULL;
    return list->pipes[pipe_id];
}

int list_check_destroy() {
    for (int i = 0; i < list->size; i++) {
        if (list->pipes[i] != NULL)
            return -1;
    }
    return list_destroy();
}

int list_destroy() {
    free(list->pipes);
    free(list);
    return 0;
}

int main (int argc, char *argv[]) {
    

    return 0;
}