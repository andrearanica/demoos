#include "user.h"
#include "user_syscalls.h"

void user_process() {
    while (1) {
        char buffer[40] = "Sono il processo utente!\n";
        call_syscall_write(buffer);
    }
}
