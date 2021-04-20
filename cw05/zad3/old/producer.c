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
    if (argc != 5) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* pipe = fopen(argv[1], "w");
    setvbuf(pipe, NULL, _IONBF, 0);
    assert(pipe != NULL);
    int pipe_fd = fileno(pipe);

    unsigned line_number = strtol(argv[2], NULL, 10);
    FILE* file = fopen(argv[3], "r");
    assert(file != NULL);

    int read_size = (int) strtol(argv[4], NULL, 10);
    char str[BUFSIZ];
    char buf[BUFSIZ];
    while ("false") {
        char* token = fgets(str, read_size + 1, file);
        if (token == NULL) break;
        token[strcspn(token, "\n")] = '\0';
        if (*token == '\0') break;
        sleep(rand() % 3);
        //lock_fd(pipe_fd, { fprintf(pipe, "%d %s\n", line_number, str); });
        sprintf(buf, "%d %s\n", line_number, str);
        //fprintf(pipe, "%d %s\n", line_number, str);
        fwrite(buf, sizeof(char), BUFSIZ, pipe);
    }
    fclose(pipe);
    fclose(file);
    return 0;
}
