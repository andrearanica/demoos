#include "user.h"
#include "user_syscalls.h"

char* format(char* string, int size, int number, char* dest);

void user_process() {
    int pid = call_syscall_fork();
    if (pid == 0) {
        char sonBuffer[40] = "F";
        while (1) {
            call_syscall_write(sonBuffer);
            call_syscall_yield();
        }
    } else {
        char fatherBuffer[40] = "P";
        while (1) {
            call_syscall_write(fatherBuffer);
            call_syscall_yield();
        }
    }
}

char* format(char* src, int size, int number, char* dest) {
    int src_pointer = 0;
    int dest_pointer = 0;
    
    while(src[src_pointer] != '\0' && src_pointer < size) {
        if (src[src_pointer] != '%') {
            dest[dest_pointer] = src[src_pointer];
            
            dest_pointer++;
            src_pointer++;
            continue;
        }

        // The current char is a '%'
        src_pointer++;
        if (src[src_pointer] == 'd') {
            dest[dest_pointer] = number + '0';
        }

        src_pointer++;
        dest_pointer++;
    }

    dest[dest_pointer] = '\0';
}
