#include "pipes.h"

#include <io.h>
#include <pthread.h>
#include <stdlib.h>

pipe_t **pipes = NULL;
int pipe_count = 0;
unsigned int next_id = 0;
int pipe_add(pipe_t *pipe);
pipe_t *pipe_get(unsigned int pipe_id, int *pipe_index);
int pipe_remove(unsigned int pipe_id);

unsigned int pipe_open(const int size) {
    pipe_t *pipe = malloc(sizeof(pipe_t));
    if (pipe == NULL)
        return -1;
    pipe->open_to_write = 1;
    pipe->ring_buffer = malloc(sizeof(ring_buffer_t));
    pipe->ring_buffer->buffer = malloc(size * sizeof(char));
    pipe->ring_buffer->size = size;
    pipe->ring_buffer->read = 0;
    pipe->ring_buffer->write = 0;
    pipe->ring_buffer->full = 0;
    pipe_add(pipe);

    return pipe->id;
}

int pipe_write(unsigned int p, char c) {
    const pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL || !cur_pipe->open_to_write)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->full) {
        // wait
    }

    cur_buffer->buffer[cur_buffer->write] = c;
    cur_buffer->write = (cur_buffer->write + 1) % cur_buffer->size;
    cur_buffer->full = cur_buffer->write == cur_buffer->read;

    return 1;
}

int pipe_writeDone(unsigned int p) {
    pipe_t *to_write = pipe_get(p, NULL);
    if (to_write == NULL)
        return -1;

    to_write->open_to_write = 0;

    return 1;
}

int pipe_read(unsigned int p, char *c) {
    const pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    // if there is nothing to read on the pipe and the pipe is open for writing
    while (cur_buffer->read == cur_buffer->write && !cur_buffer->full && cur_pipe->open_to_write) {
        // wait
    }

    // if the pipe is closed for writing and there is still nothing to read
    if (!cur_pipe->open_to_write && cur_buffer->read == cur_buffer->write && !cur_buffer->full) {
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

pipe_t *pipe_get(const unsigned int pipe_id, int *pipe_index) {
    if (pipes == NULL)
        return NULL;

    for (int i = 0; i < pipe_count; i++) {
        if (pipes[i] != NULL && pipes[i]->id == pipe_id) {
            if (pipe_index != NULL)
                *pipe_index = i;
            return pipes[i];
        }
    }
    return NULL;
}

int pipe_remove(const unsigned int pipe_id) {
    if (pipes == NULL)
        return 1;

    int pipe_index;
    pipe_t *to_remove = pipe_get(pipe_id, &pipe_index);
    free(to_remove->ring_buffer->buffer);
    free(to_remove->ring_buffer);
    free(to_remove);
    pipes[pipe_index] = pipes[pipe_count - 1];
    pipe_t **new_pipes = realloc(pipes, --pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL)
        return -1;
    pipes = new_pipes;
    return 0;
}