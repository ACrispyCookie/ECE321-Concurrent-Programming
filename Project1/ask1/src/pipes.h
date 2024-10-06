#ifndef PIPES_H
#define PIPES_H

#include "uthash.h"

typedef struct ring_buffer {
    int size;
    int read;
    int write;
    char *buffer;
} ring_buffer_t;

typedef struct pipe {
    int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
    UT_hash_handle hh;
} pipe_t;

int pipe_open(int size);
int pipe_write(int p, char c);
int pipe_writeDone(int p);
int pipe_read(int p, char *c);

int next_id = 0;

#endif