#ifndef _MYCOROUTINES_H_
#define _MYCOROUTINES_H_
#include <ucontext.h>

/*
    Struct describing a coroutine.
    Fields:
    unsigned int id - The coroutine ID.
*/
typedef struct coroutine {
    unsigned int id;
    ucontext_t context;
} co_t;

/*
    Initializes the main coroutine.

    Parameters:
    co_t *s - the main coroutine of the problem

    Returns:
    1 for success
    -1 if an error occurred
*/
int mycoroutine_init(co_t *main);

/*
    Initializes the given coroutine.

    Parameters:
    co_t *co - the coroutine to initialize
    void(body)(void *) - the function the coroutine runs
    void *arg - the argument passed to the coroutine 

    Returns:
    1 for success
    -1 if an error occurred
*/
int mycoroutine_create(co_t *co, void(body)(void *), void *arg);

/*
    Switches the context to the given coroutine.

    Parameters:
    co_t *co - the coroutine to initialize

    Returns:
    1 for success
    -1 if an error occurred
*/
int mycoroutine_switchto(co_t *co);

/*
    Destroys a coroutine freeing all its
    related memory.

    Parameters:
    co_t *co - the semaphore to destroy

    Returns:
    1 for success
    -1 if an error occurred
*/
int mycoroutine_destroy(co_t *co);

#endif