#include "common.h"

#define expected \
    "expected: <name> <local|net> <ip:port|/path/to/server/socket>\n"

int sockfd;

void cleanup() {
    write(sockfd, &(message_t){.type = msg_disconnect}, sizeof(message_t));
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void ext() { exit(0); }

void initialize_unix_socket(char* path) {
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };
    strcpy(addr.sun_path, path);
    sockfd = check(socket(AF_UNIX, CONNECTION_TYPE, 0));
    check(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)));
}

void initialize_ipv4_socket(char* ipv4, uint16_t port) {
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};
    check(inet_pton(AF_INET, ipv4, &addr.sin_addr));
    sockfd = check(socket(AF_INET, CONNECTION_TYPE, 0));
    check(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)));
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
        read(sockfd, &msg, sizeof(message_t));
        switch (msg.type) {
            case msg_waiting_for_player:
                printf("Waiting for other player to join...\n");
                linecount = 0;
                break;
            case msg_start_game:
                printf("Started game with user \'%s\'\n", msg.data.init.name);
                printf("You are playing as \'%c\'\n\n", msg.data.init.symbol);
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
                printf("Game ended, %c\'s are victorious\n", msg.data.winner);
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
    atexit(cleanup);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = ext;
    sigaction(SIGINT, &act, NULL);

    message_t msg = {.type = msg_connect};
    strncpy(msg.data.init.name, argv[1], CLIENT_NAME_LEN);
    write(sockfd, &msg, sizeof(msg));
    main_loop();
    return 0;
}
