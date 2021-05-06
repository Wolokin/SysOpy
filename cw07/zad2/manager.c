#include "common.h"

int *child_pids;

// atexit
void close_sem() { semctl(pizzeria_sem, 0, IPC_RMID); }

void close_shm() { shmctl(pizzeria_shm, IPC_RMID, NULL); }

void kill_children(int _signo) {
    int i = 0;
    while (child_pids[i]) {
        kill(child_pids[i], SIGTERM);
        i++;
    }
}

union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

void init_sems() {
    open_sems(IPC_CREAT | IPC_EXCL | PERMS);
    semctl(pizzeria_sem, OVEN, SETVAL, (union semun){.val = 1});
    semctl(pizzeria_sem, OVEN_FREE_SPOTS, SETVAL,
           (union semun){.val = OVEN_SZ});
    semctl(pizzeria_sem, TABLE, SETVAL, (union semun){.val = 1});
    semctl(pizzeria_sem, TABLE_FREE_SPOTS, SETVAL,
           (union semun){.val = TABLE_SZ});
    semctl(pizzeria_sem, TABLE_READY_PIZZAS, SETVAL, (union semun){.val = 0});
}

void init_shms() {
    open_shms(IPC_CREAT | IPC_EXCL | PERMS);
    attach_shared_mem();
    pizzeria_ptr->pizzas_in_oven = 0;
    pizzeria_ptr->pizzas_on_table = 0;
    pizzeria_ptr->is_active = true;
    for (int i = 0; i < OVEN_SZ; ++i) {
        pizzeria_ptr->oven_space[i] = FREE_SPACE;
    }
    for (int i = 0; i < TABLE_SZ; ++i) {
        pizzeria_ptr->table_space[i] = FREE_SPACE;
    }
    detach_shared_mem();
}

int main(int argc, char *argv[]) {
    if (argc < 2 + 1) {
        perror("Za malo argumentow!\n");
        exit(EXIT_FAILURE);
    }
    atexit(close_sem);
    atexit(close_shm);

    init_sems();
    init_shms();

    int cook_count = strtol(argv[1], NULL, 10);
    int uber_count = strtol(argv[2], NULL, 10);
    child_pids = malloc(sizeof(int) * (cook_count + uber_count + 1));
    for (int i = 0; i < cook_count; ++i) {
        child_pids[i] = fork();
        if (child_pids[i] == 0) {
            execv("./cook.out", (char *[]){"./cook.out", NULL});
        }
    }
    for (int i = 0; i < uber_count; ++i) {
        child_pids[i + cook_count] = fork();
        if (child_pids[i + cook_count] == 0) {
            execv("./uber.out", (char *[]){"./uber.out", NULL});
        }
    }
    child_pids[cook_count + uber_count] = 0;
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = kill_children;
    sigaction(SIGINT, &act, NULL);
    pause();
    while (wait(NULL) != -1)
        ;

    return 0;
}
