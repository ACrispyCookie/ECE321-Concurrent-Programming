#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define WAIT_TIME 1
#define ALARM SIGALRM
#define TIMER ITIMER_REAL

uint64_t timestamp;
static struct itimerval default_alarm, disarmed_alarm;

uint64_t GetTimerCountInNS(void) {
   struct timespec currenttime;
   clock_gettime(CLOCK_REALTIME, &currenttime);
   return UINT64_C(1000000000) * currenttime.tv_sec + currenttime.tv_nsec;
}

void thread_timeout_handler(int signum) {
    setitimer(TIMER, &disarmed_alarm, NULL);
    printf("time passed1: %lu\n", GetTimerCountInNS() - timestamp);
    timestamp = GetTimerCountInNS();
    printf("time passed2: %lu\n", GetTimerCountInNS() - timestamp);
    printf("exit\n");
    fflush(stdout);
    // setitimer(TIMER, &disarmed_alarm, NULL);
    // setitimer(TIMER, &default_alarm, NULL);
}

int main(int argc, char *argv[]) {
    default_alarm.it_interval.tv_sec = 0;
    default_alarm.it_interval.tv_usec = WAIT_TIME;
    default_alarm.it_value.tv_sec = 0;
    default_alarm.it_value.tv_usec = WAIT_TIME;

    disarmed_alarm.it_interval.tv_sec = 0;
    disarmed_alarm.it_interval.tv_usec = 0;
    disarmed_alarm.it_value.tv_sec = 0;
    disarmed_alarm.it_value.tv_usec = 0;

    struct sigaction alarm_action;
    sigset_t blocked_sigs;
    sigemptyset(&blocked_sigs);
    alarm_action.sa_handler = thread_timeout_handler;
    alarm_action.sa_mask = blocked_sigs;
    alarm_action.sa_flags = 0;
    sigaction(ALARM, &alarm_action, NULL);
    
    timestamp = GetTimerCountInNS();
    setitimer(TIMER, &default_alarm, NULL); 
    int i = 0;
    while (i < 1000000000)
        i++;
    return 0;
}

