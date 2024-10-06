#ifndef PIPES_H
#define PIPES_H

typedef struct ring_buffer {
    int size;
    int read;
    int write;
    int full;
    char *buffer;
} ring_buffer_t;

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