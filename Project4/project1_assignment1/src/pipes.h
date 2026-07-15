#ifndef PIPES_H
#define PIPES_H

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
*/
typedef struct pipe {
    unsigned int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
} pipe_t;

unsigned int pipe_open(int size);
int pipe_write(unsigned int p, char c);
int pipe_writeDone(unsigned int p);
int pipe_read(unsigned int p, char *c);

#endif
