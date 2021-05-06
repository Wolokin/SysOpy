#include "common.h"

int *child_pids;

// atexit
void delete_sems() {
    sem_unlink(SEM_NAME "_oven");
    sem_unlink(SEM_NAME "_oven_free_spots");
    sem_unlink(SEM_NAME "_table");
    sem_unlink(SEM_NAME "_table_free_spots");
    sem_unlink(SEM_NAME "_table_ready_pizzas");
}

void delete_shm() { shm_unlink(SHM_NAME); }

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
    pizzeria_sem[OVEN] = sem_open(SEM_NAME "_oven", O_CREAT | O_EXCL, PERMS, 1);
    pizzeria_sem[OVEN_FREE_SPOTS] =
        sem_open(SEM_NAME "_oven_free_spots", O_CREAT | O_EXCL, PERMS, OVEN_SZ);
    pizzeria_sem[TABLE] =
        sem_open(SEM_NAME "_table", O_CREAT | O_EXCL, PERMS, 1);
    pizzeria_sem[TABLE_FREE_SPOTS] = sem_open(
        SEM_NAME "_table_free_spots", O_CREAT | O_EXCL, PERMS, TABLE_SZ);
    pizzeria_sem[TABLE_READY_PIZZAS] =
        sem_open(SEM_NAME "_table_ready_pizzas", O_CREAT | O_EXCL, PERMS, 0);
    close_sems();
}

void init_shm() {
    pizzeria_shm = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, PERMS);
    ftruncate(pizzeria_shm, sizeof(pizzeria));
    attach_shared_mem();
    pizzeria_ptr->pizzas_in_oven = 0;
    pizzeria_ptr->pizzas_on_table = 0;
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
    atexit(delete_sems);
    atexit(delete_shm);

    init_sems();
    init_shm();

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
