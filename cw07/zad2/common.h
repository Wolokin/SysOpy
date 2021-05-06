#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Semaphore and shared memory specs
typedef enum {
    OVEN,
    OVEN_FREE_SPOTS,
    TABLE,
    TABLE_FREE_SPOTS,
    TABLE_READY_PIZZAS,
    SEM_SIZE
} sem_ids;

#define PROJ_ID 'p'
#define get_key() ftok(getenv("HOME"), PROJ_ID)

int pizzeria_sem;
int pizzeria_shm;

#define OVEN_SZ 5
#define TABLE_SZ 5
#define PIZZA_COUNT 10000
#define FREE_SPACE -1
typedef struct pizzeria {
    int oven_space[OVEN_SZ];
    size_t pizzas_in_oven;
    int table_space[TABLE_SZ];
    size_t pizzas_on_table;
    bool is_active;
} pizzeria;

#define sembuf_struct(sem_no, op) \
    (struct sembuf) { sem_no, op, 0 }
#define sem_operation1(op, sem_no1) \
    semop(pizzeria_sem, (struct sembuf[]){sembuf_struct(sem_no1, op)}, 1)
#define sem_operation2(op, sem_no1, sem_no2)             \
    semop(pizzeria_sem,                                  \
          (struct sembuf[]){sembuf_struct(sem_no1, op),  \
                            sembuf_struct(sem_no2, op)}, \
          2)

#define PERMS 0666
void open_sems(int flag) { pizzeria_sem = semget(get_key(), SEM_SIZE, flag); }

void open_shms(int flag) {
    pizzeria_shm = shmget(get_key(), sizeof(pizzeria), flag);
}

pizzeria* pizzeria_ptr;

void attach_shared_mem() {
    pizzeria_ptr = (pizzeria*)shmat(pizzeria_shm, NULL, 0);
}

void detach_shared_mem() { shmdt(pizzeria_ptr); }

void print_timestamp() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    printf("(PID: %d time: %ld) ", getpid(),
           spec.tv_sec * 1000 + spec.tv_nsec / 1000000);
}

void randsleep(int s) { sleep(s + (rand() % 3)); }

#endif  // COMMON_H
