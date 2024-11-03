#include "mysem.h"

typedef struct thred_info {
  mysem_t sem;
  pthread_t ptid;
} thread_info_t;

void init_out(int res, int semid);
void destroy_out(int res, int semid);
void down_out(int res, int semid, long unsigned int ptid);
void up_out(int res, int semid, long unsigned int ptid);
void *thread1(void *arg);
void *thread2(void *arg);
pthread_t init_thread(mysem_t sem);

int main(int argc, char *argv[]) {
  mysem_t sem;

  sem.initialized = 0;
  sem.id = semget(IPC_PRIVATE, 1, S_IRWXU);

  int val = mysem_init(&sem, 1);
  init_out(val, sem.id);

  thread_info_t *p1 = (thread_info_t *) malloc(sizeof(thread_info_t));
  p1->sem = sem;
  int rc = pthread_create(&(p1->ptid), NULL, (void *)thread1, p1);

  thread_info_t *p2 = (thread_info_t *) malloc(sizeof(thread_info_t));
  p2->sem = sem;
  rc = pthread_create(&(p2->ptid), NULL, (void *)thread1, p2);

  thread_info_t *p3 = (thread_info_t *) malloc(sizeof(thread_info_t));
  p3->sem = sem;
  rc = pthread_create(&(p3->ptid), NULL, (void *)thread2, p3);

  thread_info_t *p4 = (thread_info_t *) malloc(sizeof(thread_info_t));
  p4->sem = sem;
  rc = pthread_create(&(p4->ptid), NULL, (void *)thread2, p4);

  pthread_join(p1->ptid, NULL);
  pthread_join(p2->ptid, NULL);
  pthread_join(p3->ptid, NULL);
  pthread_join(p4->ptid, NULL);

  val = mysem_destroy(&sem);
  destroy_out(val, sem.id);

  free(p1);
  free(p2);
  return 0;
}

// pthread_t init_thread(mysem_t sem) {
//   thread_info_t *p;

//   p = (thread_info_t *) malloc(sizeof(thread_info_t));
//   p->sem = sem;

//   int rc = pthread_create(&(p->ptid), NULL, (void *)thread, p);
//   if (rc) {
//     printf("Error creating thread\n");
// //    exit(1);
//   }
//   else {
//     printf("Thread %ld created successfully\n", p->ptid);
//   }

//   return p->ptid;
// }

void *thread1(void *arg) {
  thread_info_t *info = (thread_info_t *)arg;
  printf("Thread %ld STARTED.\n", info->ptid);

  printf("Thread %ld before down() with sem %d \n", info->ptid, semctl(info->sem.id, 0, GETVAL));
  int val = mysem_down(&(info->sem), info->ptid);
  printf("Thread %ld after down() with sem %d \n", info->ptid, semctl(info->sem.id, 0, GETVAL));
  down_out(val, info->sem.id, info->ptid);

  return NULL;
}

void *thread2(void *arg) {
  thread_info_t *info = (thread_info_t *)arg;
  printf("Thread %ld STARTED.\n", info->ptid);

  printf("Thread %ld before up() with sem %d \n", info->ptid, semctl(info->sem.id, 0, GETVAL));
  int val = mysem_up(&(info->sem), info->ptid);
  printf("Thread %ld after up() with sem %d \n", info->ptid, semctl(info->sem.id, 0, GETVAL));
  up_out(val, info->sem.id, info->ptid);

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
