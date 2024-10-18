#include <stdbool.h>
#include <stdio.h>

/*
    Checks if a file of integers is sorted or not.

    Returns:
    1, if the file is not sorted or
    0, if the file is sorted.
*/
bool is_sorted(FILE *file);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>", argv[0]);
        return -1;
    }

    FILE *file = fopen(argv[1], "r");
    return is_sorted(file);
}

bool is_sorted(FILE *file) {
    int curInt, nextInt;
    fseek(file, 0, SEEK_END);
    int elements = ftell(file) / sizeof(int);
    fseek(file, 0, SEEK_SET);

    fread(&curInt, sizeof(int), 1, file);
    for (int i = 0; i < elements; i++) {
        fread(&nextInt, sizeof(int), 1, file);
        if (curInt > nextInt) {
            printf("File not sorted, found %d, after %d\n", nextInt, curInt);
            return 1;
        }
    }

    return 0;
}