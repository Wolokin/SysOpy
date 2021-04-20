#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEP printf("==================================\n");

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t to_send;

void sig_catcher(int signo) { counter++; }

void sig_analyzer(int signo, siginfo_t *info, void *context) {
    printf("Sender sent %d signals to catcher\n", to_send);
    if (info->si_value.sival_int) {
        printf("Catcher has told sender that he received %d signals\n",
               info->si_value.sival_int);
    }
    printf("Sender received %d signals from catcher\n", counter);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2 + 1) {
        fprintf(stderr, "Nie podano trybu i ilosci sygnalow...\n");
        exit(1);
    }
    char *mode = argv[1];
    to_send = strtol(argv[2], NULL, 10);
    pid_t catcher_pid;
    // For convenience
    if (argc == 4) {
        catcher_pid = strtol(argv[3], NULL, 10);
    } else {
        catcher_pid = fork();
        if (catcher_pid == 0) {
            execl("./catcher.out", "catcher", mode, NULL);
        }
        // Without sleep, sender might kill catcher before he could register his
        // SIGUSR1 handles. Not pretty but suffices xd
        sleep(1);
    }

    sigset_t block_all;
    sigfillset(&block_all);
    sigprocmask(SIG_SETMASK, &block_all, NULL);
    sigset_t suspendmask;
    sigfillset(&suspendmask);
    struct sigaction act;
    if (strcmp(mode, "KILL") == 0) {
        sigdelset(&suspendmask, SIGUSR1);
        sigdelset(&suspendmask, SIGUSR2);
        act.sa_handler = sig_catcher;
        act.sa_flags = 0;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = sig_analyzer;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR2, &act, NULL);
        for (int i = 0; i < to_send; ++i) {
            kill(catcher_pid, SIGUSR1);
        }
        kill(catcher_pid, SIGUSR2);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigdelset(&suspendmask, SIGUSR1);
        sigdelset(&suspendmask, SIGUSR2);
        act.sa_handler = sig_catcher;
        act.sa_flags = 0;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = sig_analyzer;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR2, &act, NULL);
        union sigval nothing;
        nothing.sival_int = 0;
        for (int i = 0; i < to_send; ++i) {
            sigqueue(catcher_pid, SIGUSR1, nothing);
        }
        sigqueue(catcher_pid, SIGUSR2, nothing);
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigdelset(&suspendmask, SIGRTMIN);
        sigdelset(&suspendmask, SIGRTMIN + 1);
        act.sa_handler = sig_catcher;
        act.sa_flags = 0;
        sigfillset(&act.sa_mask);
        sigaction(SIGRTMIN, &act, NULL);
        act.sa_sigaction = sig_analyzer;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGRTMIN + 1, &act, NULL);
        for (int i = 0; i < to_send; ++i) {
            kill(catcher_pid, SIGRTMIN);
        }
        kill(catcher_pid, SIGRTMIN + 1);
    } else {
        return 0;
    }
    while (sigsuspend(&suspendmask))
        ;

    return 0;
}
