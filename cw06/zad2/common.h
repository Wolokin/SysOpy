#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
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
//#include <sys/ipc.h>
//#include <sys/msg.h>

// Queue specs
#define MAX_MSG_NO 10
#define MSG_SIZE 4096
struct mq_attr attr = {0, MAX_MSG_NO, MSG_SIZE, 0};
#define SERVER_NAME "/chat_server"
#define CLIENT_NAME "/chat_client%d"
#define CLIENT_NAME_SZ 256
#define MAX_CLIENTS 255

// Client IO check timeout in ms
#define IO_TIMEOUT 50
// Client msg check timeout in {s, ns}
const struct timespec RCV_TIMEOUT = {0, 1000000};

#define int_cast(char_pointer) *((int*)(char_pointer))
#define charp_cast(int_pointer) ((char*)(int_pointer))

// Commands
#define STOP 6
#define DISCONNECT 5
#define LIST 4
#define CONNECT 3
#define INIT 2
#define TEXT 1

#endif  // COMMON_H
