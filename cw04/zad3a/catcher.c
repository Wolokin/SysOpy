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

void sig_catcher(int signo) { counter++; }

void kill_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    for (int i = 0; i < counter; i++) {
        kill(pid, SIGUSR1);
    }
    kill(pid, SIGUSR2);
    exit(0);
}

void sigqueue_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    union sigval count;
    count.sival_int = 0;
    for (int i = 0; i < counter; i++) {
        count.sival_int++;
        sigqueue(pid, SIGUSR1, count);
    }
    sigqueue(pid, SIGUSR2, count);
    exit(0);
}

void sigrt_responder(int signo, siginfo_t *info, void *context) {
    printf("Catcher received %d signals from sender\n", counter);
    pid_t pid = info->si_pid;
    for (int i = 0; i < counter; i++) {
        kill(pid, SIGRTMIN);
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
        act.sa_sigaction = kill_responder;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR2, &act, NULL);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigdelset(&suspendmask, SIGUSR1);
        sigdelset(&suspendmask, SIGUSR2);
        act.sa_handler = sig_catcher;
        act.sa_flags = 0;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = sigqueue_responder;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGUSR2, &act, NULL);
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigdelset(&suspendmask, SIGRTMIN);
        sigdelset(&suspendmask, SIGRTMIN + 1);
        act.sa_handler = sig_catcher;
        act.sa_flags = 0;
        sigfillset(&act.sa_mask);
        sigaction(SIGRTMIN, &act, NULL);
        act.sa_sigaction = sigrt_responder;
        act.sa_flags = SA_SIGINFO;
        sigfillset(&act.sa_mask);
        sigaction(SIGRTMIN + 1, &act, NULL);
    }
    while (sigsuspend(&suspendmask))
        ;

    return 0;
}
