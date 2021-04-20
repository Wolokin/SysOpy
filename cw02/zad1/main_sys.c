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

const int NAMESIZE = 255;

int print_line(int fd, char* buf, int start, int* size) {
    if (start == 0 || start >= *size) {
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
    if (write(1, &buf[start], end - start) == -1) exit(1);
    if (buf[end - 1] != '\n') {
        return end - start + print_line(fd, buf, end, size);
    }
    return end - start;
}

void print_merged_files(int fd1, int fd2) {
    char buf1[BUFSIZ];
    char buf2[BUFSIZ];
    int p1 = 0, s1 = -1;
    int p2 = 0, s2 = -1;
    while (s1 != 0 && s2 != 0) {
        p1 = (print_line(fd1, buf1, p1, &s1) + p1) % BUFSIZ;
        p2 = (print_line(fd2, buf2, p2, &s2) + p2) % BUFSIZ;
    }
    while (s1 != 0) {
        p1 = (print_line(fd1, buf1, p1, &s1) + p1) % BUFSIZ;
    }
    while (s2 != 0) {
        p2 = (print_line(fd2, buf2, p2, &s2) + p2) % BUFSIZ;
    }
}

int main(int argc, char* argv[]) {
    char *f1, *f2;
    f1 = f2 = NULL;
    if (argc >= 3) {
        f1 = argv[1];
        f2 = argv[2];
    } else {
        if (argc == 1) {
            printf("Prosze podac nazwe pierwszego pliku: ");
            f1 = calloc(NAMESIZE, sizeof(char));
            if (scanf("%s", f1) == EOF) exit(1);
        }
        printf("Prosze podac nazwe drugiego pliku: ");
        f2 = calloc(NAMESIZE, sizeof(char));
        if (scanf("%s", f2) == EOF) exit(1);
    }
    int fd1 = open(f1, O_RDONLY);
    if (fd1 == -1) {
        if (write(2, "Blad otwierania pliku: ", 23) == -1) exit(1);
        if (write(2, f1, 50) == -1) exit(1);
        exit(1);
    }
    int fd2 = open(f2, O_RDONLY);
    if (fd2 == -1) {
        if (write(2, "Blad otwierania pliku: ", 23) == -1) exit(1);
        if (write(2, f2, 50) == -1) exit(1);
        exit(1);
    }
    bench_time("Merging with sys functions bench: ",
               { print_merged_files(fd1, fd2); });
    return 0;
}
