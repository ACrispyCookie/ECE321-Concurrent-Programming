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
mythr_list_t *search_threads(bool include_running, mythr_list_t **earliest_thread);

/*
    Switches the context to the given thread.

    Parameters:
    enum thread_state running_next_state - the state to set the previously running thread to
    int next_thread_index - the thread to switch the context to
*/
void switch_thread(enum thread_state running_next_state, mythr_list_t *next_thread_node);

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

/*
    Internal function to initialize a circular thread list.

    Parameters:
    mythr_list_t **head - the pointer to store the head of the list
    Returns:
    1 for success
    -1 for failure
*/
mythr_list_t *thread_list_init();

/*
    Internal function to add a thread to a
    list of threads.

    Parameters:
    mythr_list_t *head - the list to add the thread to
    unsigned int *list_size - the size of the list
    mythr_t *thr - the thread to add
    Returns:
    1 for success
    -1 for failure
*/
int thread_list_add(mythr_list_t *head, unsigned int *list_size, mythr_t *thr);

/*
    Internal function to remove a thread from a
    list of threads.

    Parameters:
    mythr_list_t *head - the list to remove the thread from
    unsigned int *list_size - the size of the list
    mythr_t *thr - the thread to remove
    Returns:
    1 for if the thread didn't exist in the list or it was successfully removed
    -1 for failure
*/
int thread_list_remove(mythr_list_t *head, unsigned int *list_size, mythr_t *thr);

/*
    Internal function to check if a thread is present
    in a list of threads.

    Parameters:
    mythr_list_t *head - the list to check if the thread exists in
    mythr_t *thr - the thread to check
    Returns:
    true - if the thread exists in the list
    false - if the thread doesn't exist in the list
*/
bool thread_list_exists(mythr_list_t *head, mythr_t *thr);

// Variables for interrupting and default alarms
static bool no_int = false;
static struct itimerval default_alarm, disarmed_alarm;

// Thread array and main thread related
static mythr_t main;
static mythr_list_t *running_node;
static mythr_list_t *thrs;
static unsigned int thr_count = 0;

void run_scheduler(enum thread_state state) {
    alarm_op(DISABLE_ALL, NULL);
    mythr_t *current = running_node->thr;

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

    reload_states(state);
    mythr_list_t *next_ready_node, *earliest_thread_node;
    next_ready_node = search_threads(state == SLEEPING, &earliest_thread_node);

    if (next_ready_node != NULL) {
        switch_thread(state, next_ready_node);
        return;
    }

    mythr_t *earliest_thread = earliest_thread_node == NULL ? NULL : earliest_thread_node->thr;
    switch (state) {
        case READY:
            alarm_op(RESET, NULL);
            break;
        case SLEEPING: {
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread_node);
            break;
        }
        case TERMINATED:
        case BLOCKED: {
            if (earliest_thread == NULL) {
                printf("Deadlock detected!\n");
                exit(1);
            }
            
            sleep_until(earliest_thread->sleep_until);
            switch_thread(state, earliest_thread_node);
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
    for (mythr_list_t *node = running_node->next; node != running_node; node = node->next) {
        if (node == thrs)
            continue;

        mythr_t *thr = node->thr;
        bool sleep_expired = thr->state == SLEEPING && thr->sleep_until < time_in_millis();
        bool join_thread_terminated = thr->state == BLOCKED && thr->joining_on != NULL && 
                                    (thr->joining_on->state == TERMINATED || (thr->joining_on == running_node->thr && running_next_state == TERMINATED));
        if (sleep_expired || join_thread_terminated)
            thr->state = READY;
    }
}

mythr_list_t *search_threads(bool include_running, mythr_list_t **earliest_thread) {
    mythr_list_t *next_ready = NULL;
    mythr_list_t *minimum_sleeping = include_running ? running_node : NULL;

    for (mythr_list_t *node = running_node->next; node != running_node; node = node->next) {
        if (node == thrs)
            continue;
        
        mythr_t *thr = node->thr;
        if (thr->state == READY) {
            next_ready = node;
            break;
        } else if (thr->state == SLEEPING && (minimum_sleeping == NULL || minimum_sleeping->thr->sleep_until > thr->sleep_until))
            minimum_sleeping = node;
    }

    *earliest_thread = minimum_sleeping;
    return next_ready;
}

void switch_thread(enum thread_state running_next_state, mythr_list_t *next_thread_node) {
    mythr_t *current_thread = running_node->thr;
    current_thread->state = running_next_state;
    running_node = next_thread_node;

    mythr_t *next_thread = running_node->thr;
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
    if (no_int)
        return;

    run_scheduler(running_node->thr->state);
}

int mythreads_init() {
    //Initialize thread and add mythr_t to internal array
    thrs = thread_list_init();
    if (thrs == NULL)
        return -1;
    if (thread_list_add(thrs, &thr_count, &main) == -1)
        return -1;
    running_node = thrs->next;
    
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
    mythr_t *current = running_node->thr;
    current->sleep_until = time_in_millis() + (secs * 1000L);
    
    run_scheduler(SLEEPING);
    return 1;
}

int mythreads_join(mythr_t *thr) {
    struct itimerval previous;
    alarm_op(DISABLE_ALL, &previous);
    if (thr == NULL || thr->state == TERMINATED) {
        alarm_op(ENABLE_PREVIOUS, &previous);
        return 1;
    }

    running_node->thr->joining_on = thr;
    run_scheduler(BLOCKED);
    return 1;
}

int mythreads_create(mythr_t *thr, void (body)(void *), void *arg) {
    struct itimerval previous_alarm;
    alarm_op(DISABLE_ALL, &previous_alarm);
    if (thread_list_exists(thrs, thr)) {
        printf("Error: thread already exists\n");
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Initialize thread and add mythr_t to internal array
    int error = thread_list_add(thrs, &thr_count, thr);
    if (error == -1) {
        printf("Error: addin thread\n");
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }
    
    //Initialize mythr_t struct
    thr->co.id = -1;
    thr->state = READY;
    thr->joining_on = NULL;
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
    if (!thread_list_exists(thrs, thr)) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    //Remove from internal array and free memory in struct
    int error = mycoroutine_destroy(&thr->co);
    if (error == -1) {
        alarm_op(ENABLE_PREVIOUS, &previous_alarm);
        return -1;
    }

    error = thread_list_remove(thrs, &thr_count, thr);
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

void mythreads_exit() {
    thread_list_remove(thrs, &thr_count, &main);
    free(thrs);
}

mythr_list_t *thread_list_init() {
    mythr_list_t *head_node = (mythr_list_t *) malloc(sizeof(mythr_list_t));
    if (head_node == NULL)
        return NULL;

    head_node->next = head_node;
    head_node->thr = NULL;
    return head_node;
}

int thread_list_add(mythr_list_t *head, unsigned int *list_size, mythr_t *thr) {
    mythr_list_t *node = head;

    mythr_list_t *new_node = (mythr_list_t *) malloc(sizeof(mythr_list_t));
    if (new_node == NULL)
        return -1;
    
    while(node->next != head)
        node = node->next;

    new_node->next = head;
    new_node->thr = thr;
    node->next = new_node;
    (*list_size)++;
    return 1;
}

int thread_list_remove(mythr_list_t *head, unsigned int *list_size, mythr_t *thr) {
    mythr_list_t *previous = head;
    mythr_list_t *node = head->next;

    head->thr = thr;
    while (node->thr != thr) {
        previous = node;
        node = node->next;
    }
    
    if (node == head)
        return 1;
    
    previous->next = node->next;
    (*list_size)--;
    free(node);
    return 1;
}

bool thread_list_exists(mythr_list_t *head, mythr_t *thr) {
    mythr_list_t *node = head->next;

    head->thr = thr;
    while(node->thr != thr)
        node = node->next;

    return node != head;
}