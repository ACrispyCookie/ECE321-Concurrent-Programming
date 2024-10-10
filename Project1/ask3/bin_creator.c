#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./bin_creator <filename> <number of integers>\n");
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "w");
    const int N = atoi(argv[2]);
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        int r = rand();
        fwrite(&r, sizeof(int), 1, file);
    }
    fclose(file);
}
