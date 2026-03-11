#include "./user_syscalls.h"

void main() {
    while (1) {
        call_syscall_write("123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
    }
}
