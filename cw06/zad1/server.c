#include "common.h"

typedef struct client {
    int queue;
    bool assigned;
    bool available;
} client;

int server_queue;
client clients[MAX_CLIENTS];
int clients_count = 0;

// Non blocking msgrcv, true if msg was in queue
#define receive_msg(type, msg, size) \
    (msgrcv(server_queue, &msg, size, type, IPC_NOWAIT) >= 0)

void stop_handler(msgint1* msg);
void disconnect_handler(msgint1* msg);
void list_handler(msgint1* msg);
void connect_handler(msgint2* msg);
void init_handler(msgint1* msg);

void remove_queue();  // atexit

void set_int_catcher(void (*handler)(int));
void int_catcher(int _signo);

int main(void) {
    key_t server_key = get_server_key();
    server_queue = msgget(server_key, IPC_CREAT | IPC_EXCL | 0666);
    atexit(remove_queue);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].assigned = false;
        clients[i].available = false;
    }

    set_int_catcher(int_catcher);

    msgint1 msg1;
    msgint2 msg2;
    while (true) {
        sleep(1);
        if (receive_msg(STOP, msg1, INT1_SZ)) {
            stop_handler(&msg1);
        } else if (receive_msg(DISCONNECT, msg1, INT1_SZ)) {
            disconnect_handler(&msg1);
        } else if (receive_msg(LIST, msg1, INT1_SZ)) {
            list_handler(&msg1);
        } else if (receive_msg(CONNECT, msg2, INT2_SZ)) {
            connect_handler(&msg2);
        } else if (receive_msg(INIT, msg1, INT1_SZ)) {
            init_handler(&msg1);
        }
    }
    return 0;
}

void stop_handler(msgint1* msg) {
    int id = msg->data[0];
    printf("received STOP command from client %d\n", id);
    clients[id].assigned = false;
    clients[id].available = false;
    --clients_count;
    printf("Client count %d\n", clients_count);
}

void disconnect_handler(msgint1* msg) {
    int id = msg->data[0];
    printf("received DISCONNECT command from client %d\n", id);
    clients[id].available = true;
}

void list_handler(msgint1* msg) {
    int id = msg->data[0];
    printf("received LIST command from client %d\n", id);
    msgtext reply;
    reply.mtype = TEXT;
    char* strbuilder = reply.text;
    strbuilder += sprintf(strbuilder, "Available clients:\n");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].assigned && clients[i].available && i != id) {
            strbuilder += sprintf(strbuilder, "Client ID: %d\n", i);
        }
    }
    msgsnd(clients[id].queue, &reply, TEXT_SZ, 0);
}

void connect_handler(msgint2* msg) {
    unsigned id1 = msg->data[0];
    unsigned id2 = msg->data[1];
    printf("received CONNECT command from client %d to %d\n", id1, id2);
    msgint1 reply = {CONNECT, {server_queue}};
    if (!clients[id2].available || id2 > MAX_CLIENTS) {
        msgsnd(clients[id1].queue, &reply, INT1_SZ, 0);
        return;
    }
    clients[id1].available = false;
    clients[id2].available = false;
    reply.data[0] = clients[id2].queue;
    msgsnd(clients[id1].queue, &reply, INT1_SZ, 0);
    reply.data[0] = clients[id1].queue;
    msgsnd(clients[id2].queue, &reply, INT1_SZ, 0);
}

void init_handler(msgint1* msg) {
    printf("received INIT command\n");
    key_t key = msg->data[0];
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (!clients[id].assigned) {
            printf("Assigned id: %d\n", id);
            clients[id].assigned = true;
            clients[id].queue = msgget(key, 0);
            clients[id].available = true;
            msgint1 reply = {INIT, {id}};
            msgsnd(clients[id].queue, &reply, INT1_SZ, 0);
            ++clients_count;
            break;
        }
    }
}

void remove_queue() { msgctl(server_queue, IPC_RMID, NULL); }

void set_int_catcher(void (*handler)(int)) {
    struct sigaction act;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    act.sa_handler = handler;
    sigaction(SIGINT, &act, NULL);
}

void int_catcher(int _signo) {
    msgint1 msg = {STOP, {}};
    for (int id = 0; id < MAX_CLIENTS; ++id) {
        if (clients[id].assigned) {
            msgsnd(clients[id].queue, &msg, INT1_SZ, 0);
        }
    }
    while (clients_count > 0) {
        msgrcv(server_queue, &msg, INT1_SZ, STOP, 0);
        stop_handler(&msg);
    }
    exit(0);
}
