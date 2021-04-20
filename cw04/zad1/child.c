#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (strcmp(argv[1], "ignore") == 0) {
        raise(SIGUSR1);
        printf("Proces po exec zignorowal sygnal\n");
    } else if (strcmp(argv[1], "handler") == 0) {
        raise(SIGUSR1);
    } else if (strcmp(argv[1], "mask") == 0) {
        raise(SIGUSR1);
        printf("Proces po exec zablokowal sygnal\n");
    } else if (strcmp(argv[1], "pending") == 0) {
        sigset_t set;
        sigpending(&set);
        if (sigismember(&set, SIGUSR1)) {
            printf("SIGUSR1 oczekuje w procesie po exec\n");
        } else {
            printf("SIGUSR1 nie oczekuje w procesie po exec\n");
        }
    }
    return 0;
}
