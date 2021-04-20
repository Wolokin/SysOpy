#include <sys/times.h>

#include "lib.h"
#include "stdio.h"
#include "stdlib.h"

#ifdef benchmark
static clock_t real_time1, real_time2;
static struct tms cpu_time1, cpu_time2;
#define bench_time(test_name, code)                                          \
    printf("%s\n", test_name);                                               \
    real_time1 = times(&cpu_time1);                                          \
    code real_time2 = times(&cpu_time2);                                     \
    printf("Real time: %jd\n", real_time2 - real_time1);                     \
    printf("System time: %jd\n", cpu_time2.tms_stime - cpu_time1.tms_stime); \
    printf("User time: %jd\n\n", cpu_time2.tms_utime - cpu_time1.tms_utime);
#else
#define bench_time(test_name, code) code
#endif

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
            bench_time("Merging files bench:", { merge_sequence(pair_list); })
                bench_time("Loading merged files bench:", {
                    read_merged_sequence(t, pair_list);
                }) remove_sequence(&pair_list);
        } else if (strcmp(argv[i], "create_sequence") == 0) {
            ++i;
            pair_list = create_sequence(argc - i, &argv[i]);
        } else if (strcmp(argv[i], "merge_sequence") == 0) {
            bench_time("Merging files bench:", { merge_sequence(pair_list); })
        } else if (strcmp(argv[i], "read_merged_sequence") == 0) {
            bench_time("Loading merged files bench:",
                       { read_merged_sequence(t, pair_list); })
        } else if (strcmp(argv[i], "remove_sequence") == 0) {
            remove_sequence(&pair_list);
        } else if (strcmp(argv[i], "remove_block") == 0) {
            if (++i < argc) {
                bench_time("Removing block bench:",
                           { remove_block(t, strtol(argv[i], NULL, 10)); })
            }
        } else if (strcmp(argv[i], "remove_row") == 0) {
            if (++i + 1 < argc)
                remove_row(t, strtol(argv[i], NULL, 10),
                           strtol(argv[i + 1], NULL, 10));
        } else if (strcmp(argv[i], "print_merged") == 0) {
            print_merged(t);
        } else if (strcmp(argv[i], "get_block_count") == 0) {
            get_block_count(t);
        } else if (strcmp(argv[i], "get_row_count") == 0) {
            if (++i < argc) get_row_count(t, strtol(argv[i], NULL, 10));
        } else if (strcmp(argv[i], "remove_table") == 0) {
            bench_time("Removing blocks bench:", { remove_table(t); }) t =
                create_table(0);
        }
    }
    remove_sequence(&pair_list);
    remove_table(t);
}
