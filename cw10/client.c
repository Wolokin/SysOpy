#include "common.h"

#define expected \
    "expected: <name> <local|web> <ip:port|/path/to/server/socket>\n"

int sockfd;

void cleanup() {
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void initialize_unix_socket(char* path) {
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };
    // memset(&addr, 0, sizeof(addr));
    // addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    sockfd = check(socket(AF_UNIX, CONNECTION_TYPE, 0));
    check(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)));
}

void initialize_ipv4_socket(char* ipv4, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    check(inet_pton(AF_INET, ipv4, &addr.sin_addr));
    sockfd = check(socket(AF_INET, CONNECTION_TYPE, 0));
    check(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)));
}

int main(int argc, char* argv[]) {
    if (argc < 3 + 1) {
        perror("Wrong argument count, 3 " expected);
        exit(1);
    }
    if (strcmp(argv[2], "local") == 0) {
        initialize_unix_socket(argv[3]);
    } else if (strcmp(argv[2], "web") == 0) {
        initialize_ipv4_socket(strtok(argv[3], ":"),
                               strtol(strtok(NULL, ":"), NULL, 10));
    } else {
        perror("Wrong connection type, " expected);
        exit(1);
    }
    atexit(cleanup);
    send(sockfd, "uhuhu-client", 20, 0);
    char buf[BUFSIZ];
    read(sockfd, buf, BUFSIZ);
    printf("%s\n", buf);
    return 0;
}