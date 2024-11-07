#include <stdio.h>
#include <unistd.h>
#include "../assingment1/mysem.h"

#define MAX_CARS 5

void *red_car(void *arg);
void *blue_car(void *arg);
void create_car(char color);

typedef struct thread_info {
    pthread_t ptid;
} thread_info_t;

int blue_count = 0;
int red_count = 0;

mysem_t red_mtx;
mysem_t blue_mtx;

int red_passed = 0;
int blue_passed = 0;

int main(int argc, char *argv[]) {
    char color;

    mysem_init(&red_mtx, 1);
    mysem_init(&blue_mtx, 1);

    while (1) {
        const int read_result = scanf("%c", &color);
        if (read_result == EOF)
            break;
        create_car(color);
    }
}

void create_car(char color) {
    thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));

    if (color == 'r') {
        int rc = pthread_create(&(info->ptid), NULL, red_car, info);
        if (!rc) 
            printf("Successfully created a red car thread %ld\n", info->ptid);
    }
    else if (color == 'b') {
        int rc = pthread_create(&(info->ptid), NULL, blue_car, info);
        if (!rc) 
            printf("Successfully created a blue car thread %ld\n", info->ptid);
    }
    else
        printf("Invalid color\n");
}

void *red_car(void *arg) {
    thread_info_t info = *(thread_info_t *) arg;

    mysem_down(&red_mtx);
    if (red_count == 0)
        mysem_down(&blue_mtx);
    if(red_count == MAX_CARS)
        mysem_down(&red_mtx);
    if(red_passed == 2*MAX_CARS)
        mysem_down(&red_mtx);
    red_count++;
    mysem_up(&red_mtx);

    // passing bridge
    printf("Red car %ld passing bridge\n", info.ptid);
    sleep(3);
    printf("Red car %ld passed bridge\n", info.ptid);

    mysem_down(&red_mtx);
    if (red_passed == 2*MAX_CARS)
        mysem_up(&blue_mtx);
    if (red_count == MAX_CARS)
        mysem_up(&red_mtx);
    red_count--;
    if (red_count == 0)
        mysem_up(&blue_mtx);
    mysem_up(&red_mtx);

    return NULL;
}

void *blue_car(void *arg) {
    thread_info_t info = *(thread_info_t *) arg;

    mysem_down(&blue_mtx);
    if (blue_count == 0)
        mysem_down(&red_mtx);
    if(blue_count == MAX_CARS)
        mysem_down(&blue_mtx);
    if(blue_passed == 2*MAX_CARS)
        mysem_down(&blue_mtx);
    blue_count++;
    mysem_up(&blue_mtx);

    // passing bridge
    printf("Blue car %ld passing bridge\n", info.ptid);
    sleep(3);
    printf("Blue car %ld passed bridge\n", info.ptid);

    mysem_down(&blue_mtx);
    if (blue_passed == 2*MAX_CARS)
        mysem_up(&red_mtx);
    if (blue_count == MAX_CARS)
        mysem_up(&blue_mtx);
    blue_count--;
    if (blue_count == 0)
        mysem_up(&red_mtx);
    mysem_up(&blue_mtx);

    return NULL;
}