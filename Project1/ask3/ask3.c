#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct sort_info {
    pthread_t tid;
    int start;
    int end;
    int running;
} sort_info_t;

int *array;
int *sorted_array;
int size;

void *run_thread(void *arg);
void create_threads_rec(int start, int end);
void merge_arrays(const int *array1, const int *array2, int *sorted_array, int size1, int size2);
void print_array(const int *array, int size);
int qsort_comparator(const void *int1, const void *int2);

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./ask3 <file name>\n");
        return -1;
    }

    FILE *binary_file = fopen(argv[1], "r");
    fseek(binary_file, 0, SEEK_END);
    const int file_size = ftell(binary_file) + 1;
    fseek(binary_file, 0, SEEK_SET);

    size = file_size / sizeof(int);
    array = malloc(file_size);
    sorted_array = malloc(file_size);
    if (array == NULL || sorted_array == NULL)
        return -1;
    fread(array, file_size, 1, binary_file);
    printf("size: %d\n", file_size);
    printf("Unsorted array:\n");
    print_array(array, size);

    create_threads_rec(0, size - 1);
    merge_arrays(array, array + (size / 2 + 1), sorted_array, size / 2, size - 1);
    printf("Sorted array:\n");
    print_array(sorted_array, size);

    int is_sorted = 1;
    int i_not = 0;
    int element = *sorted_array;
    for (int i = 1; i < size; i++) {
        if (element > *(sorted_array + i)) {
            i_not = i;
            is_sorted = 0;
            break;
        }
    }
    printf("is_sorted: %d, %d\n", is_sorted, i_not);

    free(sorted_array);
    free(array);
    return 0;
}

void *run_thread(void *arg) {
    sort_info_t *info = arg;
    const int size = info->end - info->start + 1;
    if (size <= 64) {
        qsort(array + info->start, size, sizeof(int), qsort_comparator);
        info->running = 0;
        return NULL;
    }
    create_threads_rec(info->start, info->end);
    merge_arrays(array + info->start, array + info->start + size / 2 + 1, sorted_array + info->start,
        size / 2 + 1, info->end - (info->start + size / 2 + 1) + 1);

    return NULL;
}

void create_threads_rec(int start, int end) {
    sort_info_t *info1 = malloc(sizeof(sort_info_t *));
    info1->start = start;
    info1->end = start + size / 2;
    info1->running = 1;
    sort_info_t *info2 = malloc(sizeof(sort_info_t *));
    info2->start = start + size / 2 + 1;
    info2->end = end;
    info2->running = 1;
    pthread_create(&info1->tid, NULL, run_thread, info1);
    pthread_create(&info2->tid, NULL, run_thread, info2);
    while (info1->running || info2->running) {}

    free(info1);
    free(info2);
}

void merge_arrays(const int *array1, const int *array2, int *sorted_array, const int size1, const int size2) {
    int i = 0, j = 0, k = 0;

    while (i < size1 && j < size2) {
        if (array1[i] < array2[j])
            sorted_array[k++] = array1[i++];
        else
            sorted_array[k++] = array2[j++];
    }

    while (i < size1)
        sorted_array[k++] = array1[i++];

    while (j < size2)
        sorted_array[k++] = array2[j++];
}

void print_array(const int *array, const int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", *(array + i));
    }
    printf("\n");
}

int qsort_comparator(const void *int1, const void *int2) {
    return *(int *) int1 - *(int *) int2;
}