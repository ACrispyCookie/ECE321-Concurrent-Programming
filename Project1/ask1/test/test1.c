#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pipes.h"

typedef struct copier {
    unsigned int pipe_id1, pipe_id2;
    char *filename, *filename_copy, *filename_copy2;
    int done1, done2;
} copier_t;

void *thr1(void *arg);
void *thr2(void *arg);
size_t write_file_to_pipe(const char *filename, unsigned int pipe_id);
size_t write_pipe_to_file(unsigned int pipe_id, const char *filename);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./test1 <file name>\n");
        return 1;
    }

    copier_t *copier = malloc(sizeof(copier_t));
    copier->pipe_id2 = pipe_open(64);
    copier->pipe_id1 = pipe_open(64);
    copier->done1 = 0;
    copier->done2 = 0;
    copier->filename = argv[1];
    char *filename_copy = malloc((strlen(argv[1]) + 6) * sizeof(char));
    char *filename_copy2 = malloc((strlen(argv[1]) + 7) * sizeof(char));
    strcpy(filename_copy, argv[1]);
    strcpy(filename_copy2, argv[1]);
    strcat(filename_copy, ".copy");
    strcat(filename_copy2, ".copy2");
    copier->filename_copy = filename_copy;
    copier->filename_copy2 = filename_copy2;

    pthread_t thread1, thread2;
    const int res1 = pthread_create(&thread1, NULL, thr1, copier);
    if (res1)
        printf("Failed to create thread 1: %d", res1);

    const int res2 = pthread_create(&thread2, NULL, thr2, copier);
    if (res2)
        printf("Failed to create thread 2: %d", res2);
    while(!copier->done1 || !copier->done2) {}

    free(filename_copy);
    free(filename_copy2);
    free(copier);
    return 0;
}

void *thr1(void *arg) {
    copier_t *copier = arg;
    write_file_to_pipe(copier->filename, copier->pipe_id1);
    //write_pipe_to_file(copier->pipe_id2, copier->filename_copy2);

    copier->done1 = 1;
    return NULL;
}

void *thr2(void *arg) {
    copier_t *copier = arg;
    write_pipe_to_file(copier->pipe_id1, copier->filename_copy);
    //write_file_to_pipe(copier->filename_copy, copier->pipe_id2);

    copier->done2 = 1;
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

size_t write_pipe_to_file(const unsigned int pipe_id, const char *filename) {
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
