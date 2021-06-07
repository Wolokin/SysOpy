#include "common.h"

int localsockfd;
int netsockfd;
int epollfd;

char *local_path;

void ext() { exit(0); }

typedef struct game {
    char board[10];
    char marker;
    int opponent;
} game;

typedef struct client_t {
    union addr addr;
    enum type {
        local, net
    } type;
    bool taken;
    bool is_playing;
    char name[CLIENT_NAME_LEN];
    pthread_mutex_t mutex;
    int responding;
    game game;
} client_t;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
#define safe_print(...)                  \
    {                                    \
        critical_section(&print_mutex, { \
            printf(__VA_ARGS__);         \
            fflush(stdout);              \
        });                              \
    }

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t clients[MAX_CLIENT_COUNT];

void cleanup() {
    close(epollfd);
    close(localsockfd);
    close(netsockfd);
    unlink(local_path);
    safe_print("cleanup\n");
}

int initialize_socket(int type, struct sockaddr *addr, size_t len) {
    int sock = check(socket(type, CONNECTION_TYPE, 0));
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
               sizeof(opt));
    check(bind(sock, addr, len));
    return sock;
}

char check_winning_conditions(char board[9]) {
    // row check
    for (int i = 0; i < 9; i += 3) {
        if (board[i] != '-' && board[i] == board[i + 1] &&
            board[i] == board[i + 2]) {
            return board[i];
        }
    }
    // column check
    for (int i = 0; i < 3; ++i) {
        if (board[i] != '-' && board[i] == board[i + 3] &&
            board[i] == board[i + 6]) {
            return board[i];
        }
    }
    // cross check
    if (board[4] != '-') {
        if (board[0] == board[4] && board[0] == board[8]) {
            return board[0];
        }
        if (board[2] == board[4] && board[2] == board[6]) {
            return board[2];
        }
    }
    // check draw
    for (int i = 0; i < 9; ++i) {
        if (board[i] == '-') return '-';
    }
    return 'd';
}

void disconnect_client(int id) {
    clients[id].taken = false;
    if(clients[id].is_playing) {
        sendto(clients[id].type == net ? netsockfd : localsockfd, &(message_t){.type=msg_disconnect}, sizeof(message_t), 0,
               &clients[clients[id].game.opponent].addr.def, sizeof(union addr));
    }
    safe_print("Disconnected client %d %s\n", id, clients[id].name);
}

typedef struct thread_args {
    int sockfd;
    message_t msg;
    union addr addr;
    socklen_t socklen;
} thread_args;

void *processing_thread(void *args) {
    int sockfd = ((thread_args *) (args))->sockfd;
    message_t msg = ((thread_args *) (args))->msg;
    union addr addr = ((thread_args *) (args))->addr;
    socklen_t socklen = ((thread_args *) (args))->socklen;
    free(args);

    int id;
    for (id = 0; id < MAX_CLIENT_COUNT; ++id) {
        if (strcmp(msg.name, clients[id].name) == 0 && clients[id].taken) {
            break;
        }
    }
    if (id == MAX_CLIENT_COUNT) return NULL;

    int pos;
    switch (msg.type) {
//        case msg_connect:
//            break;
        case msg_disconnect:
            disconnect_client(id);
            break;
        case msg_move:
            pos = msg.data.move_position - 1;
            if (pos < 0 || pos >= 9 || clients[id].game.board[msg.data.move_position - 1] != '-') {
                msg.type = msg_move;
                sendto(sockfd, &msg, sizeof(message_t), 0, &clients[id].addr.def, sizeof(union addr));
                break;
            }
            clients[id].game.board[pos] = clients[id].game.marker;
            clients[clients[id].game.opponent].game.board[pos] = clients[id].game.marker;   
            strncpy(msg.data.board, clients[id].game.board, 9);
            msg.type = msg_board_update;
            sendto(sockfd, &msg, sizeof(message_t), 0, &clients[id].addr.def, sizeof(union addr));
            sendto(sockfd, &msg, sizeof(message_t), 0, &clients[clients[id].game.opponent].addr.def, sizeof(union addr));
            if(check_winning_conditions(clients[id].game.board) != '-') {
                msg.type = msg_end_game;
                msg.data.symbol = check_winning_conditions(clients[id].game.board);
                sendto(sockfd, &msg, sizeof(message_t), 0, &clients[id].addr.def, sizeof(union addr));
                sendto(sockfd, &msg, sizeof(message_t), 0, &clients[clients[id].game.opponent].addr.def, sizeof(union addr));
                clients[id].taken = false;
                clients[clients[id].game.opponent].taken = false;
            }
            else {
                msg.type = msg_move;
                sendto(sockfd, &msg, sizeof(message_t), 0, &clients[clients[id].game.opponent].addr.def, sizeof(union addr));
            }
            break;
        case msg_ping:
        critical_section(&clients[id].mutex, {
            clients[id].responding = CLIENT_MAX_CYCLES;
        })
            break;
        default:
            perror("Received unexpected message, aborting...\n");
            exit(1);
    }
    return NULL;
}

void *accepting_thread(void *args) {
    safe_print("Connection request received\n");

    int sockfd = ((thread_args *) (args))->sockfd;
    message_t msg = ((thread_args *) (args))->msg;
    union addr addr = ((thread_args *) (args))->addr;
    socklen_t socklen = ((thread_args *) (args))->socklen;
    free(args);
    assert(msg.type == msg_connect);
    critical_section(&clients_mutex, {
        // Validate username
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
            if (strncmp(msg.name, clients[i].name, CLIENT_NAME_LEN) ==
                0 && clients[i].taken) {
                sendto(sockfd, &(message_t) {.type = msg_name_taken},
                       sizeof(message_t), 0, &addr.def, socklen);
                pthread_mutex_unlock(&clients_mutex);
                safe_print("Duplicate name, rejecting connection...\n");
                return NULL;
            }
        }
        // Add to clients list
        int ind;
        for (ind = 0; ind < MAX_CLIENT_COUNT; ++ind) {
            if (clients[ind].taken == false) {
                clients[ind].type = addr.def.sa_family == AF_INET ? net : local;
                clients[ind].addr = addr;
                clients[ind].is_playing = true;
                strncpy(clients[ind].name, msg.name, CLIENT_NAME_LEN);
                clients[ind].taken = true;
                safe_print("Client got succesfully assigned id %d\n", ind);
                break;
            }
        }
        if (ind >= MAX_CLIENT_COUNT) {
            perror("Server is full, aborting...\n");
            exit(1);
        }
        // Try to connect with another player
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
            if (clients[i].taken == true && clients[i].is_playing == false) {
                clients[i].is_playing = true;
                pthread_mutex_unlock(&clients_mutex);
                msg.type = msg_start_game;
                clients[i].game.opponent = ind;
                clients[ind].game.opponent = i;
                strncpy(msg.name, clients[i].name, CLIENT_NAME_LEN);
                msg.data.symbol = clients[ind].game.marker = 'O';
                check(sendto(sockfd, &msg,
                             sizeof(message_t), 0, &clients[ind].addr.def, sizeof(union addr)));
                strncpy(msg.name, clients[ind].name, CLIENT_NAME_LEN);
                msg.data.symbol = clients[i].game.marker = 'X';
                check(sendto(sockfd, &msg,
                             sizeof(message_t), 0, &clients[i].addr.def, sizeof(union addr)));
                strncpy(clients[i].game.board, "---------", 9);
                strncpy(clients[ind].game.board, "---------", 9);
                strncpy(msg.data.board, "---------", 9);
                msg.type = msg_board_update;
                check(sendto(sockfd, &msg, sizeof(msg), 0, &clients[i].addr.def, sizeof(union addr)));
                check(sendto(sockfd, &msg, sizeof(msg), 0, &clients[ind].addr.def, sizeof(union addr)));
                msg.type = msg_move;
                check(sendto(sockfd, &msg, sizeof(msg), 0, &clients[i].addr.def, sizeof(union addr)));
                return NULL;
            }
        }
        clients[ind].is_playing = false;
    });

    check(sendto(sockfd, &(message_t) {.type = msg_waiting_for_player},
           sizeof(message_t), 0, &addr.def, socklen));
    return NULL;
}

 void* pinger_thread(void* _) {
     message_t msg = {.type = msg_ping};
     while (true) {
         sleep(5);
         for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
             if (clients[i].taken) {
                 critical_section(&clients[i].mutex,
                                  {
                                      clients[i].responding--;
                                      if (clients[i].responding <= 0) {
                                          disconnect_client(i);
                                      }
                                  })
                     sendto(clients[i].type == local ? localsockfd : netsockfd,
                            &msg, sizeof(msg), 0, &clients[i].addr.def,
                            sizeof(union addr));
             }
         }
     }
     return NULL;
 }

int main(int argc, char *argv[]) {
    if (argc < 2 + 1) {
        perror("Wrong argument count, expected 2\n");
        exit(1);
    }
    atexit(cleanup);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = ext;
    sigaction(SIGINT, &act, NULL);

    for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
        clients[i].is_playing = false;
        clients[i].taken = false;
        clients[i].responding = CLIENT_MAX_CYCLES;
        pthread_mutex_init(&clients[i].mutex, NULL);
    }

    int port = strtol(argv[1], NULL, 10);
    local_path = argv[2];

    struct sockaddr_un addr_local_tmp = {
            .sun_family = AF_UNIX,
    };
    strcpy(addr_local_tmp.sun_path, local_path);
    struct sockaddr *addr_local = (struct sockaddr *) &(addr_local_tmp);
    size_t addr_local_size = sizeof(addr_local_tmp);

    struct sockaddr_in addr_net_temp = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(port),
    };
    struct sockaddr *addr_net = (struct sockaddr *) &(addr_net_temp);
    size_t addr_net_size = sizeof(addr_net_temp);

    localsockfd = initialize_socket(AF_UNIX, addr_local, addr_local_size);
    safe_print("Initialized local socket\n");
    netsockfd = initialize_socket(AF_INET, addr_net, addr_net_size);
    safe_print("Initialized web socket\n");

    epollfd = epoll_create1(0);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, localsockfd,
              &(struct epoll_event) {
                      .events = EPOLLIN,
                      .data = {.fd = localsockfd},
              });
    epoll_ctl(epollfd, EPOLL_CTL_ADD, netsockfd,
              &(struct epoll_event) {
                      .events = EPOLLIN,
                      .data = {.fd = netsockfd},
              });

    pthread_t pinger;
    pthread_create(&pinger, NULL, pinger_thread, NULL);
    pthread_detach(pinger);

    while (true) {
        struct epoll_event events[2];
        int available = epoll_wait(epollfd, events, 2, -1);
        for (int i = 0; i < available; ++i) {
            struct thread_args *args = malloc(sizeof(struct thread_args));
            args->socklen = sizeof(union addr);
            recvfrom(events[i].data.fd, &args->msg, sizeof(message_t), 0, &args->addr.def, &args->socklen);
            args->sockfd = events[i].data.fd;
            pthread_t thread;
            if (args->msg.type == msg_connect) {
                pthread_create(&thread, NULL, accepting_thread, args);
                pthread_detach(thread);
                //accepting_thread(args);
            } else {
                pthread_create(&thread, NULL, processing_thread, args);
                pthread_detach(thread);
                //processing_thread(args);
            }
        }
    }

    return 0;
}
