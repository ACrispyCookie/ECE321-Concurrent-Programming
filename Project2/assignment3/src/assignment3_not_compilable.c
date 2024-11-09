//
// Created by panag on 11/7/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../../assignment1/src/mysem.h"
#define N 2
#define SLEEP_TIME 2

typedef enum {RED, BLUE} car_color;

typedef struct shared_thread_mem {
    unsigned int cars_on_bridge;
    unsigned int passed_red_cars;
    unsigned int passed_blue_cars;
    unsigned int drive_time;
    car_color bridge_color;
    mysem_t *check_red;
    mysem_t *check_blue;
    mysem_t *blue_q;
    mysem_t *bridge;
    mysem_t *red_q;
} ctl_thr;

typedef struct input {
    ctl_thr *in;
    int id;
} inT;

void *red_car(void *args);
void *blue_car(void *args);

inT *create_ctl_thr (const int id, const double sleep_time) {
    inT *input = (inT*) calloc(1, sizeof(inT));
    mysem_t *red_q = (mysem_t*) calloc(1, sizeof(mysem_t));
    mysem_t *blue_q = (mysem_t*) calloc(1, sizeof(mysem_t));
    mysem_t *bridge = (mysem_t*) calloc(1, sizeof(mysem_t));
    mysem_t *check_red = (mysem_t*) calloc(1, sizeof(mysem_t));
    mysem_t *check_blue = (mysem_t*) calloc(1, sizeof(mysem_t));
    ctl_thr *control_signals = (ctl_thr*) calloc(1, sizeof(ctl_thr));
    bridge->id = -1; blue_q->id = -1; red_q->id = -1;

    control_signals->check_red = check_red;
    control_signals->check_blue = check_blue;
    control_signals->blue_q = blue_q;
    control_signals->bridge = bridge;
    control_signals->red_q = red_q;
    control_signals->bridge_color = RED;
    control_signals->cars_on_bridge = 0;
    control_signals->passed_blue_cars = 0;
    control_signals->passed_red_cars = 0;
    control_signals->drive_time = 2;

    input->in = control_signals;
    input->id = id;

    return input;
}

pthread_t **create_cars (const unsigned num, car_color type, int *array_size) {
    pthread_t **thread_keys = (pthread_t**) calloc(1, sizeof(pthread_t**));
    for (int i = 0; i < num; i++) {
        thread_keys[i] = (pthread_t*) calloc(1, sizeof(pthread_t));
        if (type == RED) pthread_create(thread_keys[i], NULL, red_car, (void*) create_ctl_thr(i, SLEEP_TIME));
        else pthread_create(thread_keys[i], NULL, blue_car, (void*) create_ctl_thr(i, SLEEP_TIME));
    }

}

int main (int argc, char *argv[]) {

    pthread_t t1, t2, t3, t4;
    inT input, input1, input2, input3;

    int red_res = mysem_init(&red_q, 1);
    int blue_res = mysem_init(&blue_q, 1);
    int bridge_res = mysem_init(&bridge, 1);
    int check_res = mysem_init(&check_red, 1);
    int check_re2s = mysem_init(&check_blue, 1);

    // Initialise struct thread in
    {
        thread_mem.check_red = &check_red;
        thread_mem.check_blue = &check_blue;
        thread_mem.blue_q = &blue_q;
        thread_mem.bridge = &bridge;
        thread_mem.red_q = &red_q;
        thread_mem.bridge_color = RED;
        thread_mem.cars_on_bridge = 0;
        thread_mem.passed_blue_cars = 0;
        thread_mem.passed_red_cars = 0;
        thread_mem.drive_time = 2;
    }

    // initialise id
    {
        input.in = &thread_mem;
        input.threadid = 1;
    }

    {
        input1.in = &thread_mem;
        input1.threadid = 2;
    }

    {
        input2.in = &thread_mem;
        input2.threadid = 3;
    }

    {
        input3.in = &thread_mem;
        input3.threadid = 4;
    }

    printf("Sem: RED_Q(%d), BLUE_Q(%d), BRIDGE(%d)\n", red_res, blue_res, bridge_res);
    pthread_create(&t1, NULL, red_car, &input);
    pthread_create(&t2, NULL, blue_car, &input1);
    pthread_create(&t3, NULL, red_car, &input2);
    pthread_create(&t4, NULL, blue_car, &input3);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    mysem_destroy(&red_q);
    mysem_destroy(&blue_q);
    mysem_destroy(&bridge);
    mysem_destroy(&check_red);
    mysem_destroy(&check_blue);
}

void *red_car(void *args) {
    inT *self = (inT*) args;
    mysem_t *check_red = self->in->check_red;
    mysem_t *bridge = self->in->bridge;
    mysem_t *red_q = self->in->red_q;
    thread_input *input = self->in;

    mysem_down(red_q);
    mysem_down(check_red);
    printf("Red#%d: Accessing Bridge\n", self->threadid);
    if (input->passed_red_cars == 0) {
        printf("Red#%d: I was first!\n", self->threadid);
        mysem_down(bridge);
        printf("Red#%d: Now in Bridge! \n", self->threadid);
    }
    ++input->passed_red_cars;
    if (input->passed_red_cars < N) {
        mysem_up(red_q);
        printf("Red#%d: I Signal red behind to start! [%d]\n", self->threadid, input->passed_red_cars);
    }

    mysem_up(check_red);

    printf("Red#%d: I am starting to cross\n", self->threadid);
    sleep(input->drive_time);
    printf("Red#%d: Reached the End\n", self->threadid);

    mysem_down(check_red);
    if (--input->passed_red_cars == 0) {
        printf("Red#%d: I am last! Opening bridge. <Cars on bridge: %d>\n", self->threadid, input->cars_on_bridge);
        mysem_up(bridge);
        mysem_up(red_q);
    }
    mysem_up(check_red);

    return 0;
}

void *blue_car(void *args) {
    inT *self = (inT*) args;
    mysem_t *check_blue = self->in->check_blue;
    mysem_t *bridge = self->in->bridge;
    mysem_t *blue_q = self->in->blue_q;
    thread_input *input = self->in;

    mysem_down(blue_q);
    mysem_down(check_blue);
    printf("Blue#%d: Accessing Bridge\n", self->threadid);
    if (input->passed_blue_cars == 0) {
        printf("Blue#%d: I was first! \n", self->threadid);
        mysem_down(bridge);
        printf("Blue#%d: Now in Bridge! \n", self->threadid);
    }
    ++input->passed_blue_cars;
    if (input->passed_blue_cars < N) {
        mysem_up(blue_q);
        printf("Blue#%d: I Signal blue behind to start! [%d]\n", self->threadid, input->passed_blue_cars);
    }
    mysem_up(check_blue);

    printf("Blue#%d: I am starting to cross\n", self->threadid);
    sleep(input->drive_time);
    printf("Blue#%d: Reached the End\n", self->threadid);

    mysem_down(check_blue);
    if (--input->passed_blue_cars == 0) {
        printf("Blue#%d: I am last! Opening bridge. <Cars on bridge: %d> \n", self->threadid, input->cars_on_bridge);
        mysem_up(bridge);
        mysem_up(blue_q);
    }
    mysem_up(check_blue);

    return 0;
}