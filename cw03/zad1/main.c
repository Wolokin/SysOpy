#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("nie podano liczby procesow...");
        exit(1);
    }
    int count = strtol(argv[1], NULL, 0);
    pid_t child_pid;
    printf("PID glownego programu: %d\n", (int)getpid());
    for (int i = 0; i < count; ++i) {
        child_pid = fork();
        if (child_pid == 0) {
            printf("Proces dziecka -> ppid:%d pid:%d\n", (int)getppid(),
                   (int)getpid());
            return 0;
        }
    }
    while (wait(NULL) > 0)
        ;
    return 0;
}
