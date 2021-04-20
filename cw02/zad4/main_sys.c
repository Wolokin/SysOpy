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

int matching(char* buf, char* pattern, int s1, int s2) {
    int i = 0;
    while (i < s1 && i < s2) {
        if (buf[i] != pattern[i]) return 0;
        i++;
    }
    return i;
}

int string_size(char* buf) {
    int size = 0;
    while (buf[size] != '\n' && buf[size] != '\0') ++size;
    return size;
}

void string_copy(char* in, char* out, int size) {
    for (int i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}

void save_swapped(int fd_in, int fd_out, char* pattern, char* replacement) {
    char buf[BUFSIZ];
    int pm = 0;
    int pstart = 0;
    int pattern_size = string_size(pattern);
    int replacement_size = string_size(replacement);
    int p = 0, s = -1;
    while (s != 0) {
        if (p >= s) {
            if (write(fd_out, &buf[pstart], p - pstart - pm) == -1) exit(1);
            s = read(fd_in, buf, BUFSIZ);
            if (s == 0) break;
            p = 0;
            pstart = 0;
        }
        int match = matching(&buf[p], &pattern[pm], s - p, pattern_size);
        if (match + pm == pattern_size) {
            if (write(fd_out, &buf[pstart], p - pstart) == -1) exit(1);
            if (write(fd_out, replacement, replacement_size) == -1) exit(1);
            p += pattern_size;
            pstart = p;
            pm = 0;
        } else if (match) {
            pm = match;
        } else {
            if (pm) {
                if (write(fd_out, pattern, pm) == -1) exit(1);
                pm = 0;
            }
            ++p;
        }
    }
}

int main(int argc, char* argv[]) {
    char *f1, *f2, *s1, *s2;
    if (argc >= 5) {
        f1 = argv[1];
        f2 = argv[2];
        s1 = argv[3];
        s2 = argv[4];
    } else {
        if (write(2, "Nie podano argumentow...\n", 25) == -1) exit(1);
        exit(1);
    }
    int fd_in = open(f1, O_RDONLY);
    int fd_out = open(f2, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd_in == -1 || fd_out == -1) {
        if (write(2, "Blad otwierania pliku...", 24) == -1) exit(1);
        exit(1);
    }
    bench_time("Swapping with sys functions bench: ",
               { save_swapped(fd_in, fd_out, s1, s2); });
    return 0;
}
