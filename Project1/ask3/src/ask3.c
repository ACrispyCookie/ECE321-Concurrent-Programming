#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/*
    Struct used to pass the information to each
    new thread about the sorting.

    Fields:
    pthread_t tid - the id of the new thread.
    int start - the index at which to start sorting the file
    int end - the index at which to stop sorting the file
    int running - the current status of the thread 
*/
typedef struct sort_info {
    pthread_t tid;
    int start;
    int end;
    int running;
} sort_info_t;

static char *file_name;
/*
    Runs the logic of one thread. If the part of the file it needs to sort
    is less than 64 integers it uses quick sort and then writes the 
    updated part to the file or else it recursively creates 2 new threads.

    Parameters:
    void *arg - a pointer to sort_info_t that describes what part
                of the array the thread is responsible for sorting.
    Returns:
    NULL since the return value is ignored.

*/
void *run_thread(void *arg);

/*
    Splits the given array in half and creates 2 new threads
    that run external merge sort on the two halves.

    Parameters:
    int start - the start of array to split.
    int end - the end of the array to split.

*/
void create_threads_rec(int start, int end);

/*
    Merges 2 sorted parts of integers on the file.

    Parameters:
    int start1 - the start of the first array of integers.
    int start2 - the start of the second array of integers.
    int size1 - the size of the first array.
    int size2 - the size of the second array.

*/
void merge_file_array(int start1, int start2, int size1, int size2);

/*
    Reads integers from the file and writes them to the given buffer.

    Parameters:
    int *buffer - the buffer to write the integers.
    int start - the position at which to start reading from the file.
    int element_count - the number of integers to read.

*/
size_t read_from_file(int *buffer, int start, int element_count);

/*
    Writes a buffer of integers to the file.

    Parameters:
    int *buffer - the buffer containing the integers to write.
    int start - the position at which to start writing on the file.
    int element_count - the number of integers in the buffer.

*/
size_t write_to_file(const int *buffer, int start, int element_count);

/*
    Prints an array of integers seperated by commas.

    Parameters:
    int *array - the array of integers to print
    int size - the size of the array
*/
void print_array(const int *array, int size);

/*
    Simple comparator used for the quick sort.

    Returns:
    the difference int1 - int2.
*/
int qsort_comparator(const void *int1, const void *int2);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file name>\n", argv[0]);
        return -1;
    }

    // Create file name and find file size
    file_name = malloc((strlen(argv[1]) + 1) * sizeof(char));
    strcpy(file_name, argv[1]);
    FILE *file_to_read = fopen(file_name, "r");
    if (file_to_read == NULL) {
        printf("Error opening file!");
        return -1;
    }
    fseek(file_to_read, 0, SEEK_END);
    const int file_size = ftell(file_to_read);
    fclose(file_to_read);

    // Start the external merge sort
    const int size = file_size / sizeof(int);
    const int mid = size / 2;
    create_threads_rec(0, size - 1);
    merge_file_array(0, mid, mid, size - mid);
    return 0;
}

void *run_thread(void *arg) {
    sort_info_t *info = arg;
    const int size = info->end - info->start + 1;
    if (size <= 64) {
        int *buffer = malloc(size * sizeof(int));
        read_from_file(buffer, info->start, size);
        qsort(buffer, size, sizeof(int), qsort_comparator);
        write_to_file(buffer, info->start, size);
        free(buffer);
        info->running = 0;
        return NULL;
    }

    const int mid = size / 2;
    create_threads_rec(info->start, info->end);
    merge_file_array(info->start, info->start + mid, mid, size - mid);
    info->running = 0;
    return NULL;
}

void create_threads_rec(const int start, const int end) {
    const int size = end - start + 1;
    const int mid = start + size / 2;
    sort_info_t *info1 = malloc(sizeof(sort_info_t));
    info1->start = start;
    info1->end = mid - 1;
    info1->running = 1;
    sort_info_t *info2 = malloc(sizeof(sort_info_t));
    info2->start = mid;
    info2->end = end;
    info2->running = 1;
    pthread_create(&info1->tid, NULL, run_thread, info1);
    pthread_create(&info2->tid, NULL, run_thread, info2);
    while (info1->running || info2->running) {}

    free(info1);
    free(info2);
}

void merge_file_array(const int start1, const int start2, const int size1, const int size2) {
    int i = 0, j = 0, k = 0;
    int *array1 = malloc(size1 * sizeof(int));
    int *array2 = malloc(size2 * sizeof(int));
    int *array3 = malloc((size1 + size2) * sizeof(int));
    read_from_file(array1, start1, size1);
    read_from_file(array2, start2, size2);

    while (i < size1 && j < size2) {
        if (array1[i] < array2[j])
            array3[k++] = array1[i++];
        else
            array3[k++] = array2[j++];
    }

    while (i < size1)
        array3[k++] = array1[i++];

    while (j < size2)
        array3[k++] = array2[j++];

    write_to_file(array3, start1, size1 + size2);
    free(array1);
    free(array2);
    free(array3);
}

void print_array(const int *array, const int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", *(array + i));
    }
    printf("\n");
}

size_t read_from_file(int *buffer, const int start, const int element_count) {
    FILE *file = fopen(file_name, "r");
    fseek(file, (long) start * sizeof(int), SEEK_SET);
    const size_t bytes_read = fread(buffer, sizeof(int), element_count, file);
    fclose(file);
    return bytes_read;
}

size_t write_to_file(const int *buffer, const int start, const int element_count) {
    FILE *file = fopen(file_name, "r+");
    fseek(file, (long) start * sizeof(int), SEEK_SET);
    const size_t bytes_read = fwrite(buffer, sizeof(int), element_count, file);
    fclose(file);
    return bytes_read;
}

int qsort_comparator(const void *int1, const void *int2) {
    return *(int *) int1 - *(int *) int2;
}