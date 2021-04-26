#include "common.h"

int main(void) {
    // struct mq_attr attr = {0, MAX_MSG_NO, MAX_MSG_SIZE, 0};
    int test = get_server_queue();
    perror(NULL);
    // sleep(10);
}
