#include "../common/user_syscalls.h"
#include "../common/string.h"

void main(int n_arguments, char* arg1, char* arg2) {
    call_syscall_write("[TEST]\nI'm loaded from the filesystem.\n");

    if (n_arguments == 2) {
        call_syscall_write("TEST ARGUMENTS NUMBER (EXPECTED 2) - SUCCESS\n");
    } else {
        call_syscall_write("TEST ARGUMENTS NUMBER - FAIL\n");
    }

    call_syscall_write("First argument: "); call_syscall_write(arg1); call_syscall_write("\n");

    call_syscall_exit();
    call_syscall_write("This text should never appear.\n");
}
