#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ELF_COUNT 10
#define REQ_ELF_COUNT 3
#define REINDEER_COUNT 9
#define REQ_REINDEER_COUNT 9

#define MAX_DELIVERIES 3

#define SEED time(NULL) ^ getpid() ^ pthread_self()

#define critical_section(mutex, code) \
    pthread_mutex_lock(mutex);        \
    code pthread_mutex_unlock(mutex);

#define P(x) printf("%s: %d\n", #x, x);

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wakeup_santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_reindeer_cond = PTHREAD_COND_INITIALIZER;

size_t pending_elves = 0;
pthread_t pending_elves_ids[REQ_ELF_COUNT];
size_t pending_reindeer = 0;

void* santa_thread(void* args) {
    unsigned int rstate = SEED;

    for (int deliveries = 0; deliveries < MAX_DELIVERIES;) {
        printf("Mikołaj: Śpię\n");
        critical_section(&santa_mutex, {
            while (pending_reindeer != REQ_REINDEER_COUNT &&
                   pending_elves != REQ_ELF_COUNT) {
                pthread_cond_wait(&wakeup_santa_cond, &santa_mutex);
            }

            printf("Mikołaj: Budzę się\n");

            if (pending_reindeer == REQ_REINDEER_COUNT) {
                deliveries++;
                printf("Mikołaj: Dostarczam zabawki\n");

                pthread_mutex_unlock(&santa_mutex);
                sleep(2 + (rand_r(&rstate) % 3));
                pthread_mutex_lock(&santa_mutex);

                pending_reindeer = 0;
                pthread_cond_broadcast(&wakeup_reindeer_cond);
            }
            if (pending_elves == REQ_ELF_COUNT) {
                printf("Mikołaj: Rozwiązuję problemy elfów ");
                for (int i = 0; i < REQ_ELF_COUNT; ++i) {
                    printf("%ld, ", pending_elves_ids[i]);
                    pending_elves_ids[i] = pthread_self();
                }
                printf("\n");

                pthread_mutex_unlock(&santa_mutex);
                sleep(1 + (rand_r(&rstate) % 2));
                pthread_mutex_lock(&santa_mutex);

                pending_elves = 0;
                pthread_cond_broadcast(&wakeup_elves_cond);
            }
        });
    }
    return NULL;
}

void* elf_thread(void* args) {
    unsigned int rstate = SEED;
    pthread_t id = pthread_self();

    while (true) {
        sleep(2 + (rand_r(&rstate) % 4));
        critical_section(&santa_mutex, {
            // Waiting for other elves to finish
            if (pending_elves == 3) {
                printf("Elf: Czekam na powrót elfów, %ld\n", id);
            }
            while (pending_elves == 3) {
                pthread_cond_wait(&wakeup_elves_cond, &santa_mutex);
            }
            // Joining the queue
            size_t my_ticket = pending_elves;
            pending_elves_ids[pending_elves++] = id;
            // Checking if there are enough of us to wake up santa
            if (pending_elves == REQ_ELF_COUNT) {
                printf("Elf: Wybudzam mikołaja, %ld\n", id);
                pthread_cond_broadcast(&wakeup_elves_cond);
                pthread_cond_broadcast(&wakeup_santa_cond);
            }
            // Waiting until there is enough of us to wake up santa
            else {
                printf("Elf: Czeka %lu elfów na mikołaja, %ld\n", pending_elves,
                       id);
                while (pending_elves < REQ_ELF_COUNT) {
                    pthread_cond_wait(&wakeup_elves_cond, &santa_mutex);
                }
            }
            // Waiting for santa to solve our problem
            while (pthread_equal(pending_elves_ids[my_ticket], id)) {
                pthread_cond_wait(&wakeup_elves_cond, &santa_mutex);
            }
            printf("Elf: Mikołaj rozwiązał problem, %ld\n", id);
        })
    }
    return NULL;
}

void* reindeer_thread(void* args) {
    unsigned int rstate = SEED;
    pthread_t id = pthread_self();
    while (true) {
        sleep(5 + (rand_r(&rstate) % 6));
        critical_section(&santa_mutex, {
            // Joining the queue
            pending_reindeer++;
            // Checking if I am the last returning reindeer
            if (pending_reindeer == REQ_REINDEER_COUNT) {
                printf("Renifer: Wybudzam mikołaja, %ld\n", id);
                pthread_cond_broadcast(&wakeup_santa_cond);
                pthread_cond_broadcast(&wakeup_reindeer_cond);
            }
            // Waiting for other reindeer
            while (pending_reindeer != REQ_REINDEER_COUNT) {
                printf("Renifer: Czeka %lu reniferów na mikołaja, %ld\n",
                       pending_reindeer, id);
                pthread_cond_wait(&wakeup_reindeer_cond, &santa_mutex);
            }
            // Waiting for santa to finish delivering presents
            while (pending_reindeer > 0) {
                pthread_cond_wait(&wakeup_reindeer_cond, &santa_mutex);
            }
        })
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t elves[ELF_COUNT];
    pthread_t reindeer[REINDEER_COUNT];
    pthread_t santa;

    pthread_create(&santa, NULL, santa_thread, NULL);
    for (int i = 0; i < REINDEER_COUNT; ++i) {
        pthread_create(&reindeer[i], NULL, reindeer_thread, NULL);
    }
    for (int i = 0; i < ELF_COUNT; ++i) {
        pthread_create(&elves[i], NULL, elf_thread, NULL);
    }
    pthread_join(santa, NULL);
    return 0;
}
