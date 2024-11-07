#ifndef _MYSEM_H_
#define _MYSEM_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

typedef struct mysem {
    int id;
    bool initialized;
} mysem_t;

int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(mysem_t *s);
bool priority(int self, int comp);
int max(int *buffer, int size);

#endif