#include <arpa/inet.h>
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mqueue.h>
#include <ncurses.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define CONNECTION_TYPE SOCK_DGRAM

#define CLIENT_NAME_LEN 20

#define MAX_QUEUE_LEN 5

#define MAX_CLIENT_COUNT 10

#define TIMEOUT 1000

#define CLIENT_MAX_CYCLES 5

#define check(retval)                             \
    ({                                            \
        int error = retval;                       \
        if (error == -1) perror(strerror(errno)); \
        assert(error != -1);                      \
        error;                                    \
    })

#define critical_section(mutex, code) \
    pthread_mutex_lock(mutex);        \
    code;                             \
    pthread_mutex_unlock(mutex);

typedef struct message_t {
    enum message_type_t {
        msg_connect,
        msg_disconnect,
        msg_name_taken,
        msg_waiting_for_player,
        msg_start_game,
        msg_board_update,
        msg_move,
        msg_end_game,
        msg_ping,
    } type;
    char name[CLIENT_NAME_LEN];
    union message_data_t {
        char board[9];
        int move_position;
        char symbol;
    } data;
} message_t;

union addr {
    struct sockaddr def;
    struct sockaddr_in net;
    struct sockaddr_un local;
};