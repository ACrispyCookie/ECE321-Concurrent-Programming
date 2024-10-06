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
    int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
} pipe_t;

int pipe_open(int size);
int pipe_write(int p, char c);
int pipe_writeDone(int p);
int pipe_read(int p, char *c);

int next_id = 0;

#endif