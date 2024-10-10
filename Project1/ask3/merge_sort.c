#include <stdio.h>
#include <stdlib.h>

void merge_sort(FILE *file, int start, int end);
void merge_arrays(const int array1_start, const int array2_start, const int array3_start, int size1, const int size2);
size_t read_from_file(int *buffer, const int start, const int element_count);
size_t write_to_file(const int *buffer, const int start, const int element_count);

FILE *file;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./merge_sort <filename>\n");
    }

    file = fopen(argv[1], "r");
}

void merge_sort(int start, int end) {
    const int size = end - start + 1;
    const int mid = start + size / 2;

    if (size == 2) {

    }
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

size_t read_from_file(int *buffer, const int start, const int element_count) {
    fseek(file, (long) start * sizeof(int), SEEK_SET);
    const size_t bytes_read = fread(buffer, sizeof(int), element_count, file);
    fclose(file);
    return bytes_read;
}
size_t write_to_file(const int *buffer, const int start, const int element_count) {
    fseek(file, (long) start * sizeof(int), SEEK_SET);
    const size_t bytes_read = fwrite(buffer, sizeof(int), element_count, file);
    fclose(file);
    return bytes_read;
}
