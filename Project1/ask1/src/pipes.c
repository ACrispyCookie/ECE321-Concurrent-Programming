#include "pipes.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pipe_t **pipes = NULL;
int pipe_count = 0;
int pipe_add(pipe_t *pipe);
pipe_t *pipe_get(int pipe_id);
int pipe_remove(int pipe_id);

int pipe_open(const int size) {
    pipe_t *pipe = malloc(sizeof(pipe_t *));
    if (pipe == NULL)
        return -1;
    pipe->open_to_write = 1;
    pipe->ring_buffer = malloc(sizeof(ring_buffer_t *));
    pipe->ring_buffer->buffer = malloc(size * sizeof(char));
    pipe->ring_buffer->size = size;
    pipe->ring_buffer->read = 0;
    pipe->ring_buffer->write = 0;
    pipe_add(pipe);

    return pipe->id;
}

int pipe_write(int p, char c) {
    const pipe_t *cur_pipe = pipe_get(p);
    if (cur_pipe == NULL || !cur_pipe->open_to_write)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->full) {
        // wait
    }

    cur_buffer->buffer[cur_buffer->write] = c;
    cur_buffer->write = (cur_buffer->write + 1) % cur_buffer->size;
    cur_buffer->full = (cur_buffer->write == cur_buffer->read);

    return 1;
}

int pipe_writeDone(int p) {
    pipe_t *to_write = pipe_get(p);
    if (to_write == NULL)
        return -1;
    
    to_write->open_to_write = 0;

    return 1;
}

int pipe_read(int p, char *c) {
    const pipe_t *cur_pipe = pipe_get(p);
    if (cur_pipe == NULL)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->read == cur_buffer->write && !cur_buffer->full && cur_pipe->open_to_write) {
        // wait
    }

    if (cur_buffer->read == cur_buffer->write && !cur_pipe->open_to_write) {
        pipe_remove(p);
        return 0;
    }

    *c = *(cur_buffer->buffer + cur_buffer->read);
    cur_buffer->read = (cur_buffer->read + 1) % cur_buffer->size;
    cur_buffer->full = 0;
    
    return 1;
}

int pipe_add(pipe_t *pipe) {
    pipe->id = next_id;
    pipe_t **new_pipes = realloc(pipes, ++pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL)
        return -1;
    pipes = new_pipes;
    pipes[pipe_count - 1] = pipe;
    ++next_id;
    return 0;
}

pipe_t *pipe_get(const int pipe_id) {
    if (pipes == NULL)
        return NULL;

    for (int i = 0; i < pipe_count; i++) {
        if (pipes[i] != NULL && pipes[i]->id == pipe_id)
            return pipes[i];
    }
    return NULL;
}

int pipe_remove(const int pipe_id) {
    if (pipes == NULL)
        return 1;

    pipe_t *to_remove = pipe_get(pipe_id);
    free(to_remove->ring_buffer->buffer);
    free(to_remove->ring_buffer);
    free(to_remove);
    pipe_t **new_pipes = realloc(pipes, --pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL)
        return -1;
    pipes = new_pipes;
    return 0;
}

int main(void) {

}