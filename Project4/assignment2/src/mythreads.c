#include "mythreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

enum alarm_op {
    DISABLE_ALL, // Disable interrupting too
    DISABLE, // Disable only alarm
    ENABLE_PREVIOUS,
    RESET
};

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
    Reloads the states of all the threads currently running.

    Parameter:
    enum thread_state running_next_state - The next state the running thread will change to
*/
void reload_states(enum thread_state state);

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
mythr_t *search_threads(bool include_running, mythr_t **earliest_thread);

/*
    Switches the context to the given thread.

    Parameters:
    enum thread_state running_next_state - the state to set the previously running thread to
    int next_thread_index - the thread to switch the context to
*/
void switch_thread(enum thread_state running_next_state, mythr_t *next_thread_node);

/*
    Performs an operation on the alarm and stores the previous alarm

    Parameters:
    enum alarm_op operation - The operation to perform
    struct itimerval *previous - the struct to store the previous alarm
*/
void alarm_op(enum alarm_op operation, struct itimerval *previous);

/*
    Returns an epoch timestamp in milliseconds.
*/
unsigned long long time_in_millis();

/*
    Sleeps until the given timestamp. If the timestamp has passed
    the function returns.

    Parameters:
    int timestamp - Epoch timestamp
*/
void sleep_until(unsigned long long timestamp);

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
static mythr_t *running_thr;
static list_t *thrs;

void run_scheduler(enum thread_state state) {
    alarm_op(DISABLE_ALL, NULL);

    if (thrs->size == 1) {
        switch (state) {
            case TERMINATED:
            case READY:
                break;
            case BLOCKED:
                printf("Deadlock detected!\n");
                exit(1);
            case SLEEPING:
                sleep_until(running_thr->sleep_until);
        }
        alarm_op(DISABLE, NULL);
        return;
    }

    reload_states(state);
    mythr_t *next_ready, *earliest_thread;
    next_ready = search_threads(state == SLEEPING, &earliest_thread);

    if (next_ready != NULL) {
        switch_thread(state, next_ready);
        return;
    }

    switch (state) {
        case READY:
            alarm_op(RESET, NULL);
            break;
        case SLEEPING: {
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread);
            break;
        }
        case TERMINATED:
        case BLOCKED: {
            if (earliest_thread == NULL) {
                printf("Deadlock detected!\n");
                exit(1);
            }
            
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread);
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

void reload_states(enum thread_state running_next_state) {
    node_t *running_node = list_find(thrs, running_thr, NULL);
    for (node_t *node = running_node->next; node != running_node; node = node->next) {
        if (node == thrs->head)
            continue;

        mythr_t *thr = (mythr_t *) node->data;
        bool sleep_expired = thr->state == SLEEPING && thr->sleep_until < time_in_millis();
        bool join_thread_terminated = thr->state == BLOCKED && thr->joining_on != NULL && 
                                    (thr->joining_on->state == TERMINATED || (thr->joining_on == running_thr && running_next_state == TERMINATED));
        if (sleep_expired || join_thread_terminated)
            thr->state = READY;
    }
}

mythr_t *search_threads(bool include_running, mythr_t **earliest_thread) {
    node_t *running_node = list_find(thrs, running_thr, NULL);
    mythr_t *next_ready = NULL;
    mythr_t *minimum_sleeping = include_running ? running_thr : NULL;

    for (node_t *node = running_node->next; node != running_node; node = node->next) {
        if (node == thrs->head)
            continue;
        
        mythr_t *thr = (mythr_t *) node->data;
        if (thr->state == READY) {
            next_ready = thr;
            break;
        } else if (thr->state == SLEEPING && (minimum_sleeping == NULL || minimum_sleeping->sleep_until > thr->sleep_until))
            minimum_sleeping = thr;
    }

    *earliest_thread = minimum_sleeping;
    return next_ready;
}

void switch_thread(enum thread_state running_next_state, mythr_t *next_thread) {
    running_thr->state = running_next_state;
    running_thr = next_thread;

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
            // printf("disable alarm\n");
            break;
        case ENABLE_PREVIOUS:
            // printf("enable previous alarm\n");
            setitimer(ITIMER_REAL, previous, NULL);
            break;
        case RESET:
            // printf("reset alarm\n");
            setitimer(ITIMER_REAL, &default_alarm, previous);
            break;
    }
}

unsigned long long time_in_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (unsigned long long)(tv.tv_sec) * 1000 +
           (unsigned long long)(tv.tv_usec) / 1000;
}

void sleep_until(unsigned long long timestamp) {
    int sleep_amount_usecs = (timestamp - time_in_millis()) * 1000;
    while (sleep_amount_usecs > 0) {
        usleep(sleep_amount_usecs);
        sleep_amount_usecs = (timestamp - time_in_millis()) * 1000;
    }
}

void thread_timeout_handler(int signum) {
    // printf("alarm hit\n");
    if (no_int)
        return;

    run_scheduler(running_thr->state);
    // printf("alarm exit\n");
}

int mythreads_init() {
    //Initialize thread and add mythr_t to internal array
    thrs = list_init();
    if (thrs == NULL)
        return -1;
    if (list_add(thrs, &main) == -1)
        return -1;
    running_thr = &main;
    
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
    sigaddset(&blocked_sigs, SIGALRM);
    alarm_action.sa_handler = thread_timeout_handler;
    alarm_action.sa_mask = blocked_sigs;
    alarm_action.sa_flags = 0;
    sigaction(SIGALRM, &alarm_action, NULL);
    sigaction(SIGINT, &alarm_action, NULL);

    //Initialize main mythr_t struct
    main.joining_on = NULL;
    if (mycoroutine_init(&main.co) == -1)
        return -1;

    return 1;
}

int mythreads_yield() {
    run_scheduler(READY);
    return 1;
}

int mythreads_sleep(int secs) {
    alarm_op(DISABLE_ALL, NULL);
    running_thr->sleep_until = time_in_millis() + (secs * 1000L);
    
    run_scheduler(SLEEPING);
    return 1;
}

int mythreads_join(mythr_t *thr) {
    struct itimerval previous;
    alarm_op(DISABLE_ALL, &previous);
    // printf("join start\n");
    if (thr == NULL || thr->state == TERMINATED) {
        alarm_op(ENABLE_PREVIOUS, &previous);
        return 1;
    }

    running_thr->joining_on = thr;
    run_scheduler(BLOCKED);
    return 1;
}

int mythreads_create(mythr_t *thr, void (body)(void *), void *arg) {
    struct itimerval previous_alarm;
    alarm_op(DISABLE_ALL, &previous_alarm);
    if (list_find(thrs, thr, NULL) != NULL) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Initialize thread and add mythr_t to internal list
    if (list_add(thrs, thr) == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }
    
    //Initialize mythr_t struct
    thr->state = READY;
    thr->joining_on = NULL;
    thr->runnable.body = body;
    thr->runnable.arg = arg;
    if (mycoroutine_create(&thr->co, run_thread, &thr->runnable) == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Start alarm if this was the second thread or restore previous running time
    if (thrs->size == 2)
        alarm_op(RESET, &previous_alarm);
    else 
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
    
    return 1;
}

int mythreads_destroy(mythr_t *thr) {
    struct itimerval previous_alarm;
    alarm_op(DISABLE_ALL, &previous_alarm);
    if (list_find(thrs, thr, NULL) == NULL) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Destroy coroutine of thread
    if (mycoroutine_destroy(&thr->co) == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Remove thread from internal list
    if (list_remove(thrs, thr) == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Disable alarm if this is the last thread
    if (thrs->size != 1)
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
    else
        alarm_op(DISABLE, NULL);
    
    return 1;
}

void mythreads_exit() {
    mycoroutine_destroy(&main.co);
    list_destroy(thrs);
}