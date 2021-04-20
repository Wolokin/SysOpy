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

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "Bad argument count (%d)\n", argc - 1);
        exit(1);
    }
    FILE* program;
    char buf[BUFSIZ];
    if (argc == 2) {
        sprintf(buf, "ping -c 10 %s" ,argv[1]);
    }
    if (argc == 4) {
        sprintf(buf, "wget "
                     "--recursive "
                     "--level=%ld "
                     "--page-requisites "
                     "--adjust-extension "
                     "--span-hosts "
                     "--convert-links "
                     "--restrict-file-names=windows "
                     "--domains %s "
                     "--no-parent %s", strtol(argv[3], NULL, 10), argv[2], argv[1]);
    }
    program = popen(buf, "r");
    assert(program != NULL);
    while (fread(buf, sizeof(char), BUFSIZ, program)) {
        printf("%s", buf);
    }
    return 0;
}
