#include "common.h"

int localsockfd;
int netsockfd;

char* local_path;

void cleanup() {
    shutdown(localsockfd, SHUT_RDWR);
    close(localsockfd);
    shutdown(netsockfd, SHUT_RDWR);
    close(netsockfd);
    unlink(local_path);
}

void initialize_socket(struct sockaddr* addr, size_t len) {
    localsockfd = check(socket(AF_UNIX, CONNECTION_TYPE, 0));
    check(bind(localsockfd, addr, len));
}

int main(int argc, char* argv[]) {
    if (argc < 2 + 1) {
        perror("Wrong argument count, expected 2\n");
        exit(1);
    }
    atexit(cleanup);
    int port = strtol(argv[1], NULL, 10);
    local_path = argv[2];

    struct sockaddr addr_local = (struct sockaddr*)((struct sockaddr_un){
        .sun_family = AF_UNIX,
    });
    strcpy(addr_local.sun_path, local_path);

    struct sockaddr_in addr_net = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };

    initialize_socket((struct sockaddr*)&addr_local, sizeof(addr_local));
    initialize_socket((struct sockaddr*)&addr_net, sizeof(addr_net));

    listen(localsockfd, MAX_QUEUE_LEN);
    listen(netsockfd, MAX_QUEUE_LEN);

    int new_socket =

        return 0;
}
