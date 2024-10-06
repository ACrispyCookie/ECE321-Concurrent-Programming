#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/pipes.h"

volatile unsigned int pipe_id1, pipe_id2;
volatile unsigned int thread1_done = 0, thread2_done = 0;

void *thr1(void *filename);
void *thr2(void *filename);
size_t write_file_to_pipe(const char *filename, unsigned int pipe_id);
size_t read_file_from_pipe(const char *filename, unsigned int pipe_id);


int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./test1 <file name>\n");
        return 1;
    }

    pipe_id1 = pipe_open(64);
    pipe_id2 = pipe_open(64);
    pthread_t thread1, thread2;

    const int res1 = pthread_create(&thread1, NULL, thr1, argv[1]);
    if (res1) {
        printf("Failed to create thread 1: %d", res1);
    }

    const int res2 = pthread_create(&thread2, NULL, thr2, argv[1]);
    if (res2) {
        printf("Failed to create thread 2: %d", res2);
    }
    while(!thread1_done || !thread2_done) {}

    return 0;
}

void *thr1(void *filename) {
    write_file_to_pipe(filename, pipe_id1);

    char *copy2_name = malloc((strlen(filename) + 6) * sizeof(char));
    strcpy(copy2_name, filename);
    strcat(copy2_name, ".copy2");
    read_file_from_pipe(copy2_name, pipe_id2);

    free(copy2_name);
    thread1_done = 1;
    return NULL;
}

void *thr2(void *filename) {
    char *copy_name = malloc((strlen(filename) + 5) * sizeof(char));
    strcpy(copy_name, filename);
    strcat(copy_name, ".copy");
    read_file_from_pipe(copy_name, pipe_id1);
    write_file_to_pipe(copy_name, pipe_id2);

    free(copy_name);
    thread2_done = 1;
    return NULL;
}

size_t write_file_to_pipe(const char *filename, const unsigned int pipe_id) {
    char buffer;
    size_t bytes_read = 0;
    size_t total_bytes = 0;
    FILE *file = fopen(filename, "r");
    while(1) {
        bytes_read = fread(&buffer, sizeof(char), 1, file);
        total_bytes += bytes_read;
        if (bytes_read == 0)
            break;
        pipe_write(pipe_id, buffer);
    }
    pipe_writeDone(pipe_id);
    fclose(file);
    return total_bytes;
}

size_t read_file_from_pipe(const char *filename, const unsigned int pipe_id) {
    char buffer;
    size_t total_bytes = 0;
    FILE *file = fopen(filename, "w");
    while (1) {
        const int read_res = pipe_read(pipe_id, &buffer);
        if (read_res == 0)
            break;
        fwrite(&buffer, sizeof(char), 1, file);
        ++total_bytes;
    }
    fclose(file);
    return total_bytes;
}