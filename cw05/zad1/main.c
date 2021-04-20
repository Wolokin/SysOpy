#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Max. number of different ingredients ('skladnikow')
#define CHAIN_LIMIT 100

// Max. number of command arguments
#define ARG_LIMIT 100

// Max. line count
#define LINE_LIMIT 100

#define make_reader(fd) \
    close(fd[1]);       \
    dup2(fd[0], STDIN_FILENO)

#define make_writer(fd) \
    close(fd[0]);       \
    dup2(fd[1], STDOUT_FILENO)

#define replace_pipe(fd1, fd2) \
    close(fd1[0]);             \
    close(fd1[1]);             \
    fd1[0] = fd2[0];           \
    fd1[1] = fd2[1]

#define check_execvp(args)           \
    if (execvp(args[0], args) == -1) \
    fprintf(stderr, "%s exec failure!\n %s", args[0], strerror(errno))

// Cutting spaces and newlines from both sides
#define trim_str(name)                    \
    {                                     \
        while (*name == ' ') ++name;      \
        char* p = name;                   \
        while (*p != '\0') {              \
            ++p;                          \
        }                                 \
        --p;                              \
        while (*p == ' ' && *p == '\n') { \
            --p;                          \
        }                                 \
        *p = '\0';                        \
    }

typedef struct command {  // prog
    char** args;
    struct command* next_in_chain;  // for joining command chains
} command;

typedef struct command_chain {  // skladnik
    char* name;
    struct command* head;
    struct command* tail;
} command_chain;

void execute_chains(command* head) {
    int current_pipe[2], previous_pipe[2] = {-1, -1};
    while (head != NULL) {
        pipe(current_pipe);
        if (fork() == 0) {
            make_reader(previous_pipe);
            if (head->next_in_chain != NULL) {
                make_writer(current_pipe);
            }
            check_execvp(head->args);
        }
        replace_pipe(previous_pipe, current_pipe);
        head = head->next_in_chain;
    }
    replace_pipe(previous_pipe, current_pipe);
    while (wait(NULL) > 0) {
    }
}

void parse_command(command* cmd, char* args) {
    char* token;
    cmd->args = malloc((ARG_LIMIT + 1) * sizeof(char*));
    int i = 0;
    while ((token = strsep(&args, " ")) && i < ARG_LIMIT) {
        cmd->args[i++] = token;
    }
    while (i < ARG_LIMIT + 1) {
        cmd->args[i++] = NULL;
    }
}

void parse_chain(command_chain* chain, char* line) {
    char* name = strsep(&line, "=");
    trim_str(name);
    chain->name = name;
    char* args = strsep(&line, "|");
    trim_str(args);
    command* previous = malloc(sizeof(command));
    parse_command(previous, args);
    chain->head = previous;
    command* current;
    while ((args = strsep(&line, "|"))) {
        trim_str(args);
        current = malloc(sizeof(command));
        parse_command(current, args);
        previous->next_in_chain = current;
        previous = current;
    }
    previous->next_in_chain = NULL;
    chain->tail = previous;
}

command* join_chains(command_chain chains[], char* line) {
    char* token = strsep(&line, "|");
    trim_str(token);
    int i = 0;
    // Searching for chain by name
    while (strcmp(chains[i].name, token) != 0) ++i;
    int last_chain_index = i;
    command* result = chains[i].head;
    while ((token = strsep(&line, "|"))) {
        trim_str(token);
        i = 0;
        while (strcmp(chains[i].name, token) != 0) ++i;
        chains[last_chain_index].tail->next_in_chain = chains[i].head;
        last_chain_index = i;
    }
    chains[last_chain_index].tail->next_in_chain = NULL;
    return result;
}

int main(void) {
    FILE* file = fopen("instructions.txt", "r");
    command_chain chains[CHAIN_LIMIT];
    char* lines[LINE_LIMIT];
    for (int i = 0; i < LINE_LIMIT; ++i) lines[i] = NULL;
    size_t read_size = 0;
    int i = 0;
    while (getline(&lines[i], &read_size, file) > 0 && i < CHAIN_LIMIT) {
        if (*lines[i] == ' ' || *lines[i] == '\n') break;
        parse_chain(&(chains[i]), lines[i]);
        ++i;
    }
    size_t chains_count = i++;
    while (getline(&lines[i], &read_size, file) > 0 && i < CHAIN_LIMIT) {
        if (*lines[i] == ' ' || *lines[i] == '\n') break;
        execute_chains(join_chains(chains, lines[i++]));
    }
    ++i;
    for (size_t j = 0; j < chains_count; ++j) {
        while (chains[j].head != chains[j].tail) {
            free(chains[j].head->args);
            command* tmp = chains[j].head->next_in_chain;
            free(chains[j].head);
            chains[j].head = tmp;
        }
        free(chains[j].head->args);
        free(chains[j].head);
    }
    for (int j = 0; j < i; ++j) {
        free(lines[j]);
    }
    return 0;
}
