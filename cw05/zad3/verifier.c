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

int main(int argc, char* argv[]) {
    assert(argc == 3);
    FILE* in = fopen(argv[1], "r");
    FILE* out = fopen(argv[2], "r");
    char* buf_in;
    char* buf_out;
    char* name = NULL;
    size_t name_size = 0;
    size_t in_size = 0;
    size_t out_size = 0;
    getline(&name, &name_size, out);
    name[strcspn(name, "\n")] = '\0';
    FILE* output = fopen(name, "r");
    while (getline(&name, &name_size, in) > 0) {
        name[strcspn(name, "\n")] = '\0';
        FILE* input = fopen(name, "r");
        getline(&buf_in, &in_size, input);
        getline(&buf_out, &out_size, output);
        char* tmp1 = buf_in;
        char* tmp2 = buf_out;
        if (strcmp(strsep(&tmp1, "\n"), strsep(&tmp2, "\n")) != 0) {
            printf("Difference detected while checking file %s\n", name);
            printf("Expected:\t%s\nFound:\t\t%s\n", buf_in, buf_out);
            exit(0);
        }
        fclose(input);
    }
    printf("Files verified, 0 differences detected\n");
    return 0;
}
