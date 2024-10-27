#include "mysem.h"

int main(void) {
  mysem_t sem;

  sem.initialized = 0;
  sem.id = semget(IPC_PRIVATE,1,S_IRWXU);
  int res = mysem_init(&sem, 1);
  printf("%d\n", res);

  res = mysem_up(&sem);
  printf("%d Woke up\n",res);
  res = mysem_up(&sem);
  printf("%d Woke up\n",res);
  res = mysem_up(&sem);
  printf("%d Woke up\n",res);

  return 0;
}