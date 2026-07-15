#include "pipes.h"

#include <pthread.h>
#include <stdlib.h>

static pipe_t **pipes = NULL;
static unsigned int pipe_count = 0;
static pthread_mutex_t pipes_lock = PTHREAD_MUTEX_INITIALIZER;

static int pipe_add(pipe_t *pipe);
static pipe_t *pipe_get(unsigned int pipe_id);
static int pipe_remove_locked(unsigned int pipe_id);

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
    pthread_mutex_init(&pipe->lock, NULL);
    pthread_cond_init(&pipe->can_read, NULL);
    pthread_cond_init(&pipe->can_write, NULL);

    if (pipe_add(pipe) != 0) {
        pthread_cond_destroy(&pipe->can_write);
        pthread_cond_destroy(&pipe->can_read);
        pthread_mutex_destroy(&pipe->lock);
        free(pipe->ring_buffer->buffer);
        free(pipe->ring_buffer);
        free(pipe);
        return (unsigned int)-1;
    }

    return pipe->id;
}

int pipe_write(unsigned int p, char c) {
    pipe_t *cur_pipe = pipe_get(p);
    if (cur_pipe == NULL)
        return -1;

    pthread_mutex_lock(&cur_pipe->lock);
    if (!cur_pipe->open_to_write) {
        pthread_mutex_unlock(&cur_pipe->lock);
        return -1;
    }

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    while (cur_buffer->count == cur_buffer->size && cur_pipe->open_to_write)
        pthread_cond_wait(&cur_pipe->can_write, &cur_pipe->lock);

    if (!cur_pipe->open_to_write) {
        pthread_mutex_unlock(&cur_pipe->lock);
        return -1;
    }

    cur_buffer->buffer[cur_buffer->write] = c;
    cur_buffer->write = (cur_buffer->write + 1) % cur_buffer->size;
    cur_buffer->count++;
    pthread_cond_signal(&cur_pipe->can_read);
    pthread_mutex_unlock(&cur_pipe->lock);
    return 1;
}

int pipe_writeDone(unsigned int p) {
    pipe_t *to_write = pipe_get(p);
    if (to_write == NULL)
        return -1;

    pthread_mutex_lock(&to_write->lock);
    to_write->open_to_write = 0;
    pthread_cond_broadcast(&to_write->can_read);
    pthread_mutex_unlock(&to_write->lock);
    return 1;
}

int pipe_read(unsigned int p, char *c) {
    pipe_t *cur_pipe = pipe_get(p);
    if (cur_pipe == NULL || c == NULL)
        return -1;

    pthread_mutex_lock(&cur_pipe->lock);
    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;

    while (cur_buffer->count == 0 && cur_pipe->open_to_write)
        pthread_cond_wait(&cur_pipe->can_read, &cur_pipe->lock);

    if (cur_buffer->count == 0 && !cur_pipe->open_to_write) {
        pthread_mutex_unlock(&cur_pipe->lock);

        pthread_mutex_lock(&pipes_lock);
        pipe_remove_locked(p);
        pthread_mutex_unlock(&pipes_lock);
        return 0;
    }

    *c = cur_buffer->buffer[cur_buffer->read];
    cur_buffer->read = (cur_buffer->read + 1) % cur_buffer->size;
    cur_buffer->count--;
    pthread_cond_signal(&cur_pipe->can_write);
    pthread_mutex_unlock(&cur_pipe->lock);
    return 1;
}

static int pipe_add(pipe_t *pipe) {
    pthread_mutex_lock(&pipes_lock);
    pipe->id = pipe_count++;
    pipe_t **new_pipes = realloc(pipes, pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL) {
        pipe_count--;
        pthread_mutex_unlock(&pipes_lock);
        return -1;
    }
    pipes = new_pipes;
    pipes[pipe_count - 1] = pipe;
    pthread_mutex_unlock(&pipes_lock);
    return 0;
}

static pipe_t *pipe_get(const unsigned int pipe_id) {
    pthread_mutex_lock(&pipes_lock);
    pipe_t *result = NULL;
    for (unsigned int i = 0; i < pipe_count; i++) {
        if (pipes[i] != NULL && pipes[i]->id == pipe_id) {
            result = pipes[i];
            break;
        }
    }
    pthread_mutex_unlock(&pipes_lock);
    return result;
}

static int pipe_remove_locked(const unsigned int pipe_id) {
    if (pipes == NULL)
        return 1;

    unsigned int pipe_index = pipe_count;
    for (unsigned int i = 0; i < pipe_count; i++) {
        if (pipes[i] != NULL && pipes[i]->id == pipe_id) {
            pipe_index = i;
            break;
        }
    }
    if (pipe_index == pipe_count)
        return 1;

    pipe_t *to_remove = pipes[pipe_index];
    pthread_cond_destroy(&to_remove->can_write);
    pthread_cond_destroy(&to_remove->can_read);
    pthread_mutex_destroy(&to_remove->lock);
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
