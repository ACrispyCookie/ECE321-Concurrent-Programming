#include "mysem.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>

/*
    Internal function to add a binary semaphore to the internal
    array of binary semaphores.

    Parameters:
    mysem_t *s - the semaphore to add
    int id - the id of the semaphore
    Returns:
    1 for success
    -1 for failure
*/
int mysem_add(mysem_t *s, int id);

/*
    Internal function to remove a binary semaphore from the internal
    array of binary semaphores.

    Parameters:
    mysem_t *s - the semaphore to remove
    Returns:
    1 for success
    -1 for failure
*/
int mysem_remove(const mysem_t *s);

/*
    Internal function to get the index of a binary semaphore
    in the internal array of binary semaphores.

    Parameters:
    mysem_t *s - the semaphore to get
    Returns:
    -1 if a binary semaphore with the given id doesn't exist
    or the index of the binary semaphore if it exists
*/
int mysem_get(const mysem_t *s);

static mysem_t **sems;
static unsigned int sems_count = 0;

int mysem_init(mysem_t *s, int n) {
    if (mysem_get(s) != -1)
        return -1;
    if (!(n == 0 || n == 1))
        return 0;

    //Initialize system semaphore and add mysem to internal array
    int id = sems_count;
    int error = mysem_add(s, id);
    if (error == -1) {
        return -1;
    }
    s->val = n;
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->q, NULL);

    return 1;
}

int mysem_down(mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;

    //Using lock for mutual exclusion
    printf("locking on down\n");
    pthread_mutex_lock(&s->lock);
    printf("locked on down %d\n", s->val);
    --s->val;
    if (s->val < 0) {
        printf("waiting and unlocking\n");
        pthread_cond_wait(&s->q, &s->lock);
        return 1;
    }
    printf("unlocking on down\n");
    pthread_mutex_unlock(&s->lock);
    printf("unlocked on down\n");

    return 1;
}

int mysem_up(mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;

    //Using lock for mutual exclusion
    printf("locking on up\n");
    pthread_mutex_lock(&s->lock);
    printf("locked on up %d\n", s->val);
    if (s->val == 1)
        return 0;
    ++s->val;
    if (s->val <= 0) {
        printf("signalling\n");
        pthread_cond_signal(&s->q);
    }
    printf("unlocking on up\n");
    pthread_mutex_unlock(&s->lock);
    printf("unlocked on up\n");

    return 1;
}

int mysem_destroy(mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;

    //Remove from internal array and destroy system semaphore
    pthread_cond_destroy(&s->q);
    pthread_mutex_destroy(&s->lock);
    mysem_remove(s);
    return 1;
}

int mysem_add(mysem_t *s, const int id) {
    s->id = id;
    mysem_t **new_sems = realloc(sems, ++sems_count * sizeof(mysem_t *));
    if (new_sems == NULL)
        return -1;
    sems = new_sems;
    sems[sems_count - 1] = s;
    return 1;
}

int mysem_remove(const mysem_t *s) {
    if (sems == NULL)
        return 1;

    int sem_index = mysem_get(s);
    if (sem_index == -1)
        return 1;
    sems[sem_index] = sems[sems_count - 1];
    --sems_count;
    
    if (sems_count == 0) {
        free(sems);
        return 1;
    }

    mysem_t **new_sems = realloc(sems, sems_count * sizeof(mysem_t *));
    if (new_sems == NULL)
        return -1;
    sems = new_sems;
    return 1;
}

int mysem_get(const mysem_t *s) {
    for (int i = 0; i < sems_count; i++) {
        if (sems[i]->id == s->id)
            return i;
    }
    return -1;
}