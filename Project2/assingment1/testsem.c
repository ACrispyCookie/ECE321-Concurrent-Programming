#include "mysem.h"

int thread_count = 0;
int blocked_count = 0;

typedef struct thred_info {
  mysem_t sem;
  pthread_t ptid;
} thread_info_t;

void init_out(int res, int semid);
void destroy_out(int res, int semid);
void down_out(int res, int semid, long unsigned int ptid);
void up_out(int res, int semid, long unsigned int ptid);
void *thread(void *arg);
pthread_t init_thread(mysem_t sem);

int main(int argc, char *argv[]) {
  mysem_t sem;
  pthread_t ptid1, ptid2, ptid3, ptid4;

  sem.initialized = 0;
  sem.id = semget(IPC_PRIVATE, 1, S_IRWXU);

  int val = mysem_init(&sem, 1);
  init_out(val, sem.id);

  ptid1 = init_thread(sem);
  ptid2 = init_thread(sem);
  ptid3 = init_thread(sem);
  ptid4 = init_thread(sem);

  pthread_join(ptid1, NULL);
  pthread_join(ptid2, NULL);
  pthread_join(ptid3, NULL);
  pthread_join(ptid4, NULL);

  val = mysem_destroy(&sem);
  destroy_out(val, sem.id);

  return 0;
}

pthread_t init_thread(mysem_t sem) {
  thread_info_t *p;

  p = (thread_info_t *) malloc(sizeof(thread_info_t));
  p->sem = sem;

  int rc = pthread_create(&(p->ptid), NULL, (void *)thread, p);
  if (rc) {
    printf("Error creating thread\n");
//    exit(1);
  }
  else {
    printf("Thread %ld created successfully\n", p->ptid);
    thread_count++;
  }

  return p->ptid;
}

void *thread(void *arg) {
  thread_info_t *info = (thread_info_t *)arg;

  printf("Thread %ld BLOCKED\n", info->ptid);
  printf("-----------------\n");
  int val = mysem_down(&(info->sem));

  printf("Thread %ld in CS with sem %d\n", info->ptid, semctl(info->sem.id, 0, GETVAL));
//  down_out(val, info.sem.id, info.ptid);
  printf("Thread %ld in CS\n", info->ptid);
  sleep(5);

  val = mysem_up(&(info->sem));
  printf("-----------------\n");
  printf("Thread %ld out of CS with sem %d\n", info->ptid, semctl(info->sem.id, 0, GETVAL));
//  up_out(val, info.sem.id, info.ptid);

  free(info);
  return NULL;
}

void init_out(int res, int semid) {
  if (res == -1) {
    printf("INIT: Sem %d is already initialized to %d\n", semid, semctl(semid, 0, GETVAL));
//    exit(1);
  }
  else if (!res) {
    printf("INIT: Sem %d initialization value must be 0 or 1\n", semid);
//    exit(1);
  }
  else
    printf("INIT: Succesfully initialized sem %d to %d\n", semid, semctl(semid, 0, GETVAL));
}

void destroy_out(int res, int semid) {
  if (res == -1) {
    if (errno == EINVAL) {
      printf("DESTROY: Sem %d is already destroyed\n", semid);
    }
    else
      printf("DESTROY: Sem %d is not initialized\n", semid);
    }
    else
      printf("DESTROY: Succesfully destroyed sem %d\n", semid);
}

void down_out(int res, int semid, pthread_t ptid) {
  if (res == -1)
    printf("DOWN: On thread %ld sem %d is not initialized\n", ptid, semid);
  else
    printf("DOWN: Thread %ld decremented sem %d to %d\n", ptid, semid, semctl(semid, 0, GETVAL));
}

void up_out(int res, int semid, pthread_t ptid) {
  if (res == -1)
    printf("UP: On thread %ld sem %d is not initialized\n", ptid, semid);
  if (!res)
    printf("UP: Thread %ld lost an up call of sem %d, it's value is %d\n", ptid, semid, semctl(semid, 0, GETVAL));
  else
    printf("UP: Thread %ld incremented sem %d to %d\n", ptid, semid, semctl(semid, 0, GETVAL));
}
