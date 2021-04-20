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

const int LINELENGTH = 20;

inline int copy_line(FILE* fd, char* buf, char* line, int start, int* size) {
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

inline int string_size(char* buf) {
    int size = 0;
    while (buf[size++] != '\n') continue;
    return size;
}

inline int read_int(char* line) {
    int res = 0, i = 0;
    while (line[i] < '0') i++;
    char tmp = line[i] - '0';
    while (line[i] >= '0') {
        res = res * 10 + tmp;
        tmp = line[++i] - '0';
    }
    return res;
}

inline int is_even(int i) { return !(i % 2); }

inline int is_zero_or_seven(int i) {
    i = (i / 10) % 10;
    return i == 0 || i == 7;
}

inline int is_square(int i) {
    int d = 1, u = i;
    while (d != u) {
        int s = (d + u) / 2;
        if (s * s < i)
            d = s + 1;
        else if (s * s > i)
            u = s;
        else
            return 1;
    }
    return d * d == i;
}

void process_numbers(FILE* fd_source, FILE* fd_dest1, FILE* fd_dest2,
                     FILE* fd_dest3) {
    char buf[BUFSIZ];
    char line[LINELENGTH];
    int p = 0, s = -1;
    while (1) {
        p = (copy_line(fd_source, buf, line, p, &s) + p) % BUFSIZ;
        if (s == 0) break;
        int i = read_int(line);
        int size = string_size(line);
        if (is_even(i))
            if (fwrite(line, sizeof(char), size, fd_dest1) == 0) exit(1);
        if (is_zero_or_seven(i))
            if (fwrite(line, sizeof(char), size, fd_dest2) == 0) exit(1);
        if (is_square(i))
            if (fwrite(line, sizeof(char), size, fd_dest3) == 0) exit(1);
    }
}

int main() {
    const char* source = "dane.txt";
    const char* dest1 = "a.txt";
    const char* dest2 = "b.txt";
    const char* dest3 = "c.txt";
    FILE* fd_source = fopen(source, "r");
    if (fd_source == NULL) {
        if (fwrite("Nie udalo sie wczytac pliku...\n", sizeof(char), 30,
                   stderr) == 0)
            exit(1);
        exit(1);
    }
    FILE* fd_dest1 = fopen(dest1, "w");
    FILE* fd_dest2 = fopen(dest2, "w");
    FILE* fd_dest3 = fopen(dest3, "w");
    if (fd_dest1 == NULL || fd_dest2 == NULL || fd_dest3 == NULL) {
        if (fwrite("Nie udalo sie utworzyc plikow...\n", sizeof(char), 32,
                   stderr) == 0)
            exit(1);
        exit(1);
    }
    bench_time("Numbers processing with lib functions bench: ",
               { process_numbers(fd_source, fd_dest1, fd_dest2, fd_dest3); });
    return 0;
}
