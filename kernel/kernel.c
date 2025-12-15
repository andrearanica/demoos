#include <stddef.h>
#include <stdint.h>
#include "../libs/scheduler.h"
#include "../libs/fork.h"
#include "../libs/utils.h"
#include "../libs/syscalls.h"
#include "../drivers/uart/uart.h"
#include "../drivers/timer/timer.h"
#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"
#include "../drivers/sd/sd_filesystem.h"

void kernel_process();
void user_process();
void user_process_fs();
void user_process_print();

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    uart_init();
    uart_puts("Hello, kernel world!\r\n");

    irq_vector_init();
    timer_init();
    enable_interrupt_controller();
    enable_irq();

    int fs_ok = sd_filesystem_init();
    if (fs_ok == SD_FILESYSTEM_INIT_OK) {
        uart_puts("[DEBUG] SD filesystem init successful.\n");
    } else {
        uart_puts("[DEBUG] SD filesystem init error.\n");
    }

    int res = fork(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
}

void kernel_process() {
    uart_puts("Kernel process started.\n");

    int error = move_to_user_mode((unsigned long)&user_process);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }
}

void user_process() {
    call_syscall_write("[DEBUG] User process started.\n");

    unsigned long stack_1 = call_syscall_malloc();
    if (stack_1 < 0) {
        call_syscall_write("[ERROR] Cannot allocate stack for process 1.\n");
    }
    call_syscall_clone(&user_process_print, "1", stack_1);

    unsigned long stack_2 = call_syscall_malloc();
    if (stack_1 < 0) {
        call_syscall_write("[ERROR] Cannot allocate stack for process 1.\n");
    }
    call_syscall_clone(&user_process_print, "2", stack_2);

    call_syscall_exit();
}

void user_process_fs() {
    int error;
    int fd = call_syscall_open_file("prova.txt", FAT_READ | FAT_WRITE | FAT_CREATE);
    if (fd == -1) {
        call_syscall_write("[ERROR] Cannot open file 'prova.txt'.\n");
        call_syscall_exit();
    }

    int* cnt;

    error = call_syscall_write_file(fd, "ciao! sono il primo processo e ho scritto sul file", 51, cnt);
    if (error) {
        call_syscall_write("[DEBUG] Cannot write on file 'prova.txt'.\n");
    } else {
        call_syscall_write("[DEBUG] File 'prova.txt' written.\n");
    }

    char buffer[51];
    error = call_syscall_read_file(fd, buffer, 51, cnt);
    if (error) {
        call_syscall_write("[ERROR] Cannot read file 'prova.txt'.\n");
    } else {
        call_syscall_write("[DEBUG] File read: the content is '");
        call_syscall_write(buffer);
        call_syscall_write("'\n");
    }

    call_syscall_close_file(fd);
}

void user_process_print(char* process_name) {
    while (1) {
        call_syscall_write("Processo ");
        call_syscall_write(process_name);
        call_syscall_write("\n");
        call_syscall_yield();
    }
}
