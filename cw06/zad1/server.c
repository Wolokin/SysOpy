#include "common.h"

#define FREE -1
typedef struct client {
    int queue;
    int id;  // FREE if client stopped
    bool is_available;
} client;

int server_queue;
client clients[MAX_CLIENTS];
int clients_count = 0;

void remove_queue() { msgctl(server_queue, IPC_RMID, NULL); }

void stop_handler(msgbuf* buf) {
    int id = strtol(buf->text, NULL, 10);
    printf("received STOP command from client %d\n", id);
    clients[id].id = FREE;
    clients[id].is_available = false;
    --clients_count;
    printf("Client count %d\n", clients_count);
}

void init_handler(msgbuf* buf) {
    printf("received INIT command\n");
    key_t key = strtol(buf->text, NULL, 10);
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (clients[id].id == FREE) {
            printf("Assigned id: %d\n", id);
            clients[id].id = id;
            clients[id].queue = msgget(key, 0);
            clients[id].is_available = true;
            msgbuf reply;
            reply.mtype = INIT;
            sprintf(reply.text, "%d", id);
            msgsnd(clients[id].queue, &reply, 12, 0);
            ++clients_count;
            break;
        }
    }
}

void disconnect_handler(msgbuf* buf) {
    int id;
    sscanf(buf->text, "%d", &id);
    printf("received DISCONNECT command from client %d\n", id);
    clients[id].is_available = true;
}

void list_handler(msgbuf* buf) {
    int id = strtol(buf->text, NULL, 10);
    printf("received LIST command from client %d\n", id);
    msgbuf reply;
    reply.mtype = TEXT;
    char* strbuilder = reply.text;
    strbuilder += sprintf(strbuilder, "Available clients:\n");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].id != FREE && clients[i].is_available && i != id) {
            strbuilder += sprintf(strbuilder, "Client ID: %d\n", i);
        }
    }
    msgsnd(clients[id].queue, &reply, strlen(reply.text), 0);
}

void connect_handler(msgbuf* buf) {
    unsigned id1, id2;
    sscanf(buf->text, "%d %d", &id1, &id2);
    printf("received CONNECT command from client %d to %d\n", id1, id2);
    msgbuf msg;
    msg.mtype = CONNECT;
    if (!clients[id2].is_available || id2 > MAX_CLIENTS) {
        sprintf(msg.text, "%d", server_queue);
        msgsnd(clients[id1].queue, &msg, DEF_SIZE, 0);
        return;
    }
    clients[id1].is_available = false;
    clients[id2].is_available = false;
    sprintf(msg.text, "%d", clients[id2].queue);
    msgsnd(clients[id1].queue, &msg, DEF_SIZE, 0);
    sprintf(msg.text, "%d", clients[id1].queue);
    msgsnd(clients[id2].queue, &msg, DEF_SIZE, 0);
}

void int_catcher(int _signo) {
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (clients[id].id != FREE) {
            msgbuf msg;
            msg.mtype = STOP;
            msgsnd(clients[id].queue, &msg, 0, 0);
        }
    }
    msgbuf buf;
    while (clients_count > 0) {
        msgrcv(server_queue, &buf, MAX_MSG, STOP, 0);
        stop_handler(&buf);
    }
    exit(0);
}

int main(void) {
    key_t server_key = get_server_key();
    server_queue = msgget(server_key, IPC_CREAT | IPC_EXCL | 0666);
    atexit(remove_queue);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].id = FREE;
        clients[i].is_available = false;
    }

    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = int_catcher;
    sigaction(SIGINT, &act, NULL);

    msgbuf buf;
    while (true) {
        sleep(1);
        if (msgrcv(server_queue, &buf, MAX_MSG, STOP, IPC_NOWAIT) >= 0) {
            stop_handler(&buf);
        } else if (msgrcv(server_queue, &buf, MAX_MSG, DISCONNECT,
                          IPC_NOWAIT) >= 0) {
            disconnect_handler(&buf);
        } else if (msgrcv(server_queue, &buf, MAX_MSG, LIST, IPC_NOWAIT) >= 0) {
            list_handler(&buf);
        } else if (msgrcv(server_queue, &buf, MAX_MSG, CONNECT, IPC_NOWAIT) >=
                   0) {
            connect_handler(&buf);
        } else if (msgrcv(server_queue, &buf, MAX_MSG, INIT, IPC_NOWAIT) >= 0) {
            init_handler(&buf);
        }
    }
    return 0;
}
