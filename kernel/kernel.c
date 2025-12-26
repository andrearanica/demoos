#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"
#include "../drivers/sd/sd_filesystem.h"
#include "../drivers/timer/timer.h"
#include "../drivers/uart/uart.h"
#include "../libs/fork.h"
#include "../libs/scheduler.h"
#include "../libs/syscalls.h"
#include "../libs/utils.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void kernel_process();
void user_process();
void user_process_fs();
void shell();

void handle_help(char* buffer);
void handle_ls(char* buffer, char* working_directory);
void handle_pwd(char* working_directory);
void handle_mkdir(char* buffer, char* working_directory);
void handle_cd(char* buffer, char* working_directory);

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3) {
  uart_init();
  uart_puts("demoOS v.0.0.0\n");
  irq_vector_init();
  uart_puts("[DONE] irq vector init\n");
  timer_init();
  uart_puts("[DONE] timer init\n");
  enable_interrupt_controller();
  uart_puts("[DONE] enable interrupt controller\n");
  enable_irq();
  uart_puts("[DONE] enable irq\n");

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
  char working_directory[64] = "/\0";
  call_syscall_write("[SHELL] Welcome to the shell!\n");
  while (1) {
    call_syscall_write("demoos:");
    call_syscall_write(working_directory);
    call_syscall_write("$ ");

    char buffer[64];
    memset(buffer, 0, 64);

    call_syscall_input(buffer, 64);
    call_syscall_write("\n");

    if (memcmp(buffer, "help", 4) == 0) {
      handle_help(buffer);
    } else if (memcmp(buffer, "ls", 2) == 0) {
      handle_ls(buffer, working_directory);
    } else if (memcmp(buffer, "pwd", 3) == 0) {
      handle_pwd(working_directory);
    } else if (memcmp(buffer, "mkdir", 5) == 0) {
      handle_mkdir(buffer, working_directory);
    } else if (memcmp(buffer, "cd", 2) == 0) {
      handle_cd(buffer, working_directory);
    } else {
      call_syscall_write("[SHELL] Command '");
      call_syscall_write(buffer);
      call_syscall_write("' not found.\n");
    }
  }

  call_syscall_exit();
}

void handle_help(char* buffer) {
    call_syscall_write("[demoos shell - 0.0.1]\n");
    call_syscall_write("Available commands:\n");
    call_syscall_write("  help       - Show this help message\n");
    call_syscall_write("  pwd        - Show the current working directory\n");
    call_syscall_write("  ls         - Show content of the current folder\n");
    call_syscall_write("  mkdir      - Create a directory in the working directory\n");
}

void handle_ls(char* buffer, char* working_directory) {
    int fd = syscall_open_dir(working_directory);
    if (fd == -1) {
        call_syscall_write("[SHELL] Error opening folder '");
        call_syscall_write(buffer);
        call_syscall_write("'.\n");
    }

    FatEntryInfo *info;
    while (1) {
        memset(info->name, 0, 64);
        int result = call_syscall_get_next_entry(fd, info);
        if (result != 1) {
            break;
        }

        if (info->is_dir) {
            call_syscall_write("\x1b[34m ");
            call_syscall_write(info->name);
            call_syscall_write("\x1b[0m");
        } else {
            call_syscall_write(info->name);
        }

        call_syscall_write("\n");
    }
}

void handle_pwd(char* working_directory) {
    call_syscall_write(working_directory);
    call_syscall_write("\n");
}

void handle_mkdir(char* buffer, char* working_directory) {
    char command[32] = {0};
    char dir_name[32] = {0};
    strsplit(buffer, ' ', command, dir_name);

    if (*dir_name == 0) {
        uart_puts("[SHELL] Error: please specify the directory name.\n");
        return;
    }

    char temp[64];
    memset(temp, 0, 64);
    memcpy(temp, working_directory, 64);
    strcat(temp, working_directory);
    strcat(temp, dir_name);

    int fd = syscall_create_dir(dir_name);
        if (fd == -1) {
        call_syscall_write("[SHELL] Cannot create '");
        call_syscall_write(dir_name);
        call_syscall_write("'.\n");
    }
}

void handle_cd(char* buffer, char* working_directory) {
    char command[32] = {0};
    char destination_dir[32] = {0};

    strsplit(buffer, ' ', command, destination_dir);

    if (memcmp(destination_dir, "/", 64) == 0) {
        call_syscall_write("[SHELL] Error: cannot go outside root folder '/'\n");
        return;
    }

    char temp[64];
    memset(temp, 0, 64);
    if (memcmp(destination_dir, "./", 2) == 0) {
        int i = 0;

        int c = 2;
        while (c != 0) {
            destination_dir[i] = destination_dir[i+1];
            i++;

            if (destination_dir[i] == '\0') {
                i = 0;
                c--;
            }
        }

        call_syscall_write("[DEBUG] Clean dir: ");
        call_syscall_write(destination_dir);
        call_syscall_write("\n");

        memcpy(temp, working_directory, 64);
    }

    strcat(temp, destination_dir);

    if (call_syscall_open_dir(temp) == -1) {
        call_syscall_write("[SHELL] Error: cannot change directory to '");
        call_syscall_write(temp);
        call_syscall_write("'.\n");
        return;
    }

    memcpy(working_directory, temp, 64);
}

