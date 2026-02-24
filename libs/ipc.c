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
        // FIXME put process in wait
        return -1;
    }
    
    message->source_process = current_process;
    message->destination_process = destination_process;
    strcpy(message->body, body);

    int push_ok = push_message(&destination_process->messages_buffer, message);
    if (push_ok == -1) {
        uart_puts("[DEBUG] Error pushing message\n");
        return -1;
    }
}

void receive_message(struct PCB* destination_process, char* body) {
    // FIXME put process in wait instead of busy waiting
    struct Message* received_message = allocate_kernel_page();

    while (1) {
        // print_circular_buffer(&destination_process->messages_buffer);
        int pop_ok = pop_message(&destination_process->messages_buffer, received_message);
        if (pop_ok == 0) {
            break;
        }
        schedule();
    }
    strcpy(body, received_message->body);
}

// Pushes a message in the given circular buffer; return -1 if an error occoured
int push_message(struct MessagesCircularBuffer* buffer, struct Message* message) {
    int next_head = buffer->head + 1;
    if (next_head >= MAX_MESSAGES_PER_PROCESS) {
        next_head = 0;
    }

    if (next_head == buffer->tail) {
        // It means that the buffer is full
        return -1;
    }

    buffer->buffer[buffer->head] = *message;
    buffer->head = next_head;

    return 0;
}

// Pops the next message from the queue and puts in it message; returns -1 if an error occoured
int pop_message(struct MessagesCircularBuffer* buffer, struct Message* message) {
    if (buffer->head == buffer->tail) {
        // The buffer is empty
        return -1;
    }

    int next_tail = buffer->tail + 1;
    if (next_tail >= MAX_MESSAGES_PER_PROCESS) {
        next_tail = 0;
    }

    message->source_process = buffer->buffer[buffer->tail].source_process;
    message->destination_process = buffer->buffer[buffer->tail].destination_process;
    strcpy(message->body, buffer->buffer[buffer->tail].body);

    buffer->tail = next_tail;

    return 0;
}

void print_circular_buffer(struct MessagesCircularBuffer* buffer) {
    uart_puts("[BUFFER] Head: "); uart_hex(buffer->head);
    uart_puts("| Tail: "); uart_hex(buffer->tail);
    uart_puts("\n");
    for (int i = 0; i < MAX_MESSAGES_PER_PROCESS; i++) {
        uart_hex(i);
        uart_puts("\t");
        uart_puts(buffer->buffer[i].body);
        uart_puts("\n");
    }
}
