#ifndef __IPC_H
#define __IPC_H

#define BODY_MAX_SIZE 256

struct Message {
    struct PCB* source_process;
    struct PCB* destination_process;
    char body[BODY_MAX_SIZE];
};

void send_message(struct PCB* source_process, int destination_process_pid, char* body);
void receive_message(struct Message*);

#endif