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

// Max. no. of lines
#define LINES_LIMIT 100 + 1  // Lines start from 1

#define lock_fd(fd, code)   \
    {                       \
        flock(fd, LOCK_EX); \
        code;               \
        flock(fd, LOCK_UN); \
    }

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* pipe = fopen(argv[1], "r");
    int pipe_fd = fileno(pipe);
    assert(pipe != NULL);

    FILE* file = fopen(argv[2], "w");
    int file_fd = fileno(file);
    assert(file != NULL);

    // unsigned read_size = strtol(argv[3], NULL, 10);  // Not needed
    char* lines[LINES_LIMIT];
    size_t lengths[LINES_LIMIT];
    for (int i = 0; i < LINES_LIMIT; ++i) {
        lines[i] = NULL;
        lengths[i] = 1;  // Space for '\0'
    }

    char line[BUFSIZ];
    char num[BUFSIZ];
    unsigned line_number;
    while ("true") {
        //line_number = strtol(num, NULL, 10);
        printf("reading\n");
        flock(pipe_fd, LOCK_EX);
        /* printf("in\n"); */
        /* fflush(stdout); */
        if (fscanf(pipe, "%d %s\n", &line_number, line) != 2) break;
        /* printf("out\n"); */
        /* fflush(stdout); */
        flock(pipe_fd, LOCK_UN);
        size_t line_length = strlen(line);
        lines[line_number] =
            realloc(lines[line_number],
                    sizeof(char) * (lengths[line_number] + line_length));
        assert(lines[line_number] != NULL);
        strcpy(lines[line_number] + lengths[line_number] - 1, line);
        lengths[line_number] += line_length;
    }
    for (size_t i = 1; i < LINES_LIMIT; ++i) {
        if (lines[i] != NULL) {
            lock_fd(file_fd, { fprintf(file, "%s", lines[i]); });
            free(lines[i]);
        }
        lock_fd(file_fd, { fprintf(file, "\n"); });
    }
    fclose(file);
    fclose(pipe);
    return 0;
}
