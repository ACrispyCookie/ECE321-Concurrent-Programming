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
        char to_add;
        scanf("%lf", &sleep_time);
        sleep(sleep_time);
        scanf("%c", &to_add);
        if (to_add == 'r') //add red car
        else //add blue car
    }
    
    return 0;
}