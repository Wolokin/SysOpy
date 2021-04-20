#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int sig) {
    printf("Odebralem sygnal %d\tPID: %d\n", sig, getpid());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Nie podano argumentu...\n");
        exit(1);
    }
    struct sigaction act;
    act.sa_handler = SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (strcmp(argv[1], "ignore") == 0) {
        if (fork() == 0) {
            act.sa_handler = SIG_IGN;
            sigaction(SIGUSR1, &act, NULL);
            raise(SIGUSR1);
            printf("Proces przed exec zignorowal sygnal\n");
            execv("child.out", argv);
        }
    } else if (strcmp(argv[1], "handler") == 0) {
        if (fork() == 0) {
            act.sa_handler = handler;
            sigaction(SIGUSR1, &act, NULL);
            raise(SIGUSR1);
            execv("child.out", argv);
        }
    } else if (strcmp(argv[1], "mask") == 0) {
        if (fork() == 0) {
            sigaddset(&act.sa_mask, SIGUSR1);
            sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
            raise(SIGUSR1);
            printf("Proces przed exec zablokowal sygnal\n");
            execv("child.out", argv);
        }
    } else if (strcmp(argv[1], "pending") == 0) {
        if (fork() == 0) {
            sigaddset(&act.sa_mask, SIGUSR1);
            sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
            raise(SIGUSR1);
            sigset_t set;
            sigpending(&set);
            if (sigismember(&set, SIGUSR1)) {
                printf("SIGUSR1 oczekuje w procesie przed exec\n");
            } else {
                printf("SIGUSR1 nie oczekuje w procesie przed exec\n");
            }
            execv("child.out", argv);
        }
    }
    while (wait(NULL) > 0)
        ;
    return 0;
}
