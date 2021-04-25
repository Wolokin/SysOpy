#include "common.h"

key_t client_key;
int client_queue;
int server_queue;
int assigned_id;

msgint1 default_msg;  // Default msg only with id assigned by server
#define send_default_msg(type) \
    default_msg.mtype = type;  \
    msgsnd(server_queue, &default_msg, INT1_SZ, 0);

#define receive_msg(type, msg, size) \
    (msgrcv(client_queue, &msg, size, type, IPC_NOWAIT) >= 0)

void init();
void stop_handler();
void disconnect_command_handler();
void disconnect_msg_handler(int queue);
void connect_command_handler(char* command);
void connect_msg_handler(msgint1* msg);
void chat_loop(int queue);
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
    key_t server_key = get_server_key();
    server_queue = msgget(server_key, 0);

    set_int_catcher(int_catcher);

    init();

    msgint1 msg1;
    char* line = NULL;
    size_t size = 0;
    printf("$ ");
    fflush(stdout);
    while (true) {
        if (receive_msg(STOP, msg1, INT1_SZ)) {
            server_stop_handler();
        } else if (receive_msg(CONNECT, msg1, INT1_SZ)) {
            connect_msg_handler(&msg1);
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
    msgint1 msg = {INIT, {client_key}};
    msgsnd(server_queue, &msg, INT1_SZ, 0);
    msgrcv(client_queue, &msg, INT1_SZ, INIT, 0);
    assigned_id = msg.data[0];
    printf("Server assigned id: %d\n", assigned_id);
    default_msg.data[0] = assigned_id;
}

void stop_handler() {
    printf("Stopping\n");
    send_default_msg(STOP);
    exit(0);
}

void disconnect_command_handler() {
    printf("Disconnecting\n");
    send_default_msg(DISCONNECT);
}

void disconnect_msg_handler(int queue) {
    printf("Disconnecting\n");
    send_default_msg(DISCONNECT);
    msgsnd(queue, &default_msg, INT1_SZ, 0);
}

void connect_command_handler(char* command) {
    strsep(&command, " ");
    int id = strtol(command, NULL, 10);
    msgint2 msg = {CONNECT, {assigned_id, id}};
    msgsnd(server_queue, &msg, INT2_SZ, 0);
    msgint1 msg2;
    msgrcv(client_queue, &msg2, INT1_SZ, CONNECT, 0);
    int queue = msg2.data[0];
    if (queue == server_queue) {
        printf("Requested client is not available...\n");
        return;
    }
    chat_loop(queue);
}

void connect_msg_handler(msgint1* msg) { chat_loop(msg->data[0]); }

void chat_loop(int queue) {
    system("clear");
    printf("Entered chat mode\n=======================\n");
    msgint1 msg;
    msgtext textmsg = {TEXT, {}};
    char* line = NULL;
    size_t size = 0;
    while (true) {
        if (receive_msg(STOP, msg, INT1_SZ)) {
            server_stop_handler();
        } else if (receive_msg(DISCONNECT, msg, INT1_SZ)) {
            disconnect_command_handler();
            break;
        } else if (receive_msg(TEXT, textmsg, TEXT_SZ)) {
            printf("> %s", textmsg.text);
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
                strncpy(textmsg.text, line, TEXT_SZ);
                msgsnd(queue, &textmsg, TEXT_SZ, 0);
            }
        }
    }
    if (line != NULL) free(line);
}

void list_handler() {
    send_default_msg(LIST);
    msgtext msg;
    msgrcv(client_queue, &msg, TEXT_SZ, TEXT, 0);
    printf("%s", msg.text);
}

void server_stop_handler() {
    printf("Server is shutting down...\n");
    stop_handler();
}

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

void get_queue() {
    struct timespec ts = {0, 1000000};
    for (int i = 0; i < MAX_RETRY; ++i) {
        client_key = get_random_key();
        nanosleep(&ts, NULL);
        client_queue = msgget(client_key, IPC_CREAT | IPC_EXCL | 0666);
        if (client_queue != -1) break;
    }
    if (client_queue == -1) {
        if (errno == EEXIST) {
            perror("Could not open queue, there may be too many clients...\n");
            exit(1);
        } else {
            perror(strerror(errno));
        }
    }
}

void remove_queue() { msgctl(client_queue, IPC_RMID, NULL); }

void set_int_catcher(void (*handler)(int)) {
    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = handler;
    sigaction(SIGINT, &act, NULL);
}

void int_catcher(int _signo) { stop_handler(); }
