#include "mythreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


enum alarm_op {
    DISABLE_ALL, // Disable interrupting too
    DISABLE, // Disable only alarm
    ENABLE_PREVIOUS,
    RESET
};

/*
    Internal function to add a thread to the internal
    array of threads.

    Parameters:
    mythr_t *thr - the thread to add
    int id - the id of the thread
    Returns:
    1 for success
    -1 for failure
*/
int mythr_add(mythr_t *thr, int id);

/*
    Internal function to remove a thread from the internal
    array of threads.

    Parameters:
    mythr_t *thr - the thread to remove
    Returns:
    1 for success
    -1 for failure
*/
int mythr_remove(const mythr_t *thr);

/*
    Internal function to get the index of a thread
    in the internal array of threads.

    Parameters:
    mythr_t *thr - the thread to get
    Returns:
    -1 if a thread with the given id doesn't exist
    or the index of the thread if it exists
*/
int mythr_get(const mythr_t *thr);

/*
    Performs an operation on the alarm and stores the previous alarm

    Parameters:
    enum alarm_op operation - The operation to perform
    struct itimerval *previous - the struct to store the previous alarm
*/
void alarm_op(enum alarm_op operation, struct itimerval *previous);

/*
    Handles the logic of the scheduler.

    Parameters:
    enum thread_state state - the state to set the previously running thread to
*/
void run_scheduler(enum thread_state state);

/*
    Function that is passed to the coroutines of
    the threads.
*/
void run_thread(void *arg);

/*
    Searches the next ready thread in queue and finds the thread
    that needs to wake up the earliest if there is any.

    Parameters:
    bool include_running - Include the currently running thread as a sleeping thread
    int *earliest_thread - The thread that is sleeping and needs to wake up the earliest

    Returns:
    the index of the next ready thread in the queue
    or -1 if there is no ready thread in the queue
*/
int search_threads(bool include_running, int *earliest_thread);

/*
    Switches the context to the given thread.

    Parameters:
    enum thread_state state - the state to set the previously running thread to
    int next_thread_index - the thread to switch the context to
*/
void switch_thread(enum thread_state state, int next_thread_index);

/*
    Sleeps until the given timestamp. If the timestamp has passed
    the function returns.

    Parameters:
    int timestamp - Epoch timestamp
*/
void sleep_until(int timestamp);

/*
    SIGALRM signal handler for context switching.

    Parameters:
    int signum - the signal number of the received signal
*/
void thread_timeout_handler(int signum);

// Variables for interrupting and default alarms
static bool no_int = false;
static struct itimerval default_alarm, disarmed_alarm;

// Thread array and main thread related
static mythr_t main;
static mythr_t **thrs;
static unsigned int thr_count = 0;
static unsigned int running = 0;

void run_scheduler(enum thread_state state) {
    alarm_op(DISABLE_ALL, NULL);
    mythr_t *current = thrs[running];

    if (thr_count == 1) {
        switch (state) {
            case TERMINATED:
            case READY:
                break;
            case BLOCKED:
                printf("Deadlock detected!\n");
                exit(1);
            case SLEEPING:
                sleep_until(current->sleep_until);
        }
        alarm_op(DISABLE, NULL);
        return;
    }

    int next_ready_index, earliest_thread_index;
    next_ready_index = search_threads(state == SLEEPING, &earliest_thread_index);

    if (next_ready_index != -1) {
        switch_thread(state, next_ready_index);
        return;
    }

    mythr_t *earliest_thread = thrs[earliest_thread_index];
    switch (state) {
        case READY:
            alarm_op(RESET, NULL);
            break;
        case SLEEPING: {
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread_index);
            break;
        }
        case TERMINATED:
        case BLOCKED: {
            if (earliest_thread_index == -1) {
                printf("Deadlock detected!\n");
                exit(1);
            }
            
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread_index);
        }
    }
}

void run_thread(void *arg) {
    thread_runnable_t *runnable = (thread_runnable_t *) arg;
    void (*body) (void *) = runnable->body;
    void *body_arg = runnable->arg;

    body(body_arg);
    run_scheduler(TERMINATED);
}

int search_threads(bool include_running, int *earliest_thread) {
    int next_ready = -1;
    int minimum_sleeping = include_running ? running : -1;

    for (int i = (running + 1) % thr_count; i != running; i = (i + 1) % thr_count) {
        mythr_t *next = thrs[i];
        if (next->state == READY) {
            next_ready = i;
            break;
        } else if (next->state == SLEEPING && (minimum_sleeping == -1 || (thrs[minimum_sleeping])->sleep_until > next->sleep_until))
            minimum_sleeping = i;
    }

    *earliest_thread = minimum_sleeping;
    return next_ready;
}

void switch_thread(enum thread_state state, int next_thread_index) {
    mythr_t *current_thread = thrs[running];
    current_thread->state = state;
    running = next_thread_index;

    mythr_t *next_thread = thrs[running];
    next_thread->state = READY;
    alarm_op(RESET, NULL);
    mycoroutine_switchto(&next_thread->co);
}

void alarm_op(enum alarm_op operation, struct itimerval *previous) {
    no_int = !operation;
    switch (operation)
    {
        case DISABLE_ALL:
        case DISABLE:
            setitimer(ITIMER_REAL, &disarmed_alarm, previous);
            break;
        case ENABLE_PREVIOUS:
            setitimer(ITIMER_REAL, previous, NULL);
            break;
        case RESET:
            setitimer(ITIMER_REAL, &default_alarm, previous);
            break;
    }
}

void sleep_until(int timestamp) {
    int sleep_amount = timestamp - time(NULL);
    if (sleep_amount > 0)
        sleep(sleep_amount);
}

void thread_timeout_handler(int signum) {
    if (no_int)
        return;
    run_scheduler(READY);
}

int mythreads_init() {
    //Initialize thread and add mythr_t to internal array
    int id = thr_count;
    int error = mythr_add(&main, id);
    if (error == -1)
        return -1;
    
    //Initialize alarms for the schedulers
    default_alarm.it_interval.tv_sec = 0;
    default_alarm.it_interval.tv_usec = THREAD_TIMEOUT_TIME;
    default_alarm.it_value.tv_sec = 0;
    default_alarm.it_value.tv_usec = THREAD_TIMEOUT_TIME;

    disarmed_alarm.it_interval.tv_sec = 0;
    disarmed_alarm.it_interval.tv_usec = 0;
    disarmed_alarm.it_value.tv_sec = 0;
    disarmed_alarm.it_value.tv_usec = 0;

    //Set signal handler for SIGALRM and SIGINT
    struct sigaction alarm_action;
    sigset_t blocked_sigs;
    sigemptyset(&blocked_sigs);
    sigaddset(&blocked_sigs, SIGINT);
    alarm_action.sa_handler = thread_timeout_handler;
    alarm_action.sa_mask = blocked_sigs;
    alarm_action.sa_flags = 0;
    sigaction(SIGALRM, &alarm_action, NULL);
    sigaction(SIGINT, &alarm_action, NULL);

    //Initialize main mythr_t struct
    error = mycoroutine_init(&main.co);
    if (error == -1)
        return -1;

    return 1;
}

int mythreads_yield() {
    run_scheduler(READY);
    return 1;
}

int mythreads_sleep(int secs) {
    return 1;
}

int mythreads_create(mythr_t *thr, void (body)(void *), void *arg) {
    struct itimerval previous_alarm;
    alarm_op(DISABLE_ALL, &previous_alarm);
    if (mythr_get(thr) != -1) {
        printf("Error: thread already exists\n");
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Initialize thread and add mythr_t to internal array
    int id = thr_count;
    int error = mythr_add(thr, id);
    if (error == -1) {
        printf("Error: addin thread\n");
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }
    
    //Initialize mythr_t struct
    thr->co.id = -1;
    thr->state = READY;
    thr->runnable.body = body;
    thr->runnable.arg = arg;
    error = mycoroutine_create(&thr->co, run_thread, &thr->runnable);
    if (error == -1) {
        printf("Error: creating coroutine\n");
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    if (thr_count == 2)
        alarm_op(RESET, &previous_alarm);
    else 
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
    
    return 1;
}

int mythreads_destroy(mythr_t *thr) {
    struct itimerval previous_alarm;
    alarm_op(DISABLE_ALL, &previous_alarm);
    if (mythr_get(thr) == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Remove from internal array and free memory in struct
    int error = mycoroutine_destroy(&thr->co);
    if (error == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    error = mythr_remove(thr);
    if (error == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    if (thr_count != 1)
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
    else
        alarm_op(DISABLE, NULL);
    
    return 1;
}

int mythr_add(mythr_t *thr, const int id) {
    thr->id = id;
    mythr_t **new_thrs = realloc(thrs, ++thr_count * sizeof(mythr_t *));
    if (new_thrs == NULL)
        return -1;
    thrs = new_thrs;
    thrs[thr_count - 1] = thr;
    return 1;
}

int mythr_remove(const mythr_t *thr) {
    if (thrs == NULL)
        return 1;

    int thr_index = mythr_get(thr);
    if (thr_index == -1)
        return 1;
    thrs[thr_index] = thrs[thr_count - 1];
    --thr_count;
    
    if (thr_count == 0) {
        free(thrs);
        return 1;
    }

    mythr_t **new_thrs = realloc(thrs, thr_count * sizeof(mythr_t *));
    if (new_thrs == NULL)
        return -1;
    thrs = new_thrs;
    return 1;
}

int mythr_get(const mythr_t *thr) {
    for (int i = 0; i < thr_count; i++) {
        if (thrs[i]->id == thr->id)
            return i;
    }
    return -1;
}