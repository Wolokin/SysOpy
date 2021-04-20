#include "lib.h"
#include "stdio.h"

#define stats() {\
    print_merged(t);\
    printf("block count: %d\n", get_block_count(t));\
    printf("row count0: %d\n", get_row_count(t, 0));\
    printf("row count1: %d\n", get_row_count(t, 1));\
}

int main() {
    table* t = create_table(3);
    char* sequence[] = {"tests/f1.txt:tests/f2.txt", "tests/f3.txt:tests/f4.txt", "nonexistent1.txt:nonexistent2.txt", "badformat"};
    pair_node* h = create_sequence(3, sequence);
    merge_sequence(h);
    read_merged_sequence(t, h);
    stats();
    remove_block(t, 1);
    remove_block(t, 1);
    remove_row(t,0,2);
    remove_row(t,0,2);
    remove_row(t,0,7);
    remove_row(t,0,12);
    remove_row(t,1,1);
    stats();
    remove_sequence(h);
    remove_table(t);
}
