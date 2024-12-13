#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pipes.h"

/*
    A helper struct used to pass important
    data used by the two threads.

    Fields:
    unsigned int pipe_id1 - The id of the first pipe to be used.
    unsigned int pipe_id2 - The id of the second pipe to be used.
    char *filename - The source file to write to the first pipe.
    char *filename_copy - The first copy to be created by reading the first pipe.
    char *filename_copy2 - The second copy to be created by reading the second pipe.
*/
typedef struct copier {
    unsigned int pipe_id1, pipe_id2;
    char *filename, *filename_copy, *filename_copy2;
    co_t *main, *co1, *co2;
} copier_t;

/*
    The function run by the first thread created that reads the
    original file and writes its contents to the first pipe. After that
    it reads bytes from the second pipe and writes it to the second copy of 
    the original file.

    Parameters:
    void *arg - A pointer to the copier struct.
*/
void coroutine1(void *arg);

/*
    The function run by the second thread created that reads the
    first pipe and writes its contents to the first copy of the original file.
    After that it reads bytes from the first copy of the original file and writes 
    its contents to the second pipe.

    Parameters:
    void *arg - A pointer to the copier struct.
*/
void coroutine2(void *arg);

/*
    Reads a file with the given name and writes it
    byte by byte to the given pipe.

    Parameters:
    char *filename - The name of the file to read.
    unsigned int pipe_id - The pipe to write the file contents to.
    co_t *fallback - The coroutine to switch to if there is no work to be done.
    Returns:
    The number of total bytes read from the file.
*/
size_t write_file_to_pipe(const char *filename, unsigned int pipe_id, co_t *fallback);

/*
    Reads a file from the given pipe and writes it to
    a new file with the given name.

    Parameters:
    unsigned int pipe_id - The pipe to read the file from.
    char *filename - The name of the file to create and write the pipe contents to.
    co_t *fallback - The coroutine to switch to if there is no work to be done.
    Returns:
    The number of total bytes read from the pipe.
*/
size_t write_pipe_to_file(unsigned int pipe_id, const char *filename, co_t *fallback);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file name>\n", argv[0]);
        return 1;
    }

    co_t main, co1, co2;
    main.id = -1; co1.id = -1; co2.id = -1;
    copier_t *copier = malloc(sizeof(copier_t));
    copier->pipe_id2 = pipe_open(64);
    copier->pipe_id1 = pipe_open(64);
    copier->filename = argv[1];
    char *filename_copy = malloc((strlen(argv[1]) + 6) * sizeof(char));
    char *filename_copy2 = malloc((strlen(argv[1]) + 7) * sizeof(char));
    strcpy(filename_copy, argv[1]);
    strcpy(filename_copy2, argv[1]);
    strcat(filename_copy, ".copy");
    strcat(filename_copy2, ".copy2");
    copier->filename_copy = filename_copy;
    copier->filename_copy2 = filename_copy2;
    copier->main = &main;
    copier->co1 = &co1;
    copier->co2 = &co2;
    mycoroutine_init(&main);

    const int res1 = mycoroutine_create(&co1, coroutine1, copier);
    if (res1 == -1)
        printf("Failed to create coroutine 1: %d\n", res1);

    const int res2 = mycoroutine_create(&co2, coroutine2, copier);
    if (res2 == -1)
        printf("Failed to create coroutine 2: %d\n", res2);

    mycoroutine_switchto(&co1);
    mycoroutine_destroy(&co1);
    mycoroutine_destroy(&co2);
    mycoroutine_destroy(&main);
    free(filename_copy);
    free(filename_copy2);
    free(copier);
    return 0;
}

void coroutine1(void *arg) {
    copier_t *copier = arg;
    write_file_to_pipe(copier->filename, copier->pipe_id1, copier->co2);
    printf("Thread1: Switching to Thread2 to start the writing process of the second file!\n");
    mycoroutine_switchto(copier->co2);
    write_pipe_to_file(copier->pipe_id2, copier->filename_copy2, copier->co2);
    printf("Thread1: Job done! Switching to main!\n");
    mycoroutine_switchto(copier->main);
}

void coroutine2(void *arg) {
    copier_t *copier = arg;
    write_pipe_to_file(copier->pipe_id1, copier->filename_copy, copier->co1);
    write_file_to_pipe(copier->filename_copy, copier->pipe_id2, copier->co1);
    printf("Thread2: Job done! Switching to Thread1!\n");
    mycoroutine_switchto(copier->co1);
}

size_t write_file_to_pipe(const char *filename, const unsigned int pipe_id, co_t *fallback) {
    char buffer;
    size_t bytes_read = 0;
    size_t total_bytes = 0;
    FILE *file = fopen(filename, "r");
    while(1) {
        bytes_read = fread(&buffer, sizeof(char), 1, file);
        total_bytes += bytes_read;
        if (bytes_read == 0)
            break;
        pipe_write(pipe_id, buffer, fallback);
    }
    pipe_writeDone(pipe_id);
    fclose(file);
    return total_bytes;
}

size_t write_pipe_to_file(const unsigned int pipe_id, const char *filename, co_t *fallback) {
    char buffer;
    size_t total_bytes = 0;
    FILE *file = fopen(filename, "w");
    while (1) {
        const int read_res = pipe_read(pipe_id, &buffer, fallback);
        if (read_res == 0)
            break;
        fwrite(&buffer, sizeof(char), 1, file);
        ++total_bytes;
    }
    fclose(file);
    return total_bytes;
}
