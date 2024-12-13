#include "pipes.h"

#include <stdlib.h>
#include <stdio.h>

// Pipe array
static pipe_t **pipes = NULL;
static unsigned int pipe_count = 0;

/*
    Internal function to add a new pipe_t to the 
    pipe array and assign an id to it.

    Parameters:
    pipe_t *pipe - A pointer to the pipe to be added.
    Returns:
    0 for success or 
    -1 if it failed to add the pipe to the array.
*/
int pipe_add(pipe_t *pipe);

/*
    Internal function to get a pipe using its id
    by searching inside the pipe array.

    Parameters:
    unsigned int pipe_id - The id of the pipe to look for in the array.
    int *pipe_index - Optional parameter to get the index of the pipe, if found.
    Returns:
    A pointer to the pipe_t struct with the matching pipe_id.
*/
pipe_t *pipe_get(unsigned int pipe_id, int *pipe_index);

/*
    Internal function to remove a pipe with
    the given id from the pipe array.

    Parameters:
    unsigned int pipe_id - The id of the pipe to remove from the array.
    Returns:
    0 for success, 
    -1 if it failed to remove the pipe from the array or 
    1 if a pipe with the given id doesn't exist in the array.
*/
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
    pipe->ring_buffer->write = 1;
    pipe_add(pipe);

    return pipe->id;
}

int pipe_write(unsigned int p, char c, co_t *read_thread) {
    const pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL || !cur_pipe->open_to_write)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    //if the write pointer is right behind the read pointer
    if ((cur_buffer->write + 1) % cur_buffer->size == cur_buffer->read) {
        mycoroutine_switchto(read_thread);
    }

    cur_buffer->buffer[cur_buffer->write] = c;
    cur_buffer->write = (cur_buffer->write + 1) % cur_buffer->size;

    return 1;
}

int pipe_writeDone(unsigned int p) {
    pipe_t *to_write = pipe_get(p, NULL);
    if (to_write == NULL)
        return -1;

    to_write->open_to_write = 0;

    return 1;
}

int pipe_read(unsigned int p, char *c, co_t *write_thread) {
    const pipe_t *cur_pipe = pipe_get(p, NULL);
    if (cur_pipe == NULL)
        return -1;

    ring_buffer_t *cur_buffer = cur_pipe->ring_buffer;
    // if there is nothing to read on the pipe and the pipe is open for writing
    if ((cur_buffer->read + 1) % cur_buffer->size == cur_buffer->write && cur_pipe->open_to_write) {
        mycoroutine_switchto(write_thread);
    }

    // if the pipe is closed for writing and there is still nothing to read
    if (!cur_pipe->open_to_write && (cur_buffer->read + 1) % cur_buffer->size == cur_buffer->write) {
        pipe_remove(p);
        return 0;
    }

    *c = *(cur_buffer->buffer + ((cur_buffer->read + 1) % cur_buffer->size));
    cur_buffer->read = (cur_buffer->read + 1) % cur_buffer->size;
    
    return 1;
}

int pipe_add(pipe_t *pipe) {
    pipe->id = pipe_count++;
    pipe_t **new_pipes = realloc(pipes, pipe_count * sizeof(pipe_t *));
    if (new_pipes == NULL)
        return -1;
    pipes = new_pipes;
    pipes[pipe_count - 1] = pipe;
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
    if (to_remove == NULL)
        return 1;
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