#include "syscalls.h"

.globl call_syscall_write
call_syscall_write:
    // For convention, w8 is the systemcall number
    mov w8, #SYSCALL_WRITE_NUMBER
    // Then I generate a synchronous exception
    svc #0
    ret

.globl call_syscall_clone
call_syscall_clone:
    // First I save the arguments for the child process
    // because the handler called later overwrites them
    mov x10, x0     // Function
    mov x11, x1     // Argument
    mov x12, x2     // Stack

    // I call the syscall handler for the clone
    mov x0, x2
    mov w8, #SYSCALL_CLONE_NUMBER
    svc 0x0
    cmp x0, #0
    // If the syscall return 0, we are inside the new thread and we need to
    // start it
    beq thread_start
    ret

.globl call_syscall_malloc
call_syscall_malloc:
    mov w8, #SYSCALL_MALLOC_NUMBER
    svc #0
    ret

.globl call_syscall_exit
call_syscall_exit:
    mov w8, #SYSCALL_EXIT_NUMBER
    svc #0
    ret

.globl call_syscall_create_dir
call_syscall_create_dir:
    mov w8, #SYSCALL_CREATE_DIR_NUMBER
    svc #0
    ret

.globl call_syscall_open_dir
call_syscall_open_dir:
    mov w8, #SYSCALL_OPEN_DIR_NUMBER
    svc #0
    ret

.globl call_syscall_open_file
call_syscall_open_file:
    mov w8, #SYSCALL_OPEN_FILE_NUMBER
    svc #0
    ret

.globl call_syscall_close_file
call_syscall_close_file:
    mov w8, #SYSCALL_CLOSE_FILE_NUMBER
    svc #0
    ret

.globl call_syscall_write_file
call_syscall_write_file:
    mov w8, #SYSCALL_WRITE_FILE_NUMBER
    svc #0
    ret

.globl call_syscall_read_file
call_syscall_read_file:
    mov w8, #SYSCALL_READ_FILE_NUMBER
    svc #0
    ret

thread_start:
    mov x29, 0
    mov x0, x11
    blr x10

    mov w8, #SYSCALL_EXIT_NUMBER
    svc 0x0
