#include "./user_syscalls.h"

void main() {
    call_syscall_write("Hi! I am a process loaded from the file system!\n");
    call_syscall_exit();
    call_syscall_write("This text should not appear.\n");
}
