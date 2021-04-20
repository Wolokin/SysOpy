#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEP printf("==================================\n");

volatile sig_atomic_t recursive_flag = 1;

void siginfo_handler(int signo, siginfo_t *info, void *context) {
    printf("SA_SIGINFO handler\n");
    printf("Received signal %d from PID %d\n", signo, info->si_pid);
    printf("Sending process ID %d\n", info->si_pid);
    printf("Real user ID of sending process %d\n", info->si_uid);
    printf("Exit value or signal %d\n", info->si_status);
    printf("User time consumed %ld\n", info->si_utime);
    printf("System time consumed %ld\n", info->si_stime);
    printf("End of handler\n");
}

void nodefer_handler(int signo) {
    printf("SA_NODEFER handler\n");
    if (recursive_flag) {
        recursive_flag = 0;
        raise(SIGUSR1);
    }
    sleep(1);
    printf("End of handler\n");
}

void resethand_handler(int signo) {
    printf("SA_RESETHAND handler\n");
    printf("End of handler\n");
}

int main(void) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    SEP;

    act.sa_sigaction = siginfo_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);
    sleep(1);

    SEP;

    recursive_flag = 1;
    act.sa_handler = nodefer_handler;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
    printf("\nWithout SA_NODEFER:\n\n");
    raise(SIGUSR1);
    sleep(1);

    recursive_flag = 1;
    act.sa_flags = SA_NODEFER;
    sigaction(SIGUSR1, &act, NULL);
    printf("\nWith SA_NODEFER:\n\n");
    raise(SIGUSR1);
    sleep(1);

    SEP;

    act.sa_handler = resethand_handler;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
    printf("\nWithout SA_RESETHAND:\n\n");
    raise(SIGUSR1);
    sleep(1);
    raise(SIGUSR1);
    sleep(1);

    act.sa_flags = SA_RESETHAND;
    sigaction(SIGUSR1, &act, NULL);
    printf("\nWith SA_RESETHAND:\n\n");
    raise(SIGUSR1);
    sleep(1);
    fflush(stdout);
    raise(SIGUSR1);
    sleep(1);

    return 0;
}
