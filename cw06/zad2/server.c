#include "common.h"

#define FREE -1
typedef struct client {
    int queue;
    char queue_name[CLIENT_NAME_SZ];
    bool assigned;
    int chatting_with;
} client;

int server_queue;
client clients[MAX_CLIENTS];
int clients_count = 0;

void stop_handler(char* msg);
void disconnect_handler(char* msg);
void disconnect_chatter(int id);
void list_handler(char* msg);
void connect_handler(char* msg);
void init_handler(char* msg);

void remove_queue();  // atexit

void set_int_catcher(void (*handler)(int));
void int_catcher(int _signo);

int main(void) {
    server_queue =
        mq_open(SERVER_NAME, O_CREAT | O_EXCL | O_RDONLY, 0666, &attr);
    assert(server_queue != -1);
    atexit(remove_queue);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].assigned = false;
        clients[i].chatting_with = FREE;
    }

    set_int_catcher(int_catcher);

    char msg[MSG_SIZE];
    unsigned int prio;
    while (true) {
        if (mq_receive(server_queue, msg, MSG_SIZE, &prio) != -1) {
            switch (prio) {
                case STOP:
                    stop_handler(msg);
                    break;
                case DISCONNECT:
                    disconnect_handler(msg);
                    break;
                case LIST:
                    list_handler(msg);
                    break;
                case CONNECT:
                    connect_handler(msg);
                    break;
                case INIT:
                    init_handler(msg);
            }
        } else {
            perror(strerror(errno));
        }
    }
    return 0;
}

void stop_handler(char* msg) {
    int id = int_cast(msg);
    printf("received STOP command from client %d\n", id);
    mq_close(clients[id].queue);
    clients[id].assigned = false;
    disconnect_chatter(clients[id].chatting_with);
    clients[id].chatting_with = FREE;
    --clients_count;
    printf("Client count %d\n", clients_count);
}

void disconnect_handler(char* msg) {
    int id = int_cast(msg);
    printf("received DISCONNECT command from client %d\n", id);
    disconnect_chatter(clients[id].chatting_with);
    clients[id].chatting_with = FREE;
}

void disconnect_chatter(int id) {
    if (clients[id].chatting_with != FREE) {
        mq_send(clients[id].queue, "", 0, DISCONNECT);
    }
}

void list_handler(char* msg) {
    int id = int_cast(msg);
    printf("received LIST command from client %d\n", id);
    char reply[MSG_SIZE];
    char* strbuilder = reply;
    strbuilder += sprintf(strbuilder, "Available clients:\n");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].assigned && clients[i].chatting_with == FREE &&
            i != id) {
            strbuilder += sprintf(strbuilder, "Client ID: %d\n", i);
        }
    }
    mq_send(clients[id].queue, reply, MSG_SIZE, LIST);
}

void connect_handler(char* msg) {
    unsigned id1 = int_cast(msg);
    unsigned id2 = int_cast(msg + sizeof(int));
    printf("received CONNECT command from client %d to %d\n", id1, id2);
    if (clients[id2].chatting_with != FREE || id2 > MAX_CLIENTS) {
        mq_send(clients[id1].queue, "", 0, CONNECT);
        return;
    }
    clients[id1].chatting_with = id2;
    clients[id2].chatting_with = id1;
    mq_send(clients[id1].queue, clients[id2].queue_name, CLIENT_NAME_SZ,
            CONNECT);
    mq_send(clients[id2].queue, clients[id1].queue_name, CLIENT_NAME_SZ,
            CONNECT);
}

void init_handler(char* msg) {
    printf("received INIT command\n");
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (!clients[id].assigned) {
            printf("Assigned id: %d\n", id);
            clients[id].assigned = true;
            clients[id].queue = mq_open(msg, O_WRONLY);
            strncpy(clients[id].queue_name, msg, CLIENT_NAME_SZ);
            clients[id].chatting_with = FREE;
            mq_send(clients[id].queue, (char*)&id, sizeof(int), INIT);
            ++clients_count;
            break;
        }
    }
}

void remove_queue() { mq_unlink(SERVER_NAME); }

void set_int_catcher(void (*handler)(int)) {
    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = handler;
    sigaction(SIGINT, &act, NULL);
}

void int_catcher(int _signo) {
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (clients[id].assigned) {
            mq_send(clients[id].queue, "", 0, STOP);
        }
    }
    char msg[MSG_SIZE];
    while (clients_count > 0) {
        mq_receive(server_queue, msg, MSG_SIZE, NULL);
        stop_handler(msg);
    }
    exit(0);
}
