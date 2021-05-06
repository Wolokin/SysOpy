#include "common.h"

int find_ready_pizza_index() {
    for (int i = 0; i < TABLE_SZ; ++i) {
        if (pizzeria_ptr->table_space[i] != FREE_SPACE) return i;
    }
    perror(
        "Something went wrong, there should be ready pizzas on the table but "
        "there are none :(\n");
    exit(EXIT_FAILURE);
}

int main() {
    atexit(detach_shared_mem);
    atexit(close_sems);
    open_sems();
    open_shm();
    attach_shared_mem();

    srand(getpid());
    for (int i = 0; i < PIZZA_COUNT; ++i) {
        sem_wait(pizzeria_sem[TABLE_READY_PIZZAS]);
        sem_wait(pizzeria_sem[TABLE]);
        int pizza_index = find_ready_pizza_index();
        int pizza_type = pizzeria_ptr->table_space[pizza_index];
        pizzeria_ptr->table_space[pizza_index] = FREE_SPACE;
        pizzeria_ptr->pizzas_on_table--;
        print_timestamp();
        printf("Pobieram pizze: %d. Liczba pizz na stole: %ld.\n", pizza_type,
               pizzeria_ptr->pizzas_on_table);
        sem_post(pizzeria_sem[TABLE_FREE_SPOTS]);
        sem_post(pizzeria_sem[TABLE]);

        randsleep(4);
        print_timestamp();
        printf("Dostarczam pizze: %d.\n", pizza_type);
        randsleep(4);
    }
}
