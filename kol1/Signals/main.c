#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sighandler(int signo, siginfo_t* info, void* _ucontext) {
    printf("Received signal %d with value %d\n", signo,
           info->si_value.sival_int);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;

    //..........

    // zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1 i SIGUSR2
    // zdefiniuj obsluge SIGUSR1 i SIGUSR2 w taki sposob zeby proces potomny
    // wydrukowal na konsole przekazana przez rodzica wraz z sygnalami SIGUSR1 i
    // SIGUSR2 wartosci
    action.sa_flags = SA_SIGINFO;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &action.sa_mask, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    int child = fork();
    if (child == 0) {
        sleep(1);
    } else {
        // wyslij do procesu potomnego sygnal przekazany jako argv[2]
        // wraz z wartoscia przekazana jako argv[1]
        int signo = strtol(argv[2], NULL, 10);
        int val = strtol(argv[1], NULL, 10);
        sigqueue(child, signo, (union sigval){.sival_int = val});
    }

    return 0;
}
