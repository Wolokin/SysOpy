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
volatile sig_atomic_t do_count = 1;
pid_t sender_pid;
sigset_t suspendmask;

void sig_catcher(int signo, siginfo_t *info, void *context) {
    sender_pid = info->si_pid;
    if (do_count) counter++;
}

void kill_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    do_count = 0;
    for (int i = 0; i < counter; i++) {
        kill(pid, SIGUSR1);
        sigsuspend(&suspendmask);
    }
    kill(pid, SIGUSR2);
    exit(0);
}

void sigqueue_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    union sigval counted;
    counted.sival_int = 0;
    do_count = 0;
    for (int i = 0; i < counter; i++) {
        counted.sival_int++;
        sigqueue(pid, SIGUSR1, counted);
        sigsuspend(&suspendmask);
    }
    sigqueue(pid, SIGUSR2, counted);
    exit(0);
}

void sigrt_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    do_count = 0;
    for (int i = 0; i < counter; i++) {
        kill(pid, SIGRTMIN);
        sigsuspend(&suspendmask);
    }
    kill(pid, SIGRTMIN + 1);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 1 + 1) {
        fprintf(stderr, "Nie podano trybu...\n");
        exit(1);
    }
    char *mode = argv[1];
    printf("%d\n", getpid());

    sigset_t block_all;
    sigfillset(&block_all);
    sigprocmask(SIG_SETMASK, &block_all, NULL);

    sigfillset(&suspendmask);

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    sigfillset(&act.sa_mask);
    act.sa_sigaction = sig_catcher;

    if (strcmp(mode, "KILL") == 0) {
        sigdelset(&suspendmask, SIGUSR1);
        sigdelset(&suspendmask, SIGUSR2);
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = kill_responder;
        sigaction(SIGUSR2, &act, NULL);
        while (sigsuspend(&suspendmask)) {
            kill(sender_pid, SIGUSR1);
        }
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigdelset(&suspendmask, SIGUSR1);
        sigdelset(&suspendmask, SIGUSR2);
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = sigqueue_responder;
        sigaction(SIGUSR2, &act, NULL);
        while (sigsuspend(&suspendmask)) {
            sigqueue(sender_pid, SIGUSR1, (union sigval){0});
        }
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigdelset(&suspendmask, SIGRTMIN);
        sigdelset(&suspendmask, SIGRTMIN + 1);
        sigaction(SIGRTMIN, &act, NULL);
        act.sa_sigaction = sigrt_responder;
        sigaction(SIGRTMIN + 1, &act, NULL);
        while (sigsuspend(&suspendmask)) {
            kill(sender_pid, SIGRTMIN);
        }
    }

    return 0;
}
