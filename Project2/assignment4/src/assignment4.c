#include "../../assingment1/src/mysem.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of >", argv[0]);
        return -1;
    }
    const int N = atoi(argv[1]);

    //initialize other needed fields

    while(1) {
        double sleep_time;
        scanf("%lf", &sleep_time);
        sleep(sleep_time);
        //add passenger
    }
    
    return 0;
}