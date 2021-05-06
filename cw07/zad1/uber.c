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
    open_sems(PERMS);
    open_shms(PERMS);
    attach_shared_mem();
    atexit(detach_shared_mem);

    srand(getpid());
    for (int i = 0; i < PIZZA_COUNT; ++i) {
        sem_operation2(-1, TABLE, TABLE_READY_PIZZAS);
        int pizza_index = find_ready_pizza_index();
        int pizza_type = pizzeria_ptr->table_space[pizza_index];
        pizzeria_ptr->table_space[pizza_index] = FREE_SPACE;
        pizzeria_ptr->pizzas_on_table--;
        print_timestamp();
        printf("Pobieram pizze: %d. Liczba pizz na stole: %ld.\n", pizza_type,
               pizzeria_ptr->pizzas_on_table);
        sem_operation2(1, TABLE, TABLE_FREE_SPOTS);

        randsleep(4);
        print_timestamp();
        printf("Dostarczam pizze: %d.\n", pizza_type);
        randsleep(4);
    }
}
