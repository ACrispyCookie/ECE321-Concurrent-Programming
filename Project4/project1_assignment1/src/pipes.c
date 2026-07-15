#include "pipes.h"

#include "../../assignment2/src/mythreads.h"
#include <stdlib.h>

static pipe_t **pipes = NULL;
static unsigned int pipe_count = 0;

static int pipe_add(pipe_t *pipe);
static pipe_t *pipe_get(unsigned int pipe_id, int *pipe_index);
static int pipe_remove(unsigned int pipe_id);

unsigned int pipe_open(const int size) {
    if (size <= 0)
        return (unsigned int)-1;

    pipe_t *pipe = malloc(sizeof(pipe_t));
    if (pipe == NULL)
        return (unsigned int)-1;

    pipe->ring_buffer = malloc(sizeof(ring_buffer_t));
    if (pipe->ring_buffer == NULL) {
        free(pipe);
        return (unsigned int)-1;
    }

    pipe->ring_buffer->buffer = malloc((size_t)size * sizeof(char));
    if (pipe->ring_buffer->buffer == NULL) {
        free(pipe->ring_buffer);
        free(pipe);
        return (unsigned int)-1;
    }

    pipe->open_to_write = 1;
    pipe->ring_buffer->size = size;
    pipe->ring_buffer->read = 0;
    pipe->ring_buffer->write = 0;
    pipe->ring_buffer->count = 0;

    if (pipe_add(pipe) != 0) {
        free(pipe->ring_buffer->buffer);
        free(pipe->ring_buffer);
        free(pipe);
        return (unsigned int)-1;
    }

    return pipe->id;
}

int pipe_write(unsigned int p, char c) {
    pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL || !cur_pipe->open_to_write)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->count == cur_buffer->size && cur_pipe->open_to_write)
        mythreads_yield();

    if (!cur_pipe->open_to_write)
        return -1;

    cur_buffer->buffer[cur_buffer->write] = c;
    cur_buffer->write = (cur_buffer->write + 1) % cur_buffer->size;
    cur_buffer->count++;
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
    pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL || c == NULL)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->count == 0 && cur_pipe->open_to_write)
        mythreads_yield();

    if (cur_buffer->count == 0 && !cur_pipe->open_to_write) {
        pipe_remove(p);
        return 0;
    }

    *c = cur_buffer->buffer[cur_buffer->read];
    cur_buffer->read = (cur_buffer->read + 1) % cur_buffer->size;
    cur_buffer->count--;
    return 1;
}

static int pipe_add(pipe_t *pipe) {
    pipe->id = pipe_count++;
    pipe_t **new_pipes = realloc(pipes, pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL) {
        pipe_count--;
        return -1;
    }
    pipes = new_pipes;
    pipes[pipe_count - 1] = pipe;
    return 0;
}

static pipe_t *pipe_get(const unsigned int pipe_id, int *pipe_index) {
    if (pipes == NULL)
        return NULL;

    for (unsigned int i = 0; i < pipe_count; i++) {
        if (pipes[i] != NULL && pipes[i]->id == pipe_id) {
            if (pipe_index != NULL)
                *pipe_index = (int)i;
            return pipes[i];
        }
    }
    return NULL;
}

static int pipe_remove(const unsigned int pipe_id) {
    if (pipes == NULL)
        return 1;

    int pipe_index;
    pipe_t *to_remove = pipe_get(pipe_id, &pipe_index);
    if (to_remove == NULL)
        return 1;

    free(to_remove->ring_buffer->buffer);
    free(to_remove->ring_buffer);
    free(to_remove);

    pipes[pipe_index] = pipes[pipe_count - 1];
    pipe_count--;
    if (pipe_count == 0) {
        free(pipes);
        pipes = NULL;
        return 0;
    }

    pipe_t **new_pipes = realloc(pipes, pipe_count * sizeof(pipe_t *));
    if (new_pipes != NULL)
        pipes = new_pipes;
    return 0;
}
