#ifndef PIPES_H
#define PIPES_H

#include <pthread.h>

/*
    Struct that describes a ring buffer.

    Fields:
    int size - The size of the buffer.
    int read - The next position to read from.
    int write - The next position to write to.
    int count - The number of bytes currently stored.
    char *buffer - The actual buffer used for storing bytes.
*/
typedef struct ring_buffer {
    int size;
    int read;
    int write;
    int count;
    char *buffer;
} ring_buffer_t;

/*
    Struct that describes a pipe and its state.

    Fields:
    unsigned int id - The id of the pipe.
    int open_to_write - A flag that describes if the pipe is open for writing.
    ring_buffer_t *ring_buffer - The ring buffer used for reading and writing.
    pthread_mutex_t lock - protects the ring buffer state.
    pthread_cond_t can_read - signaled when data becomes available or the pipe closes.
    pthread_cond_t can_write - signaled when buffer space becomes available.
*/
typedef struct pipe {
    unsigned int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
    pthread_mutex_t lock;
    pthread_cond_t can_read;
    pthread_cond_t can_write;
} pipe_t;

/* Opens a pipe to be used between 2 threads with the given size. */
unsigned int pipe_open(int size);

/* Writes the given byte to the given pipe, blocking while the pipe is full. */
int pipe_write(unsigned int p, char c);

/* Closes the pipe for writing. */
int pipe_writeDone(unsigned int p);

/* Reads one byte from the given pipe, blocking while empty and open. */
int pipe_read(unsigned int p, char *c);

#endif
