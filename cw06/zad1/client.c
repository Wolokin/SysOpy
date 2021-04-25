#include "common.h"

key_t client_key;
int client_queue;
int server_queue;
int assigned_id;

msgbuf default_msg;  // Default msg only with assigned_id assigned by server

void remove_queue() { msgctl(client_queue, IPC_RMID, NULL); }

void init() {
    msgbuf msg;
    msg.mtype = INIT;
    sprintf(msg.text, "%d", client_key);
    msgsnd(server_queue, &msg, DEF_SIZE, 0);
    msgrcv(client_queue, &msg, DEF_SIZE, INIT, 0);
    assigned_id = strtol(msg.text, NULL, 10);
    printf("Server assigned id: %d\n", assigned_id);
    strcpy(default_msg.text, msg.text);
}

void stop_handler() {
    printf("Stopping\n");
    default_msg.mtype = STOP;
    msgsnd(server_queue, &default_msg, 12, 0);
    exit(0);
}

void list_handler() {
    default_msg.mtype = LIST;
    msgsnd(server_queue, &default_msg, DEF_SIZE, 0);
    msgbuf msg;
    msgrcv(client_queue, &msg, MAX_MSG, TEXT, 0);
    printf("%s", msg.text);
}
void server_stop_handler() {
    printf("Server is shutting down...\n");
    stop_handler();
}

// Non blocking IO check
bool is_input() {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    if (poll(fds, 1, 50) > 0) {  // 50 ms wait
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

void disconnect_command_handler() {
    printf("Disconnecting\n");
    default_msg.mtype = DISCONNECT;
    msgsnd(server_queue, &default_msg, DEF_SIZE, 0);
}

void disconnect_msg_handler(int queue) {
    printf("Disconnecting\n");
    default_msg.mtype = DISCONNECT;
    msgsnd(queue, &default_msg, DEF_SIZE, 0);
    msgsnd(server_queue, &default_msg, DEF_SIZE, 0);
}

void chat_loop(int queue) {
    system("clear");
    printf("Entered chat mode\n");
    msgbuf buf;
    char* line = NULL;
    size_t size = 0;
    while (true) {
        if (msgrcv(client_queue, &buf, MAX_MSG, STOP, IPC_NOWAIT) >= 0) {
            server_stop_handler();
        } else if (msgrcv(client_queue, &buf, MAX_MSG, DISCONNECT,
                          IPC_NOWAIT) >= 0) {
            disconnect_command_handler();
            break;
        } else if (msgrcv(client_queue, &buf, MAX_MSG, TEXT, IPC_NOWAIT) >= 0) {
            printf("> %s", buf.text);
        } else if (is_input()) {
            getline(&line, &size, stdin);
            if (strncmp("LIST", line, strlen("LIST")) == 0) {
                list_handler();
            } else if (strncmp("CONNECT", line, strlen("CONNECT")) == 0) {
                printf("Close current connection first!\n");
            } else if (strncmp("DISCONNECT", line, strlen("DISCONNECT")) == 0) {
                disconnect_msg_handler(queue);
                break;
            } else if (strncmp("STOP", line, strlen("STOP")) == 0) {
                disconnect_msg_handler(queue);
                stop_handler();
            } else {
                buf.mtype = TEXT;
                strncpy(buf.text, line, MAX_MSG);
                msgsnd(queue, &buf, MAX_MSG, 0);
            }
        }
    }
    if (line != NULL) free(line);
}

void connect_command_handler(int id) {
    msgbuf msg;
    msg.mtype = CONNECT;
    sprintf(msg.text, "%d %d", assigned_id, id);
    msgsnd(server_queue, &msg, DEF_SIZE, 0);
    msgrcv(client_queue, &msg, MAX_MSG, CONNECT, 0);
    int queue = strtol(msg.text, NULL, 10);
    if (queue == server_queue) {
        printf("Requested client is not available...\n");
        return;
    }
    chat_loop(queue);
}

void connect_msg_handler(msgbuf* buf) {
    int queue = strtol(buf->text, NULL, 10);
    chat_loop(queue);
}

void int_catcher(int _signo) { stop_handler(); }

int main(void) {
    struct timespec ts = {0, 1000000};
    for (int i = 0; i < MAX_RETRY; ++i) {
        client_key = get_random_key();
        nanosleep(&ts, NULL);
        client_queue = msgget(client_key, IPC_CREAT | IPC_EXCL | 0666);
        if (client_queue != -1) break;
    }
    if (client_queue == -1) {
        if (errno == EEXIST) {
            perror("Could not get id, queues may be full...\n");
            exit(1);
        } else {
            perror(strerror(errno));
        }
    }
    atexit(remove_queue);
    key_t server_key = get_server_key();
    server_queue = msgget(server_key, 0);

    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = int_catcher;
    sigaction(SIGINT, &act, NULL);

    init();

    msgbuf buf;
    while (true) {
        if (msgrcv(client_queue, &buf, MAX_MSG, STOP, IPC_NOWAIT) >= 0) {
            server_stop_handler();
        } else if (msgrcv(client_queue, &buf, MAX_MSG, CONNECT, IPC_NOWAIT) >=
                   0) {
            connect_msg_handler(&buf);
        } else if (is_input()) {
            char* line = NULL;
            size_t size = 0;
            getline(&line, &size, stdin);
            if (strncmp("LIST", line, strlen("LIST")) == 0) {
                list_handler();
            } else if (strncmp("CONNECT", line, strlen("CONNECT")) == 0) {
                char* tmp = line;
                strsep(&tmp, " ");
                int id = strtol(tmp, NULL, 10);
                connect_command_handler(id);
            } else if (strncmp("DISCONNECT", line, strlen("DISCONNECT")) == 0) {
                printf("No connection is present\n");
            } else if (strncmp("STOP", line, strlen("STOP")) == 0) {
                stop_handler();
            }
            free(line);
        }
    }

    return 0;
}
