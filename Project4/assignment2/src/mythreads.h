#ifndef _MYTHREADS_H_
#define _MYTHREADS_H_
#include "../../assignment1/src/mycoroutines.h"
#include "../../list/src/list.h"
#include <sys/time.h>

#define THREAD_TIMEOUT_TIME 7000
#define ALARM_TYPE SIGALRM
#define TIMER_TYPE ITIMER_REAL 

/*
    Enum used to describe the runtime of a thread.
*/
typedef struct thread_runnable {
    void (*body) (void *);
    void *arg;
} thread_runnable_t;

/*
    Enum describing the state of a thread.
*/
enum thread_state {
    READY,
    SLEEPING,
    BLOCKED,
    TERMINATED
};

/*
    Struct describing a thread.
    Fields:
    unsigned int id - The thread ID.
    co_t *co - The coroutine related to the thread.
*/
typedef struct mythread {
    thread_runnable_t runnable;
    co_t co;
    enum thread_state state;
    unsigned long long sleep_until;
    struct mythread *joining_on;
} mythr_t;

/*
    Struct describing a binary semaphore.
    Fields:
    int val - The value of the semaphore. Should never be touched.
    list_t *waiting - The list of the waiting threads in the semaphore 
*/
typedef struct mysem {
    int val;
    list_t *waiting;
} mysem_t;

/*
    Initializes the threads environment.

    Returns:
    1 for success
    -1 if an error occurred
*/
int mythreads_init();

/*
    Initializes a new thread.

    Parameters:
    mythr_t *thr - the pointer to store the initialized thread to
    void(body)(void *) - the function the thread runs
    void *arg - the argument passed to the thread 

    Returns:
    1 for success
    -1 if an error occurred
*/
int mythreads_create(mythr_t *thr, void(body)(void *), void *arg);

/*
    Voluntarily passes the runtime to another thread.

    Returns:
    1 for success
*/
int mythreads_yield();

/*
    Makes the thread wait for the given time.

    Parameters:
    int secs - the amount of seconds to sleep

    Returns:
    1 for success
*/
int mythreads_sleep(int secs);

/*
    Waits for the given thread to finish.

    Parameters:
    mythr_t *thr - the thread to wait for

    Returns:
    1 for success
    -1 if an error occurred
*/
int mythreads_join(mythr_t *thr);

/*
    Destroys a thread freeing all its
    related memory.

    Parameters:
    mythr_t *thr - the thread to destroy

    Returns:
    1 for success
    -1 if an error occurred
*/
int mythreads_destroy(mythr_t *thr);

/*
    Creates a binary semaphore and initializes it
    to the given value.

    Parameters:
    mysem_t *s - the pointer to store the initialized semaphore to
    int val - the value to initialize the semaphore to 

    Returns:
    1 for success
    0 if the n is not 0 or 1
    -1 if the given semaphore is already initialized
*/
int mythreads_sem_create(mysem_t *s, int val);

/*
    If the semaphore has a value of 0 the caller goes into
    a queue and sleeps until someone calls mythreads_mysem_up and removes
    him from the queue.

    Parameters:
    mysem_t *s - the semaphore to call down on

    Returns:
    1 for success
    -1 if the given semaphore is not initialized
*/
int mythreads_sem_down(mysem_t *s);

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
int mythreads_sem_up(mysem_t *s);

/*
    Destroys a binary semaphore freeing all its
    related memory.

    Parameters:
    mysem_t *s - the semaphore to destroy

    Returns:
    1 for success
    -1 if the given semaphore is not initialized or is already destroyed
*/
int mythreads_sem_destroy(mysem_t *s);

/*
    Exits the threads environment freeing
    any related allocated memory.
*/
void mythreads_exit();

#endif