#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//#define NDEBUG

#define lock_fd(fd, code)   \
    {                       \
        flock(fd, LOCK_EX); \
        sleep(1);           \
        code;               \
        flock(fd, LOCK_UN); \
    }

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* pipe = fopen(argv[1], "w");
    assert(pipe != NULL);
    int pipe_fd = fileno(pipe);

    unsigned line_number = strtol(argv[2], NULL, 10);
    FILE* file = fopen(argv[3], "r");
    assert(file != NULL);

    unsigned read_size = strtol(argv[4], NULL, 10);
    char str[BUFSIZ];
    fgets(str, 10000, file);
    fseek(file, 0, SEEK_SET);
    printf("%s", str);
    fflush(stdout);
    while (fgets(str, read_size + 1, file)) {
        sleep(rand() % 3);
        lock_fd(1, { printf("^|%d %s|^\n", line_number, str); });
        fflush(stdout);
        lock_fd(pipe_fd, { fprintf(pipe, "%d %s\n", line_number, str); });
    }
    // fclose(pipe);
    // fclose(file);
    return 0;
}
