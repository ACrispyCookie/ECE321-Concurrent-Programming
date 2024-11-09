#include "mysem.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>

int mysem_add(mysem_t *s, int id);
int mysem_remove(const mysem_t *s);
int mysem_get(const mysem_t *s);

static mysem_t **sems;
static unsigned int sems_count = 0;

int mysem_init(mysem_t *s, int n) {
    if (mysem_get(s) != -1)
        return -1;
    if (!(n == 0 || n == 1))
        return 0;

    int id = semget(IPC_PRIVATE, 1, S_IRWXU);
    int error = mysem_add(s, id);
    if (error == -1) {
        semctl(id, 0, IPC_RMID);
        return -1;
    }
    semctl(s->id, 0, SETVAL, n);
    s->val = n;
    pthread_mutex_init(&s->lock, NULL);

    return 1;
}

int mysem_down(mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;
    struct sembuf op; op.sem_num = 0; op.sem_op = -1; op.sem_flg = 0;

    pthread_mutex_lock(&s->lock);
    --s->val;
    if (s->val == 0) {
        semop(s->id, &op, 1);
        pthread_mutex_unlock(&s->lock);
        return 1;
    }
    pthread_mutex_unlock(&s->lock);
    semop(s->id, &op, 1);

    return 1;
}

int mysem_up(mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;
    struct sembuf op; op.sem_num = 0; op.sem_op = 1; op.sem_flg = 0;

    pthread_mutex_lock(&s->lock);
    if (s->val == 1) {
        pthread_mutex_unlock(&s->lock);
        return 0;
    }
    ++s->val;
    pthread_mutex_unlock(&s->lock);
    semop(s->id, &op, 1);

    return 1;
}

int mysem_destroy(const mysem_t *s) {
    if (mysem_get(s) == -1)
        return -1;
    mysem_remove(s);
    semctl(s->id, 0, IPC_RMID);
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