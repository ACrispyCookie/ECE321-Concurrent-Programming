#include "mysem.h"

static pthread_mutex_t mutex;
static int tickets[100] = {0};
static bool taking[100] = {false};
static int thread_count = 0;

int mysem_init(mysem_t *s, int n) {
  if(s->initialized == 1)
    return -1;
  if (!(n == 1 || n == 0))
      return 0;

  pthread_mutex_init(&mutex, NULL);
  semctl(s->id, 0, SETVAL, n);
  s->initialized = 1;

  return 1;
}

int mysem_down(mysem_t *s) {
  struct sembuf op;

  if(!s->initialized)
    return -1;

  thread_count++;
  int i = thread_count - 1;
  /*******/
  /* no new threads should be added in the queue
   during this part of code */ 
  taking[i] = true;
  tickets[i] = max(tickets, thread_count) + 1;
  taking[i] = false;

  for (int j=0; j < thread_count; j++) {
    while (taking[j]) {}
    while (priority(i, j)) {}
  }
  // must be executed with priority
  op.sem_num = 0; op.sem_op = -1; op.sem_flg = 0;
  semop(s->id, &op, 1);

  tickets[i] = 0;
  /*******/

  return 1;
}

int mysem_up(mysem_t *s) {
  struct sembuf op;

  if(!s->initialized)
    return -1;
  if(semctl(s->id, 0, GETVAL) == 1)
    return 0;

  thread_count++;
  int i = thread_count - 1;
  /*******/
  /* no new threads should be added in the queue
   during this part of code */ 
  taking[i] = true;
  tickets[i] = max(tickets, thread_count) + 1;
  taking[i] = false;

  for (int j=0; j < thread_count; j++) {
    while (taking[j]) {}
    while (priority(i, j)) {}
  }
  // must be executed with priority
  op.sem_num = 0; op.sem_op = 1; op.sem_flg = 0;
  semop(s->id, &op, 1);

  tickets[i] = 0;
  /*******/

  return 1;
}

int mysem_destroy(mysem_t *s) {
  if(!s->initialized || semctl(s->id, 0, GETVAL) == -1)
    return -1;

  pthread_mutex_destroy(&mutex);
  semctl(s->id, 0, IPC_RMID);
  return 1;
}

// int add_thread_2q() {
//   thread_count++;
//   int *new_tickets = (int *) realloc(tickets, thread_count * sizeof(int *));
//   if (new_tickets == NULL)
//     return -1;
//   tickets = new_tickets;
//   tickets[thread_count - 1] = 0;
  
//   bool *new_taking = (bool *) realloc(taking, thread_count * sizeof(bool *));
//   if (new_taking == NULL)
//     return -1;
//   taking = new_taking;
//   taking[thread_count - 1] = false;
  
//   return 0;
// }

// int remove_thread_from_q(int thread_index) {
//   if (tickets == NULL && taking == NULL)
//     return 1;

//   tickets[thread_index] = tickets[thread_count - 1];
//   taking[thread_index] = taking[thread_count - 1];
//   thread_count--;
//   int *new_tickets = (int *) realloc(tickets, thread_count * sizeof(int *));
//   if (new_tickets == NULL)
//     return -1;
//   tickets = new_tickets;

//   bool *new_taking = (bool *) realloc(taking, thread_count * sizeof(bool *));
//   if (new_taking == NULL)
//     return -1;
//   taking = new_taking;
  
//   return 0;
// }

/* finds if thread with index self has priority over 
  a competing thread with index comp */ 
bool priority(int self, int comp) {
  // self has priority
  if(tickets[comp] == 0)
    return false;
  // self has priority
  else if (tickets[self] < tickets[comp])
    return false;
  // compiting thread i has priority
  else if (tickets[self] > tickets[comp])
    return true;
  // tie. Index of buffer is the critirium
  else
    return (self > comp);
}

int max(int *buffer, int size) {
  int tmp = 0;

  for (int i = 0; i < size; i++) {
    if (buffer[tmp] < buffer[i])
      tmp = i;
  }

  return (buffer[tmp]);
}