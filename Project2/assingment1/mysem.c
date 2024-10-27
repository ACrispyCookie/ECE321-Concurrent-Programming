#include "mysem.h"

static struct sembuf op;

int mysem_init(mysem_t *s, int n) {
  if(s->initialized == 1)
    return -1;
  if (!(n == 1 || n == 0))
      return 0;

  semctl(s->id, 0, SETVAL, n);
  s->initialized = 1;

  return 1;
}

int mysem_down(mysem_t *s) {
  if(!s->initialized)
    return -1;

  op.sem_num = 0; op.sem_op = -1; op.sem_flg = 0;
  semop(s->id, &op, 1);

  return 1;
}

int mysem_up(mysem_t *s) {
  if(!s->initialized)
    return -1;
  if(semctl(s->id, 0, GETVAL) == 1)
    return 0;

  op.sem_num = 0; op.sem_op = 1; op.sem_flg = 0;
  semop(s->id, &op, 1);

  return 1;
}

int mysem_destroy(mysem_t *s) {
   if(!s->initialized || semctl(s->id, 0, GETVAL) == -1)
     return -1;

   semctl(s->id, 0, IPC_RMID);
   return 1;
}
