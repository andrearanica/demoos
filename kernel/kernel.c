#include <stddef.h>
#include <stdint.h>
#include <string.h>
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
void shell();

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

    int error = move_to_user_mode((unsigned long)&shell);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }
}

void shell() {
    char working_directory[40] = "/\0";
    call_syscall_write("[SHELL] Welcome to the shell!\n");
    while (1) {
        call_syscall_write("> ");
        char buffer[64] = {0};
        call_syscall_input(buffer, 64);
        call_syscall_write("\n");

        if (memcmp(buffer, "help", 4) == 0) {
            call_syscall_write("[demoos shell - 0.0.1]\n");
            call_syscall_write("Available commands:\n");
            call_syscall_write("  help       - Show this help message\n");
            call_syscall_write("  pwd        - Show the current working directory\n");
            call_syscall_write("  ls         - Show content of the current folder\n");
            call_syscall_write("  mkdir      - Create a directory in the working directory\n");
        } else if (memcmp(buffer, "ls", 2) == 0) {
            int fd = syscall_open_dir(working_directory);
            if (fd == -1) {
                call_syscall_write("[SHELL] Error opening folder '");
                call_syscall_write(buffer);
                call_syscall_write("'.\n");
            }

            FatEntryInfo* info;
            while (1) {
                memset(info->name, 0, 64);
                int result = call_syscall_get_next_entry(fd, info);
                if (result != 1) {
                    break;
                }

                if (info->is_dir) {
                    call_syscall_write("[Directory] ");
                } else {
                    call_syscall_write("[File]      ");
                }
                call_syscall_write(info->name);
                call_syscall_write("\n");
            }
        } else if (memcmp(buffer, "pwd", 3) == 0) {
            call_syscall_write(working_directory);
            call_syscall_write("\n");
        } else if (memcmp(buffer, "mkdir", 5) == 0) {
            char command[30];
            char dir_name[30];
            strsplit(buffer, ' ', command, dir_name);
        } else {
            call_syscall_write("[SHELL] Command '");
            call_syscall_write(buffer);
            call_syscall_write("' not found.\n");
        }
    }

    call_syscall_exit();
}

void user_process_fs() {
    int error = call_syscall_create_dir("cartella");
    if (error) {
        uart_puts("[ERROR] Cannot create 'cartella' dir\n");
    } else {
        uart_puts("[DEBUG] Directory 'cartella' created\n");
    }
    call_syscall_exit();
}
