#include "pipes.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pipe_t *pipes = NULL;
pipe_t *pipe_new(int size);
pipe_t *pipe_get(int pipe_id);
int pipe_remove(int pipe_id);

int pipe_open(const int size) {
    const pipe_t *initialized_pipe = pipe_new(size);

    return initialized_pipe->id;
}

int pipe_write(int p, char c) {
    const pipe_t *to_write = pipe_get(p);
    if (to_write == NULL)
        return -1;

    return 1;
}

int pipe_writeDone(int p) {
    pipe_t *to_write = pipe_get(p);
    if (to_write == NULL)
        return -1;

    return ;
}

int pipe_read(int p, char *c) {
    pipe_t *to_write = pipe_get(p);
    if (to_write == NULL)
        return -1;

    return ;
}

pipe_t *pipe_new(int size) {
    pipe_t *pipe = malloc(sizeof(pipe_t *));
    pipe->id = next_id;
    pipe->open_to_write = 1;
    pipe->ring_buffer = malloc(sizeof(ring_buffer_t *));
    pipe->ring_buffer->buffer = malloc(size * sizeof(char));
    pipe->ring_buffer->size = size;
    pipe->ring_buffer->read = 0;
    pipe->ring_buffer->write = 0;
    HASH_ADD_INT(pipes, id, pipe);
    ++next_id;
    return pipe;
}

pipe_t *pipe_get(const int pipe_id) {
    pipe_t *to_find;

    HASH_FIND_INT(pipes, &pipe_id, to_find);
    return to_find;
}

int pipe_remove(const int pipe_id) {
    pipe_t *to_remove = pipe_get(pipe_id);
    if (to_remove == NULL)
        return 1;

    HASH_DEL(pipes, to_remove);
    free(to_remove->ring_buffer->buffer);
    free(to_remove->ring_buffer);
    free(to_remove);
    return 0;
}