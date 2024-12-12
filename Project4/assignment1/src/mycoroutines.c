#include "mycoroutines.h"

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
    //Initialize co_t struct
    

    return 1;
}

int mycoroutine_create(co_t *co, void(body)(void *), void *arg) {

}

int mycoroutine_switchto(co_t *co) {

}

int mycoroutine_destroy(co_t *s) {
    if (myco_get(s) == -1)
        return -1;

    //Remove from internal array and free memory in struct
    myco_remove(s);
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

int myco_remove(const co_t *s) {
    if (cos == NULL)
        return 1;

    int co_index = myco_get(s);
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