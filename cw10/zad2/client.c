#include "common.h"

#define expected \
    "expected: <name> <local|net> <ip:port|/path/to/server/socket>\n"

int sockfd;
union addr servaddr;
char* name;

void cleanup() {
    message_t msg = {.type = msg_disconnect};
    strncpy(msg.name, name, CLIENT_NAME_LEN);
    write(sockfd, &msg, sizeof(message_t));
    close(sockfd);
}

void ext() { exit(0); }

void initialize_unix_socket(char* path) {
    servaddr.local = (struct sockaddr_un) {
        .sun_family = AF_UNIX,
    };
    strncpy(servaddr.local.sun_path, path, sizeof(servaddr.local.sun_path));
    struct sockaddr_un bind_addr = {
            .sun_family = AF_UNIX
    };
    snprintf(bind_addr.sun_path, sizeof(bind_addr.sun_path), "/tmp/%d", getpid());
    sockfd = check(socket(AF_UNIX, CONNECTION_TYPE, 0));
    check(bind(sockfd, (const struct sockaddr*)&bind_addr, sizeof(bind_addr)));
    check(connect(sockfd, &servaddr.def, sizeof(servaddr.local)));
}

void initialize_ipv4_socket(char* ipv4, uint16_t port) {
    servaddr.net = (struct sockaddr_in){.sin_family = AF_INET, .sin_port = htons(port)};
    check(inet_pton(AF_INET, ipv4, &servaddr.net.sin_addr));
    sockfd = check(socket(AF_INET, CONNECTION_TYPE, 0));
    check(connect(sockfd, &servaddr.def, sizeof(servaddr)));
}

void revert_newlines(int linecount) {
    while (linecount--) {
        printf("\033[F");
    }
}

void main_loop() {
    message_t msg;
    size_t linecount = 0;
    while (true) {
        int c = check(read(sockfd, &msg, sizeof(message_t)));
        strncpy(msg.name, name, CLIENT_NAME_LEN);
        switch (msg.type) {
            case msg_waiting_for_player:
                printf("Waiting for other player to join...\n");
                linecount = 0;
                break;
            case msg_start_game:
                printf("Started game with user \'%s\'\n", msg.name);
                printf("You are playing as \'%c\'\n\n", msg.data.symbol);
                sleep(1);
                break;
            case msg_board_update:
                revert_newlines(linecount);
                printf("-------------\n");
                for (int j = 0; j < 9; j += 3) {
                    printf("| %c | %c | %c |\n", msg.data.board[j],
                           msg.data.board[j + 1], msg.data.board[j + 2]);
                    printf("-------------\n");
                }
                printf("Opponent's turn...                      \n");
                linecount = 8;
                break;
            case msg_move:
                revert_newlines(1);
                printf("Your turn, choose position (1-9):     \b\b\b\b");
                scanf("%d", &msg.data.move_position);
                write(sockfd, &msg, sizeof(msg));
                break;
            case msg_end_game:
                revert_newlines(1);
                printf("Game ended, %c\'s are victorious\n", msg.data.symbol);
                return;
            case msg_name_taken:
                printf("That name is already taken, exiting...\n");
                return;
            case msg_disconnect:
                printf("Other player left the game...\n");
                return;
            case msg_ping:
                write(sockfd, &msg, sizeof(msg));
                break;
            default:
                perror("Unexpected server response\n");
                exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3 + 1) {
        perror("Wrong argument count, 3 " expected);
        exit(1);
    }
    if (strcmp(argv[2], "local") == 0) {
        initialize_unix_socket(argv[3]);
    } else if (strcmp(argv[2], "net") == 0) {
        char* ipv4_addr = strtok(argv[3], ":");
        long ipv4_port = strtol(strtok(NULL, ":"), NULL, 10);
        initialize_ipv4_socket(ipv4_addr, ipv4_port);
    } else {
        perror("Wrong connection type, " expected);
        exit(1);
    }
    name = argv[1];
    atexit(cleanup);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = ext;
    sigaction(SIGINT, &act, NULL);

    message_t msg = {.type = msg_connect};
    strncpy(msg.name, argv[1], CLIENT_NAME_LEN);
    write(sockfd, &msg, sizeof(msg));
    main_loop();
    return 0;
}
