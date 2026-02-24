#include "ipc.h"
#include "scheduler.h"
#include "../drivers/uart/uart.h"
#include "./allocator.h"
#include <stddef.h>

void send_message(struct PCB* source_process, int destination_process_pid, char* body) {
    struct Message* message = (struct Message*)allocate_kernel_page();

    struct PCB* destination_process = NULL;
    for (int i = 0; i < n_processes; i++) {
        if (processes[i]->pid == destination_process_pid) {
            destination_process = processes[i];
        }
    }

    if (destination_process == NULL) {
        return -1;
    }

    message->source_process = current_process;
    message->destination_process = destination_process;
    strcpy(message->body, body);

    destination_process->arrived_messages[destination_process->n_arrived_messages] = message;
    destination_process->n_arrived_messages = (destination_process->n_arrived_messages + 1) % MAX_MESSAGES_PER_PROCESS;

    uart_puts("[DEBUG] Message from "); 
    uart_hex(message->source_process->pid);
    uart_puts(" to ");
    uart_hex(message->destination_process->pid);
    uart_puts(" sent\n");
}