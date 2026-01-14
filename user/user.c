#include "user.h"
#include "user_syscalls.h"

void user_process() {
    while (1) {
        call_syscall_write("Sono il processo utente!");
    }
}
