#ifndef LIB_H
#define LIB_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

const char* TEMP_FILE_NAME;

typedef struct block {
    unsigned size;
    unsigned current;
    unsigned count;
    char** rows;
} block;

typedef struct table {
    unsigned size;
    unsigned current;
    unsigned count;
    block** blocks;
} table;

typedef struct pair_node {
    char* f1;
    char* f2;
    unsigned linecount;
    struct pair_node* next;
} pair_node;

// Creates main table of size 'size'
table* create_table(unsigned size);
// Merges a file pair defined in pair_node into a temp file /tmp/merged<file1><file2>
void merge_pair(pair_node* p);
// Reads a merged file pair defined in pair_node and saved into a temp file /tmp/merged<file1><file2>
unsigned read_merged_block(table* t, pair_node* p);
// Creates a sequence of file pairs
pair_node* create_sequence(unsigned length, char** sequence);
// Deletes a sequence from memory
void remove_sequence(pair_node** h);
// Merges a sequence of pairs, saves each merge into a separate temp file /tmp/merged<file1><file2>
void merge_sequence(pair_node* head);
// Reads a sequence of pairs from temp files /tmp/merged<file1><file2>
void read_merged_sequence(table* t, pair_node* head);
// Deletes a block at index i in table
void remove_block(table* table, unsigned i);
// Deletes a row at index j from row at index i
void remove_row(table* table, unsigned i, unsigned j);
// Deletes table
void remove_table(table* t);
// Prints all merged files currently loaded into table, taking into account all of the deletes
void print_merged(table* table);
// Prints current block count
unsigned get_block_count(table* table);
// Prints current row count of block at index i
unsigned get_row_count(table* table, unsigned i);

#endif  // LIB_H
