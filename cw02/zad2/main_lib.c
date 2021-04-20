#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "sys/times.h"
#include "sys/types.h"
#include "unistd.h"

#define benchmark
#ifdef benchmark
static clock_t real_time1, real_time2;
static struct tms cpu_time1, cpu_time2;
#define bench_time(test_name, code)                               \
    real_time1 = times(&cpu_time1);                               \
    code real_time2 = times(&cpu_time2);                          \
    fprintf(stderr, "%s\n", test_name);                           \
    fprintf(stderr, "Real time: %jd\n", real_time2 - real_time1); \
    fprintf(stderr, "System time: %jd\n",                         \
            cpu_time2.tms_stime - cpu_time1.tms_stime);           \
    fprintf(stderr, "User time: %jd\n\n",                         \
            cpu_time2.tms_utime - cpu_time1.tms_utime);
#else
#define bench_time(test_name, code) code
#endif

const int LINELENGTH = 256;

int copy_line(FILE* fd, char* buf, char* line, int start, int* size) {
    if (start >= *size || start == 0) {
        *size = fread(buf, sizeof(char), BUFSIZ, fd);
        if (*size == 0) return 0;
        start = 0;
    }
    int end = start;
    while (end < *size) {
        if (buf[end++] == '\n') {
            break;
        }
    }
    for (int i = 0; i < end - start; ++i) {
        line[i] = buf[start + i];
    }
    if (buf[end - 1] != '\n') {
        return end - start + copy_line(fd, buf, &line[end - start], end, size);
    }
    return end - start;
}

int has_match(char c, char* line, int size) {
    for (int i = 0; i < size; ++i) {
        if (line[i] == c) return 1;
    }
    return 0;
}

int line_size(char* buf) {
    int size = 0;
    while (buf[size++] != '\n') continue;
    return size;
}

void print_matching_lines(char c, FILE* fd) {
    char buf[BUFSIZ];
    char line[LINELENGTH];
    int p = 0, s = -1;
    while (s != 0) {
        p = (copy_line(fd, buf, line, p, &s) + p) % BUFSIZ;
        int size = line_size(line);
        if (has_match(c, line, size)) {
            if (fwrite(line, sizeof(char), size, stdout) == 0) exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    char c;
    char* f;
    if (argc >= 3) {
        c = argv[1][0];
        f = argv[2];
    } else {
        if (fwrite("Nie podano argumentow...\n", sizeof(char), 25, stderr) == 0)
            exit(1);
        exit(1);
    }
    FILE* fd = fopen(f, "r");
    if (fd == NULL) {
        if (fwrite("Blad otwierania pliku: ", sizeof(char), 23, stderr) == 0)
            exit(1);
        if (fwrite(f, sizeof(char), 50, stderr) == 0) exit(1);
        exit(1);
    }
    bench_time("Find char with lib functions bench: ",
               { print_matching_lines(c, fd); });
    return 0;
}
