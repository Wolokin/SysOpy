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

void insert_line(int lineno, char* line, FILE* f) {
    --lineno;
    char* reader = NULL;
    size_t size = 0;
    fseek(f, 0, SEEK_SET);
    while (lineno > 0 && getline(&reader, &size, f) > 0) {
        --lineno;
    }
    while (lineno > 0) {
        fprintf(f, "\n");
        --lineno;
    }
    size_t current_pos = ftell(f);
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    if (fsize != current_pos) {
        fseek(f, current_pos, SEEK_SET);
        char* buf = malloc(fsize - current_pos + 1);
        fread(buf, 1, fsize - current_pos, f);
        buf[fsize - current_pos] = '\0';
        fseek(f, current_pos, SEEK_SET);
        fprintf(f, "%s", line);
        fwrite(buf, 1, fsize - current_pos, f);
    } else {
        fprintf(f, "%s\n", line);
    }
    free(reader);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* pipe = fopen(argv[1], "r");
    assert(pipe != NULL);
    int pipe_fd = fileno(pipe);

    FILE* file = fopen(argv[2], "r+");
    assert(file != NULL);
    setvbuf(file, NULL, _IONBF, 0);
    int file_fd = fileno(file);

    unsigned read_size = strtol(argv[3], NULL, 10);
    char* lines[LINES_LIMIT];
    size_t lengths[LINES_LIMIT];
    for (int i = 0; i < LINES_LIMIT; ++i) {
        lines[i] = NULL;
        lengths[i] = 1;  // Space for '\0'
    }

    char* line = alloca(sizeof(char) * read_size + 1);
    unsigned line_number;
    while ("false :)") {
        int result;
        lock_fd(pipe_fd,
                { result = fscanf(pipe, "%d %s", &line_number, line); });
        if (result != 2) break;
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
            lock_fd(file_fd, { insert_line(i, lines[i], file); });
            free(lines[i]);
        }
    }
    fclose(file);
    fclose(pipe);
    return 0;
}
