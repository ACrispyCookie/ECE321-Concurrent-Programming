#include "../src/mycoroutines.h"
#ifndef PIPES_H
#define PIPES_H

/*
    Struct that describes a ring buffer.

    Fields:
    int size - The size of the buffer.
    int read - The read pointer of the buffer.
    int write - The write pointer of the buffer.
    char *buffer - The actual buffer used for storing bytes.
*/
typedef struct ring_buffer {
    int size;
    int read;
    int write;
    char *buffer;
} ring_buffer_t;

/*
    Struct that describes a pipe and its state.

    Fields:
    unsigned int id - The id of the pipe.
    int open_to_write - A flag that describes if the pipe is open for writing.
    ring_buffer_t *ring_buffer - The ring buffer used for reading and writing.
*/
typedef struct pipe {
    unsigned int id;
    int open_to_write;
    ring_buffer_t *ring_buffer;
} pipe_t;

/*
    Opens a pipe to be used between 2 
    threads with the given size.

    Parameters:
    int size - The size in bytes of the pipe.
    Returns:
    The id of the new pipe that is opened.
*/
unsigned int pipe_open(int size);

/*
    Writes the given byte to the given pipe
    if the pipe exists and is not full. If the pipe
    is currently full the function waits until there
    is available space for writing.

    Parameters:
    unsigned int p - The id of the pipe to write to.
    char c - The byte to write to the pipe.
    co_t *read_thread - The thread to switch to if you can't write
    Returns:
    1 for success,
    -1 if a pipe with the given id doesn't exist
    or isn't open for writing.
*/
int pipe_write(unsigned int p, char c, co_t *read_thread);

/*
    Closes the pipe for writing.

    Parameters:
    unsigned int p - The id of the pipe to close.
    Returns:
    1 for success,
    -1 if a pipe with the given id doesn't exist
    or isn't open for writing.
*/
int pipe_writeDone(unsigned int p);

/*
    Reads one byte from the given pipe if the pipe exists
    and the pipe isn't empty. If the pipe is empty the function
    waits until a thread writes a byte to the pipe. When 0 is returned
    from this function the pipe is closed and destroyed.

    Parameters:
    unsigned int p - The id of the pipe to read from.
    char *c - The buffer to write the byte from the pipe.
    co_t *write_thread - The thread to switch to if you can't read
    Returns:
    1 for success,
    0 if the pipe is empty and closed for writing,
    -1 if a pipe with the given id doesn't exist
    or isn't open for reading.
*/
int pipe_read(unsigned int p, char *c, co_t *write_thread);

#endif