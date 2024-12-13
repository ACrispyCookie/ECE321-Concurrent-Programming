#include "mycoroutines.h"
#include <stdlib.h>
#include <signal.h>

/*
    Internal function to add a coroutine to the internal
    array of coroutines.

    Parameters:
    co_t *co - the coroutine to add
    int id - the id of the coroutine
    Returns:
    1 for success
    -1 for failure
*/
int myco_add(co_t *co, int id);

/*
    Internal function to remove a coroutine from the internal
    array of coroutine.

    Parameters:
    co_t *co - the coroutine to remove
    Returns:
    1 for success
    -1 for failure
*/
int myco_remove(const co_t *co);

/*
    Internal function to get the index of a coroutine
    in the internal array of coroutine.

    Parameters:
    co_t *co - the coroutine to get
    Returns:
    -1 if a coroutine with the given id doesn't exist
    or the index of the coroutine if it exists
*/
int myco_get(const co_t *co);

static co_t *current_context, *main_context;
static co_t **cos;
static unsigned int co_count = 0;

int mycoroutine_init(co_t *main) {
    if (myco_get(main) != -1)
        return -1;

    //Initialize coroutine and add co_t to internal array
    int id = co_count;
    int error = myco_add(main, id);
    if (error == -1) {
        return -1;
    }

    //Set current to main
    current_context = main;
    main_context = main;
    return 1;
}

int mycoroutine_create(co_t *co, void (body)(void *), void *arg) {
    if (myco_get(co) != -1)
        return -1;

    //Initialize coroutine and add co_t to internal array
    int id = co_count;
    int error = myco_add(co, id);
    if (error == -1) {
        return -1;
    }
    
    //Initialize co_t struct and ucontext_t
    getcontext(&co->context);
    co->context.uc_link = NULL;
    co->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&co->context, (void (*)(void)) body, 1, arg);

    return 1;
}

int mycoroutine_switchto(co_t *co) {
    if (myco_get(co) == -1)
        return -1;

    co_t *prev_context = current_context;
    current_context = co;
    swapcontext(&prev_context->context, &co->context);
    return 1;
}

int mycoroutine_destroy(co_t *co) {
    if (myco_get(co) == -1)
        return -1;

    //Remove from internal array and free memory in struct
    if (co != main_context)
        free(co->context.uc_stack.ss_sp);
    myco_remove(co);
    return 1;
}

int myco_add(co_t *co, const int id) {
    co->id = id;
    co_t **new_cos = realloc(cos, ++co_count * sizeof(co_t *));
    if (new_cos == NULL)
        return -1;
    cos = new_cos;
    cos[co_count - 1] = co;
    return 1;
}

int myco_remove(const co_t *co) {
    if (cos == NULL)
        return 1;

    int co_index = myco_get(co);
    if (co_index == -1)
        return 1;
    cos[co_index] = cos[co_count - 1];
    --co_count;
    
    if (co_count == 0) {
        free(cos);
        return 1;
    }

    co_t **new_cos = realloc(cos, co_count * sizeof(co_t *));
    if (new_cos == NULL)
        return -1;
    cos = new_cos;
    return 1;
}

int myco_get(const co_t *co) {
    for (int i = 0; i < co_count; i++) {
        if (cos[i]->id == co->id)
            return i;
    }
    return -1;
}