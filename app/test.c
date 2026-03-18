#include "../common/user_syscalls.h"

void main(int n_arguments, int arg1, int arg2) {
    call_syscall_write("[TEST]\nI'm loaded from the filesystem.\n");
    
    if (n_arguments == 2) {
        call_syscall_write("TEST ARGUMENTS NUMBER - SUCCESS\n");
    } else {
        call_syscall_write("TEST ARGUMENTS NUMBER - FAIL\n");
    }

    if (arg1 == 67) {
        call_syscall_write("TEST FIRST ARGUMENT - SUCCESS\n");
    } else {
        call_syscall_write("TEST FIRST ARGUMENT - FAIL\n");
    }

    if (arg2 == 100) {
        call_syscall_write("TEST SECOND ARGUMENT - SUCCESS\n");
    } else {
        call_syscall_write("TEST SECOND ARGUMENT - FAIL\n");

    }

    call_syscall_exit();
    call_syscall_write("This text should never appear.\n");
}
