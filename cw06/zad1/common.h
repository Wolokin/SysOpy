#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Queue specs
#define get_server_key() ftok(getenv("HOME"), PROJ_SERVER)
#define get_random_key() \
    (srand(getpid()), ftok(getenv("HOME"), rand() % MAX_CLIENTS))

#define MAX_CLIENTS 255
#define MAX_RETRY 100

#define PROJ_SERVER 's'
char PROJ[MAX_CLIENTS] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};

#define MAX_MSG 2000
#define DEF_SIZE 12  // Size of default message consisting only of a number

typedef struct msgbuf {
    long mtype;
    char text[MAX_MSG];
} msgbuf;

// Commands
#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define TEXT 6

#endif  // COMMON_H
