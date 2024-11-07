#ifndef _MYSEM_H_
#define _MYSEM_H_
#include <pthread.h>

typedef struct mysem {
    unsigned int id;
    int val;
    pthread_mutex_t lock;
} mysem_t;

int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(const mysem_t *s);

#endif