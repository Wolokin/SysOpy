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
        code;               \
        flock(fd, LOCK_UN); \
    }

int main(int argc, char* argv[]) {
    if (argc != 4 + 1) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* pipe = fopen(argv[1], "w");
    assert(pipe != NULL);
    setvbuf(pipe, NULL, _IONBF, 0);

    unsigned line_number = strtol(argv[2], NULL, 10);
    FILE* file = fopen(argv[3], "r");
    setvbuf(file, NULL, _IONBF, 0);
    assert(file != NULL);

    unsigned read_size = strtol(argv[4], NULL, 10);
    char* str = alloca(read_size + 1);
    char* buf = alloca(read_size + 10 + 1);
    while ("false :)") {
        char* token = fgets(str, read_size + 1, file);
        if (token == NULL) break;
        token[strcspn(token, "\n")] = '\0';
        if (*token == '\0') break;
        sprintf(buf, "%d %s\n", line_number, str);
        sleep(1 + (rand() % 2));
        fprintf(pipe, "%s", buf);
    }
    fclose(pipe);
    fclose(file);
    return 0;
}
