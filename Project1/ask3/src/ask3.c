#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct sort_info {
    pthread_t tid;
    int start;
    int end;
    int running;
} sort_info_t;

static char *file_name;
volatile int started = 0;
volatile int completed = 0;

void *run_thread(void *arg);
void create_threads_rec(int start, int end);
void merge_arrays(int array1_start, int array2_start, int array3_start, int size1, int size2);
size_t read_from_file(int *buffer, int start, int element_count);
size_t write_to_file(const int *buffer, int start, int element_count);
void print_array(const int *array, int size);
int qsort_comparator(const void *int1, const void *int2);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ask3 <file name>\n");
        return -1;
    }

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

    const int size = file_size / sizeof(int);
    const int mid = size / 2;
    create_threads_rec(0, size - 1);
    merge_arrays(0, mid, 0, mid, size - mid);
    return 0;
}

void *run_thread(void *arg) {
    sort_info_t *info = arg;
    ++started;
    printf("Started: %d/%d\n", started, TOTAL_NODES);
    const int size = info->end - info->start + 1;
    if (size <= 64) {
        int *buffer = malloc(size * sizeof(int));
        read_from_file(buffer, info->start, size);
        qsort(buffer, size, sizeof(int), qsort_comparator);
        write_to_file(buffer, info->start, size);
        free(buffer);
        ++completed;
        printf("Completed: %d/%d\n", completed, TOTAL_NODES);
        info->running = 0;
        return NULL;
    }

    const int mid = size / 2;
    create_threads_rec(info->start, info->end);
    merge_arrays(info->start, info->start + mid, info->start, mid, size - mid);
    ++completed;
    printf("Completed: %d/%d\n", completed, TOTAL_NODES);
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

void merge_arrays(const int array1_start, const int array2_start, const int array3_start, const int size1, const int size2) {
    int i = 0, j = 0, k = 0;
    int *array1 = malloc(size1 * sizeof(int));
    int *array2 = malloc(size2 * sizeof(int));
    int *array3 = malloc((size1 + size2) * sizeof(int));
    read_from_file(array1, array1_start, size1);
    read_from_file(array2, array2_start, size2);

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

    write_to_file(array3, array3_start, size1 + size2);
    // printf("---------------------------\n");
    // print_array(array1, size1);
    // print_array(array2, size2);
    // print_array(array3, size1 + size2);
    // printf("---------------------------\n");
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