#include "mycoroutines.h"
#include "../../list/src/list.h"
#include <stdlib.h>
#include <signal.h>
#include <valgrind/valgrind.h>

static co_t *current_context, *main_context;
static list_t *coroutine_list;

int mycoroutine_init(co_t *main) {
    //Initialize coroutine and add co_t to internal array
    coroutine_list = list_init();
    if (coroutine_list == NULL)
        return -1;
    if(list_add(coroutine_list, main) == -1) 
        return -1;

    //Set current to main
    current_context = main;
    main_context = main;
    return 1;
}

int mycoroutine_create(co_t *co, void (body)(void *), void *arg) {
    if (list_find(coroutine_list, co, NULL) != NULL)
        return 0;
    if(list_add(coroutine_list, co) == -1)
        return -1;
    
    //Initialize co_t struct and ucontext_t
    getcontext(&co->context);
    co->context.uc_link = NULL;
    void *stack = malloc(SIGSTKSZ);
    VALGRIND_STACK_REGISTER(stack, stack + SIGSTKSZ);
    co->context.uc_stack.ss_sp = stack;
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&co->context, (void (*)(void)) body, 1, arg);

    return 1;
}

int mycoroutine_switchto(co_t *co) {
    if (list_find(coroutine_list, co, NULL) == NULL)
        return -1;

    co_t *prev_context = current_context;
    current_context = co;
    swapcontext(&prev_context->context, &co->context);
    return 1;
}

int mycoroutine_destroy(co_t *co) {
    if (list_find(coroutine_list, co, NULL) == NULL)
        return -1;

    //Remove from internal array and free memory in struct
    if (co == main_context)
        list_destroy(coroutine_list);
    else {
        list_remove(coroutine_list, co);
        VALGRIND_STACK_DEREGISTER(NULL);
        free(co->context.uc_stack.ss_sp);
    }
    return 1;
}