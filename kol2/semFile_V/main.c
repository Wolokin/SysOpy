#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "."

int main(int argc, char **args) {
    if (argc != 4) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }

    /**************************************************
    Stworz semafor systemu V
    Ustaw jego wartosc na 1
    ***************************************************/
    int semid = semget(ftok(SEM_NAME, 'p'), 1, IPC_CREAT | IPC_EXCL | 0666);
    union semun {
        int val;               /* Value for SETVAL */
        struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
        unsigned short *array; /* Array for GETALL, SETALL */
        struct seminfo *__buf; /* Buffer for IPC_INFO
                                  (Linux-specific) */
    };
    union semun u = {.val = 1};
    semctl(semid, 0, SETVAL, u);
    struct sembuf sbuf = {0, -1, 0};

    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    int parentLoopCounter = atoi(args[1]);
    int childLoopCounter = atoi(args[2]);

    char buf[200];
    pid_t childPid;
    int max_sleep_time = atoi(args[3]);

    if (childPid = fork()) {
        int status = 0;
        srand((unsigned)time(0));

        while (parentLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            /*****************************************
            sekcja krytyczna zabezpiecz dostep semaforem
            **********************************************/
            semop(semid, &sbuf, 1);

            sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n",
                    parentLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            /*********************************
            Koniec sekcji krytycznej
            **********************************/

            sbuf.sem_op = 1;
            semop(semid, &sbuf, 1);
        }
        waitpid(childPid, &status, 0);
    } else {
        srand((unsigned)time(0));
        while (childLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            /*****************************************
            sekcja krytyczna zabezpiecz dostep semaforem
            **********************************************/

            sbuf.sem_op = -1;
            semop(semid, &sbuf, 1);

            sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n",
                    childLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            /*********************************
            Koniec sekcji krytycznej
            **********************************/

            sbuf.sem_op = 1;
            semop(semid, &sbuf, 1);
        }
        _exit(0);
    }

    /*****************************
    posprzataj semafor
    ******************************/
    semctl(semid, -1, IPC_RMID);

    close(fd);
    return 0;
}
