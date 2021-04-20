#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib.h"
#include "stdio.h"
#include "stdlib.h"

#ifdef benchmark
static clock_t real_time1, real_time2;
static struct tms cpu_time1, cpu_time2;
#define bench_time(test_name, code)                               \
    FILE* output = stdout;                                        \
    fprintf(output, "%s\n", test_name);                           \
    fflush(output);                                               \
    real_time1 = times(&cpu_time1);                               \
    code real_time2 = times(&cpu_time2);                          \
    fprintf(output, "Real time: %jd\n", real_time2 - real_time1); \
    fprintf(output, "System time: %jd\n",                         \
            cpu_time2.tms_stime - cpu_time1.tms_stime);           \
    fprintf(output, "User time: %jd\n\n",                         \
            cpu_time2.tms_utime - cpu_time1.tms_utime);
#else
#define bench_time(test_name, code) code
#endif

pid_t merge_sequence_parallel(pair_node* head) {
    pid_t child_pid;
    while (head != NULL) {
        child_pid = fork();
        if (child_pid == 0) {
            merge_pair(head);
            return child_pid;
        }
        head = head->next;
    }
    while (wait(NULL) > 0)
        ;
    return child_pid;
}

int main(int argc, char* argv[]) {
    table* t = NULL;
    if (argc > 1) {
        t = create_table(strtol(argv[1], NULL, 10));
    }
    pair_node* pair_list = NULL;

    for (unsigned i = 1; i < (unsigned)argc; i++) {
        if (strcmp(argv[i], "create_table") == 0) {
            remove_table(t);
            if (++i < argc) t = create_table(strtol(argv[i], NULL, 10));
        } else if (strcmp(argv[i], "merge_files") == 0) {
            ++i;
            pair_list = create_sequence(argc - i, &argv[i]);
            bench_time("Merging files bench:", {
                if (!merge_sequence_parallel(pair_list)) break;
            }) remove_sequence(&pair_list);
        } else if (strcmp(argv[i], "remove_table") == 0) {
            remove_table(t);
            t = create_table(0);
        }
    }
    remove_sequence(&pair_list);
    remove_table(t);
}
