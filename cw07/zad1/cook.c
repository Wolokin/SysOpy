#include "common.h"

int find_free_spot(int* tab, int sz) {
    for (int i = 0; i < sz; ++i) {
        if (tab[i] == FREE_SPACE) return i;
    }
    perror(
        "Something went wrong, there should be space in the oven but there is "
        "none :(\n");
    exit(EXIT_FAILURE);
}

int main() {
    open_sems(PERMS);
    open_shms(PERMS);
    attach_shared_mem();
    atexit(detach_shared_mem);

    srand(getpid());
    for (int i = 0; i < PIZZA_COUNT; ++i) {
        // Preping pizza
        int pizza_type = rand() % 10;
        print_timestamp();
        printf("Przygotowuje pizze: %d\n", pizza_type);
        sleep(1 + rand() % 2);

        // Adding pizza to oven
        sem_operation2(-1, OVEN, OVEN_FREE_SPOTS);
        int pizza_index = find_free_spot(pizzeria_ptr->oven_space, OVEN_SZ);
        pizzeria_ptr->oven_space[pizza_index] = pizza_type;
        pizzeria_ptr->pizzas_in_oven++;
        print_timestamp();
        printf("Dodalem pizze: %d. Liczba pizz w piecu: %ld\n", pizza_type,
               pizzeria_ptr->pizzas_in_oven);
        sem_operation1(1, OVEN);

        // Taking pizza out of oven
        sleep(1 + rand() % 2);
        sem_operation1(-1, OVEN);
        pizzeria_ptr->oven_space[pizza_index] = FREE_SPACE;
        pizzeria_ptr->pizzas_in_oven--;
        sem_operation2(1, OVEN, OVEN_FREE_SPOTS);

        // Placing pizza on table
        sem_operation2(-1, TABLE, TABLE_FREE_SPOTS);
        int table_index = find_free_spot(pizzeria_ptr->table_space, TABLE_SZ);
        pizzeria_ptr->table_space[table_index] = pizza_type;
        pizzeria_ptr->pizzas_on_table++;
        print_timestamp();
        printf(
            "Wyjmuje pizze: %d. Liczba pizz w piecu: %ld. Liczba pizz na "
            "stole: %ld\n",
            pizza_type, pizzeria_ptr->pizzas_in_oven,
            pizzeria_ptr->pizzas_on_table);
        sem_operation2(1, TABLE, TABLE_READY_PIZZAS);
    }
}
