#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "../assignment2/src/mythreads.h"

char *stdin_buf = NULL;
unsigned int stdin_size;

void thr(void *arg) {
    printf("thread\n");
}

void enableNonBlockingInput() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); // Set stdin to non-blocking
}

void fill_buffer() {
    int bytes_read;
    char buf; 
    while(1) {
        bytes_read = read(STDIN_FILENO, &buf, 1);
        if (bytes_read <= 0)
            break;
        stdin_size++;
        stdin_buf = realloc(stdin_buf, stdin_size);
        stdin_buf[stdin_size - 1] = buf;
    }
}

int scanf_w(const char *format, ...) {
    fill_buffer();
    if (stdin_size == 0)
        return 0;
    
    va_list args;
    va_start(args, format);
    int ret = vsscanf(stdin_buf, format, args);
    va_end(args);
    if (ret != 0)
        free(stdin_buf);

    return ret;
}

// int main() {
//     enableNonBlockingInput();
//     char c;
//     while (1) {
//         fill_buffer();
//         if (read(STDIN_FILENO, &c, 1) > 0) {
//             printf("You typed!\n");
//         } else {
//             printf("No input yet...\n");
//         }
//         usleep(500000); // Sleep for 500ms
//     }
//     return 0;
// }

int main(int argc, char *argv[]) {

    int timeout = atoi(argv[1]);
    mythreads_init(timeout);
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    char buf[32];
    mythr_t thread;
    // mythreads_create(&thread, thr, NULL);
    // sleep(5);
    printf("reading...\n");
    int ret = scanf_w("%31s", buf);
    printf("read: %d %s\n", ret, buf);
    // printf("%c %d %d", buf, ret, EOF);
    // mythreads_join(&thread);
    // mythreads_destroy(&thread);

    mythreads_exit();

    return 0;
}