#include "user.h"
#include "user_syscalls.h"

void user_process() {
    int pid = call_syscall_fork();
    if (pid == 0) {
        while (1) {
            call_syscall_write("Sono il figlio\n\0");
            call_syscall_yield();
        }
    } else {
        while (1) {
            call_syscall_write("Sono il padre\n\0");
            call_syscall_yield();
        }
    }
}
