#include "user.h"
#include "user_syscalls.h"

void user_process() {
    char buffer[40] = "Sono il processo utente!\n";
    call_syscall_write(buffer);
    
    int pid = call_syscall_fork();
    if (pid < 0) {
        char buffer0[40] = "Error during fork execution\n";
        call_syscall_write(buffer);
    }

    char buffer1[40] = "-------------------------------\n";
    call_syscall_write(buffer1);
    if (pid == 0) {
        char buffer2[40] = "Sono il processo figlio\n";
        call_syscall_write(buffer2);
    } else {
        char buffer2[40] = "Sono il processo padre\n";
        call_syscall_write(buffer2);
    }
}
