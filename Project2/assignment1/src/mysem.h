#ifndef _MYSEM_H_
#define _MYSEM_H_
#include <pthread.h>

/*
    Struct describing a binary semaphore.
    Fields:
    unsigned int id - The semaphore ID. Should be set to -1 before calling mysem_init.
    int val - The value of the semaphore. Should never be touched.
    pthread_mutext_t lock - Lock used internally. Should never be touched.
*/
typedef struct mysem {
    unsigned int id;
    int val;
    pthread_mutex_t lock;
} mysem_t;

/*
    Initializes a binary semaphore to the given value.
    If the semaphore is already initialized or the
    given value is not 0 or 1 the function returns early.

    NOTE: since we need a value to differentiate initialized
    and uninitialized semaphores apart the user must set the id 
    of the semaphore to -1 before calling this function.

    Parameters:
    mysem_t *s - the semaphore to initialize
    int n - the value to initialize the semaphore to

    Returns:
    1 for success
    0 if the n is not 0 or 1
    -1 if the given semaphore is already initialized
*/
int mysem_init(mysem_t *s, int n);

/*
    If the semaphore has a value of 0 the caller goes into
    a queue and sleeps until someone calls mysem_up and removes
    him from the queue. This function is thread-safe meaning that
    multiple threads at a time can call this function.

    Parameters:
    mysem_t *s - the semaphore to call down on

    Returns:
    1 for success
    -1 if the given semaphore is not initialized
*/
int mysem_down(mysem_t *s);

/*
    If there are threads currently in the queue it removes
    one thread from the queue and wakes it up. If there are
    no threads in the queue it tries to add 1 to the value of
    the semaphore. If the value is already 1 it fails.

    Parameters:
    mysem_t *s - the semaphore to call up on

    Returns:
    1 for success
    0 if the semaphore already had a value of 1
    -1 if the given semaphore is not initialized
*/
int mysem_up(mysem_t *s);

/*
    Destroys a binary semaphore freeing all its
    related memory.

    Parameters:
    mysem_t *s - the semaphore to destroy

    Returns:
    1 for success
    -1 if the given semaphore is not initialized or is already destroyed
*/
int mysem_destroy(const mysem_t *s);

#endif