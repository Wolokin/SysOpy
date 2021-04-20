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

inline int copy_line(int fd, char* buf, char* line, int start, int* size) {
    if (start >= *size || start == 0) {
        *size = read(fd, buf, BUFSIZ);
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

void process_numbers(int fd_source, int fd_dest1, int fd_dest2, int fd_dest3) {
    char buf[BUFSIZ];
    char line[LINELENGTH];
    int p = 0, s = -1;
    while (1) {
        p = (copy_line(fd_source, buf, line, p, &s) + p) % BUFSIZ;
        if (s == 0) break;
        int i = read_int(line);
        int size = string_size(line);
        if (is_even(i))
            if (write(fd_dest1, line, size) == -1) exit(1);
        if (is_zero_or_seven(i))
            if (write(fd_dest2, line, size) == -1) exit(1);
        if (is_square(i))
            if (write(fd_dest3, line, size) == -1) exit(1);
    }
}

int main() {
    const char* source = "dane.txt";
    const char* dest1 = "a.txt";
    const char* dest2 = "b.txt";
    const char* dest3 = "c.txt";
    int fd_source = open(source, O_RDONLY);
    if (fd_source == -1) {
        if (write(2, "Nie udalo sie wczytac pliku...\n", 30) == -1) exit(1);
        exit(1);
    }
    int fd_dest1 = open(dest1, O_WRONLY | O_CREAT, S_IRWXU);
    int fd_dest2 = open(dest2, O_WRONLY | O_CREAT, S_IRWXU);
    int fd_dest3 = open(dest3, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd_dest1 == -1 || fd_dest2 == -1 || fd_dest3 == -1) {
        if (write(2, "Nie udalo sie utworzyc plikow...\n", 32) == -1) exit(1);
        exit(1);
    }
    bench_time("Numbers processing with sys functions bench: ",
               { process_numbers(fd_source, fd_dest1, fd_dest2, fd_dest3); });
    return 0;
}
