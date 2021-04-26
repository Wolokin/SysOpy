#include "common.h"

int client_queue;
char client_queue_name[CLIENT_NAME_SZ];
int chatter_queue;
int server_queue;
int assigned_id;

#define send_default_msg(prio) \
    mq_send(server_queue, (char*)(&assigned_id), sizeof(int), prio)

#define receive_msg(queue, msg, prio) \
    (mq_timedreceive(queue, msg, MSG_SIZE, prio, &RCV_TIMEOUT) != -1)

void init();
void stop_handler();
void disconnect_handler();
void connect_command_handler(char* command);
void connect_handler(char* msg);
void chat_loop();
void list_handler();
void server_stop_handler();

bool is_input();  // Non blocking IO check

void get_queue();
void remove_queue();  // atexit

void set_int_catcher(void (*handler)(int));
void int_catcher(int _signo);

int main(void) {
    get_queue();
    atexit(remove_queue);
    server_queue = mq_open(SERVER_NAME, O_WRONLY);

    set_int_catcher(int_catcher);

    init();

    char msg[MSG_SIZE];
    unsigned int prio;
    char* line = NULL;
    size_t size = 0;
    printf("$ ");
    fflush(stdout);
    while (true) {
        if (receive_msg(client_queue, msg, &prio)) {
            switch (prio) {
                case STOP:
                    server_stop_handler();
                    break;
                case CONNECT:
                    connect_handler(msg);
                    printf("$ ");
                    fflush(stdout);
            }
        } else if (is_input()) {
            getline(&line, &size, stdin);
            if (strncmp("LIST", line, strlen("LIST")) == 0) {
                list_handler();
            } else if (strncmp("CONNECT", line, strlen("CONNECT")) == 0) {
                connect_command_handler(line);
            } else if (strncmp("DISCONNECT", line, strlen("DISCONNECT")) == 0) {
                printf("No connection is present\n");
            } else if (strncmp("STOP", line, strlen("STOP")) == 0) {
                stop_handler();
            }
            printf("$ ");
            fflush(stdout);
        }
    }
    if (line != NULL) free(line);
    return 0;
}

void init() {
    mq_send(server_queue, client_queue_name, CLIENT_NAME_SZ, INIT);
    char msg[MSG_SIZE];
    unsigned int prio = -1;
    mq_receive(client_queue, msg, MSG_SIZE, &prio);
    if (prio != INIT) {
        perror("Init error, bad server reply!\n");
    }
    assigned_id = *((int*)msg);
    printf("Server assigned id: %d\n", assigned_id);
}

void stop_handler() {
    printf("Stopping\n");
    send_default_msg(STOP);
    exit(0);
}

void disconnect_handler() {
    printf("Disconnecting\n");
    mq_close(chatter_queue);
    send_default_msg(DISCONNECT);
}

void connect_command_handler(char* command) {
    strsep(&command, " ");
    int id = strtol(command, NULL, 10);
    int msg[2] = {assigned_id, id};
    mq_send(server_queue, charp_cast(msg), 2 * sizeof(int), CONNECT);
    char chatter_queue_name[MSG_SIZE];
    if (mq_receive(client_queue, chatter_queue_name, MSG_SIZE, NULL) == 0) {
        printf("Requested client is not available...\n");
        return;
    }
    connect_handler(chatter_queue_name);
}

void connect_handler(char* queue_name) {
    chatter_queue = mq_open(queue_name, O_WRONLY);
    chat_loop();
}

void chat_loop() {
    system("clear");
    printf("Entered chat mode\n=======================\n");
    char msg[MSG_SIZE];
    unsigned int prio;
    char* line = NULL;
    size_t size = 0;
    while (true) {
        if (receive_msg(client_queue, msg, &prio)) {
            switch (prio) {
                case STOP:
                    server_stop_handler();
                case DISCONNECT:
                    disconnect_handler();
                    goto out_of_while;
                case TEXT:
                    printf("> %s", msg);
            }
        } else if (is_input()) {
            getline(&line, &size, stdin);
            if (strncmp("LIST", line, strlen("LIST")) == 0) {
                list_handler();
            } else if (strncmp("CONNECT", line, strlen("CONNECT")) == 0) {
                printf("Close current connection first!\n");
            } else if (strncmp("DISCONNECT", line, strlen("DISCONNECT")) == 0) {
                disconnect_handler();
                break;
            } else if (strncmp("STOP", line, strlen("STOP")) == 0) {
                stop_handler();
            } else {
                mq_send(chatter_queue, line, MSG_SIZE, TEXT);
            }
        }
    }
out_of_while:
    if (line != NULL) free(line);
}

void list_handler() {
    send_default_msg(LIST);
    char msg[MSG_SIZE];
    mq_receive(client_queue, msg, MSG_SIZE, NULL);
    printf("%s", msg);
}

void server_stop_handler() {
    printf("Server is shutting down...\n");
    stop_handler();
}

bool is_input() {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    if (poll(fds, 1, IO_TIMEOUT) > 0) {  // 50 ms wait
        if (fds[0].revents & POLLIN) {
            return true;
        } else {
            perror("Something went wrong in poll()\n");
            perror(strerror(errno));
            exit(1);
        }
    }
    return false;
}

void get_queue() {
    sprintf(client_queue_name, CLIENT_NAME, getpid());
    client_queue =
        mq_open(client_queue_name, O_CREAT | O_EXCL | O_RDONLY, 0666, &attr);
    if (client_queue == -1) {
        if (errno == EEXIST) {
            perror("Queue already exists...\n");
            exit(1);
        } else {
            perror(strerror(errno));
        }
    }
}

void remove_queue() { mq_unlink(client_queue_name); }

void set_int_catcher(void (*handler)(int)) {
    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = handler;
    sigaction(SIGINT, &act, NULL);
}

void int_catcher(int _signo) { stop_handler(); }
