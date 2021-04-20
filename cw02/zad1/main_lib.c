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

int print_line(FILE* fd, char* buf, int start, int* size) {
    if (start == 0 || start >= *size) {
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
    if (fwrite(&buf[start], sizeof(char), end - start, stdout) == 0) exit(1);
    if (buf[end - 1] != '\n') {
        return end - start + print_line(fd, buf, end, size);
    }
    return end - start;
}

void print_merged_files(FILE* fd1, FILE* fd2) {
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
    FILE* fd1 = fopen(f1, "r");
    if (fd1 == NULL) {
        if (fwrite("Blad otwierania pliku: ", sizeof(char), 23, stderr) == 0)
            exit(1);
        if (fwrite(f1, sizeof(char), 50, stderr) == 0) exit(1);
        exit(1);
    }
    FILE* fd2 = fopen(f2, "r");
    if (fd2 == NULL) {
        if (fwrite("Blad otwierania pliku: ", sizeof(char), 23, stderr) == 0)
            exit(1);
        if (fwrite(f2, sizeof(char), 50, stderr) == 0) exit(1);
        exit(1);
    }
    bench_time("Merging with lib functions bench: ",
               { print_merged_files(fd1, fd2); });
    return 0;
}
