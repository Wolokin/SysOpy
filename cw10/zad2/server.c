#include "common.h"

int localsockfd;
int netsockfd;
int epollfd;

char* local_path;

void ext() { exit(0); }

typedef struct client_t {
    bool taken;
    bool is_playing;
    int sockfd;
    char name[CLIENT_NAME_LEN];
    pthread_mutex_t mutex;
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
    shutdown(localsockfd, SHUT_RDWR);
    close(localsockfd);
    shutdown(netsockfd, SHUT_RDWR);
    close(netsockfd);
    unlink(local_path);
    safe_print("cleanup\n");
}

int initialize_socket(int type, struct sockaddr* addr, size_t len) {
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
    close(clients[id].sockfd);
    safe_print("Disconnected client %d %s\n", id, clients[id].name);
}

void handle_special_msg(int id, message_t* msg) {
    switch (msg->type) {
        case msg_disconnect:
            disconnect_client(id);
            break;
        default:
            return;
    }
}

void game_loop(int player1, int player2) {
    safe_print("Game started between %d and %d\n", player1, player2);

    int socks[2] = {clients[player1].sockfd, clients[player2].sockfd};
    int ids[2] = {player1, player2};
    char markers[2] = "OX";

    message_t msg = {.type = msg_start_game};
    msg.data.init.symbol = markers[0];
    strncpy(msg.data.init.name, clients[player2].name, CLIENT_NAME_LEN);
    write(socks[0], &msg, sizeof(msg));
    msg.data.init.symbol = markers[1];
    strncpy(msg.data.init.name, clients[player1].name, CLIENT_NAME_LEN);
    write(socks[1], &msg, sizeof(msg));

    char board[9] = "---------";
    msg.type = msg_board_update;
    strncpy(msg.data.board, board, 9);
    write(socks[0], &msg, sizeof(msg));
    write(socks[1], &msg, sizeof(msg));
    char winner = '-';
    int c = 0;
    while ((winner = check_winning_conditions(board)) == '-') {
        msg.type = msg_move;
        critical_section(&clients[ids[c % 2]].mutex, {
            write(socks[c % 2], &msg, sizeof(message_t));
            read(socks[c % 2], &msg, sizeof(message_t));
        });
        if (msg.type == msg_disconnect) {
            critical_section(&clients[ids[c % 2]].mutex,
                             { disconnect_client(ids[c % 2]); });
            msg.type = msg_disconnect;
            write(socks[(c + 1) % 2], &msg, sizeof(message_t));
            msg.type = msg_waiting_for_player;
            write(socks[(c + 1) % 2], &msg, sizeof(message_t));
            return;
        }
        int pos = msg.data.move_position - 1;
        if (pos < 0 || pos >= 9 || board[msg.data.move_position - 1] != '-') {
            // Illegal move!
            continue;
        }
        board[pos] = markers[c % 2];
        c++;
        msg.type = msg_board_update;
        strncpy(msg.data.board, board, 9);
        write(socks[0], &msg, sizeof(msg));
        write(socks[1], &msg, sizeof(msg));
    }
    msg.type = msg_end_game;
    msg.data.winner = winner;
    write(socks[0], &msg, sizeof(msg));
    write(socks[1], &msg, sizeof(msg));
}

typedef struct accepting_thread_args {
    int sockfd;
} accepting_thread_args;
void* accepting_thread(void* args) {
    safe_print("Connection request received\n");

    int sockfd = ((accepting_thread_args*)(args))->sockfd;
    free(args);
    int clientfd = accept(sockfd, NULL, NULL);

    message_t msg;
    read(clientfd, &msg, sizeof(msg));
    assert(msg.type == msg_connect);
    critical_section(&clients_mutex, {
        // Validate username
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
            if (strncmp(msg.data.init.name, clients[i].name, CLIENT_NAME_LEN) ==
                0) {
                write(clientfd, &(message_t){.type = msg_name_taken},
                      sizeof(message_t));
                pthread_mutex_unlock(&clients_mutex);
                safe_print("Duplicate name, rejecting connection...\n");
                return NULL;
            }
        }
        // Add to clients list
        int ind;
        for (ind = 0; ind < MAX_CLIENT_COUNT; ++ind) {
            if (clients[ind].taken == false) {
                clients[ind].is_playing = true;
                clients[ind].sockfd = clientfd;
                strncpy(clients[ind].name, msg.data.init.name, CLIENT_NAME_LEN);
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
                game_loop(i, ind);
                return NULL;
            }
        }
        clients[ind].is_playing = false;
    });
    write(clientfd, &(message_t){.type = msg_waiting_for_player},
          sizeof(message_t));
    return NULL;
}

void* pinger_thread(void* _) {
    int fds = epoll_create1(0);
    struct epoll_event events;
    message_t msg = {.type = msg_ping};
    while (true) {
        sleep(5);
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
        critical_section(&clients[i].mutex, {
                if (clients[i].taken) {
                    epoll_ctl(fds, EPOLL_CTL_ADD, clients[i].sockfd,
                              &(struct epoll_event){
                                  .events = EPOLLIN,
                                  .data = {.fd = clients[i].sockfd},
                              });
                    write(clients[i].sockfd, &msg, sizeof(msg));
                    int available = epoll_wait(fds, &events, 1, TIMEOUT);
                    epoll_ctl(fds, EPOLL_CTL_DEL, clients[i].sockfd,
                              &(struct epoll_event){
                                  .events = EPOLLIN,
                                  .data = {.fd = clients[i].sockfd},
                              });
                    if (available <= 0) {
                        safe_print("Client %d %s got timed out...\n", i,
                                   clients[i].name);
                        disconnect_client(i);
                    } else {
                        read(clients[i].sockfd, &msg, sizeof(msg));
                    }
                });
        }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
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
        pthread_mutex_init(&clients[i].mutex, NULL);
    }

    int port = strtol(argv[1], NULL, 10);
    local_path = argv[2];

    struct sockaddr_un addr_local_tmp = {
        .sun_family = AF_UNIX,
    };
    strcpy(addr_local_tmp.sun_path, local_path);
    struct sockaddr* addr_local = (struct sockaddr*)&(addr_local_tmp);
    size_t addr_local_size = sizeof(addr_local_tmp);

    struct sockaddr_in addr_net_temp = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };
    struct sockaddr* addr_net = (struct sockaddr*)&(addr_net_temp);
    size_t addr_net_size = sizeof(addr_net_temp);

    localsockfd = initialize_socket(AF_UNIX, addr_local, addr_local_size);
    safe_print("Initialized local socket\n");
    netsockfd = initialize_socket(AF_INET, addr_net, addr_net_size);
    safe_print("Initialized web socket\n");

    listen(localsockfd, MAX_QUEUE_LEN);
    listen(netsockfd, MAX_QUEUE_LEN);

    epollfd = epoll_create1(0);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, localsockfd,
              &(struct epoll_event){
                  .events = EPOLLIN,
                  .data = {.fd = localsockfd},
              });
    epoll_ctl(epollfd, EPOLL_CTL_ADD, netsockfd,
              &(struct epoll_event){
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
            pthread_t acceptor;
            accepting_thread_args* args = malloc(sizeof(accepting_thread_args));
            args->sockfd = events[i].data.fd;
            pthread_create(&acceptor, NULL, accepting_thread, args);
            pthread_detach(acceptor);
        }
    }

    return 0;
}
