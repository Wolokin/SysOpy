#include "lib.h"

const char* TEMP_FILE_NAME = "/tmp/merged";

table* create_table(unsigned size) {
    table* t = calloc(1, sizeof(table));
    t->size = size;
    t->current = 0;
    t->count = 0;
    t->blocks = calloc(size, sizeof(block*));
    return t;
}

block* create_block(unsigned size) {
    block* b = calloc(1, sizeof(block));
    b->size = size;
    b->current = 0;
    b->count = 0;
    b->rows = calloc(size, sizeof(char*));
    return b;
}

void concat_filename(char* filename, pair_node* p) {
    strcpy(filename, TEMP_FILE_NAME);
    char* s1 = p->f1 + strlen(p->f1);
    while (s1 > p->f1 && *(s1 - 1) != '/') {
        --s1;
    }
    char* s2 = p->f2 + strlen(p->f2);
    while (s2 > p->f2 && *(s2 - 1) != '/') {
        --s2;
    }
    strcat(filename, s1);
    strcat(filename, s2);
}

pair_node* new_node(char* f1, char* f2) {
    pair_node* n = calloc(1, sizeof(pair_node));
    n->f1 = calloc(strlen(f1) + 1, sizeof(char));
    strcpy(n->f1, f1);
    n->f2 = calloc(strlen(f2) + 1, sizeof(char));
    strcpy(n->f2, f2);
    n->next = NULL;
    return n;
}

void merge_pair(pair_node* p) {
    FILE* f1 = fopen(p->f1, "r");
    FILE* f2 = fopen(p->f2, "r");
    if (f1 == NULL) {
        printf("file '%s' doesn't exist...\n", p->f1);
        if (f2 != NULL) fclose(f2);
        return;
    }
    if (f2 == NULL) {
        printf("file %s doesn't exist...\n", p->f2);
        if (f1 != NULL) fclose(f1);
        return;
    }
    char filename[BUFSIZ];
    concat_filename(filename, p);
    FILE* out = fopen(filename, "w");
    char buff1[BUFSIZ];
    char buff2[BUFSIZ];
    p->linecount = 0;
    while (fgets(buff1, BUFSIZ, f1) != 0 && fgets(buff2, BUFSIZ, f2) != 0) {
        fprintf(out, "%s", buff1);
        fprintf(out, "%s", buff2);
        p->linecount += 2;
    }
    while (fgets(buff1, BUFSIZ, f1) != 0) {
        fprintf(out, "%s", buff1);
        ++p->linecount;
    }
    while (fgets(buff2, BUFSIZ, f2) != 0) {
        fprintf(out, "%s", buff2);
        ++p->linecount;
    }
    fclose(f1);
    fclose(f2);
    fclose(out);
}

unsigned read_merged_block(table* t, pair_node* p) {
    if (t->current >= t->size) {
        printf("Table is full, cannot load any more...");
        return t->current;
    }
    if (p->linecount == 0) return t->current;
    char filename[BUFSIZ];
    concat_filename(filename, p);
    FILE* in = fopen(filename, "r");
    if (in == NULL) return t->current;
    block* b = create_block(p->linecount);
    t->blocks[t->current] = b;
    char buff[BUFSIZ];
    for (unsigned i = 0; i < p->linecount; ++i) {
        if (fgets(buff, BUFSIZ, in) == 0) {
            b->current = i;
            break;
        }
        b->rows[i] = calloc(strlen(buff) + 1, sizeof(char));
        strcpy(b->rows[i], buff);
    }
    fclose(in);
    b->current = p->linecount;
    b->count = p->linecount;
    ++t->count;
    return t->current++;
}

pair_node* create_sequence(unsigned length, char** sequence) {
    pair_node* sentinel = calloc(1, sizeof(pair_node));
    pair_node* p = sentinel;
    sentinel->next = NULL;
    for (unsigned i = 0; i < length; ++i) {
        char pair[BUFSIZ];
        strcpy(pair, sequence[i]);
        char* f1 = strtok(pair, ":");
        char* f2 = strtok(NULL, ":");
        if (f1 == NULL || f2 == NULL) break;
        pair_node* n = new_node(f1, f2);
        p->next = n;
        p = n;
    }
    p = sentinel->next;
    free(sentinel);
    return p;
}

void remove_sequence(pair_node** head) {
    pair_node* h = *head;
    while (h != NULL) {
        free(h->f1);
        free(h->f2);
        pair_node* tmp = h;
        h = h->next;
        free(tmp);
    }
    *head = NULL;
}

void merge_sequence(pair_node* head) {
    while (head != NULL) {
        merge_pair(head);
        head = head->next;
    }
}

void read_merged_sequence(table* t, pair_node* head) {
    while (head != NULL) {
        read_merged_block(t, head);
        head = head->next;
    }
}

void remove_block(table* table, unsigned i) {
    if (i >= table->current) return;
    block* b = table->blocks[i];
    if (b == NULL) return;
    for (unsigned j = 0; j < b->current; ++j) {
        if (b->rows[j] == NULL) continue;
        free(b->rows[j]);
        b->rows[j] = NULL;
    }
    free(b->rows);
    free(b);
    --table->count;
    table->blocks[i] = NULL;
}

void remove_row(table* table, unsigned i, unsigned j) {
    if (i >= table->current) return;
    block* b = table->blocks[i];
    if (b == NULL) return;
    if (j >= b->current || b == NULL) return;
    free(b->rows[j]);
    --b->count;
    b->rows[j] = NULL;
}

void remove_table(table* t) {
    if (t == NULL) return;
    for (unsigned i = 0; i < t->current; ++i) {
        remove_block(t, i);
    }
    free(t->blocks);
    free(t);
}

void print_merged(table* table) {
    printf("=========================\n");
    for (unsigned i = 0; i < table->size; ++i) {
        block* b = table->blocks[i];
        if (b == NULL) continue;
        printf("block%d:\n", i);
        for (unsigned j = 0; j < b->size; ++j) {
            char* line = b->rows[j];
            if (line == NULL) continue;
            printf("%s", line);
        }
        printf("=========================\n");
    }
}

unsigned get_block_count(table* table) { return table->count; }

unsigned get_row_count(table* table, unsigned i) {
    return i < table->current && table->blocks[i] != NULL
               ? table->blocks[i]->count
               : 0;
}
