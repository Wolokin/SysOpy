#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/mman.h>
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

#define SEM_NAME "/pizzeria_sem"
#define SHM_NAME "/pizzeria_shm"

sem_t* pizzeria_sem[SEM_SIZE];
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
} pizzeria;

#define PERMS 0666
void open_sems() {
    pizzeria_sem[OVEN] = sem_open(SEM_NAME "_oven", 0);
    pizzeria_sem[OVEN_FREE_SPOTS] = sem_open(SEM_NAME "_oven_free_spots", 0);
    pizzeria_sem[TABLE] = sem_open(SEM_NAME "_table", 0);
    pizzeria_sem[TABLE_FREE_SPOTS] = sem_open(SEM_NAME "_table_free_spots", 0);
    pizzeria_sem[TABLE_READY_PIZZAS] =
        sem_open(SEM_NAME "_table_ready_pizzas", 0);
}
void close_sems() {
    for (int i = 0; i < SEM_SIZE; ++i) {
        sem_close(pizzeria_sem[i]);
    }
}
void open_shm() { pizzeria_shm = shm_open(SHM_NAME, O_RDWR, PERMS); }

pizzeria* pizzeria_ptr;

void attach_shared_mem() {
    pizzeria_ptr =
        (pizzeria*)mmap(NULL, sizeof(pizzeria), PROT_READ | PROT_WRITE,
                        MAP_SHARED, pizzeria_shm, 0);
}

void detach_shared_mem() { munmap(pizzeria_ptr, sizeof(pizzeria)); }

void print_timestamp() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    printf("(PID: %d time: %ld) ", getpid(),
           spec.tv_sec * 1000 + spec.tv_nsec / 1000000);
}

void randsleep(int s) { sleep(s + (rand() % 3)); }

#endif  // COMMON_H
