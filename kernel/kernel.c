#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"
#include "../drivers/sd/sd_filesystem.h"
#include "../drivers/timer/timer.h"
#include "../drivers/uart/uart.h"
#include "../libs/fork.h"
#include "../libs/scheduler.h"
#include "../libs/syscalls.h"
#include "../libs/utils.h"
#include "../user/user.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define UART_NORMAL_COLOR "\x1B[0m"
#define UART_RED_COLOR "\x1B[31m"
#define UART_GREEN_COLOR "\x1B[32m"
#define UART_YELLOW_COLOR "\x1B[33m"
#define UART_BLUE_COLOR "\x1B[34m"
#define UART_WHITE_COLOR "\x1B[37m"
#define UART_CLEAR_SCREEN "\e[1;1H\e[2J"

#define MAX_PATH 64

void kernel_process();
void user_process_fs();
// void shell();
void normalize_path(char*);

void breakpoint() {}

void handle_help(char* buffer);
void handle_ls(char* buffer, char* working_directory);
void handle_pwd(char* working_directory);
void handle_mkdir(char* buffer, char* working_directory);
void handle_cd(char* buffer, char* working_directory);
void handle_write(char* buffer, char* working_directory);
void handle_show(char* buffer, char* working_directory);
void handle_tree(char *buffer, char *working_directory);
void print_tree(const char *path, int depth);

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

  int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
  if (res < 0) {
      uart_puts("[ERROR] Cannot create kernel process.\n");
  }

  // FIXME if I rmeove this loop the kernel restarts itself
  while (1) {

  }
}

void kernel_process() {
    uart_puts("[DEBUG] Kernel process started.\n");

    unsigned long begin = (unsigned long)&user_begin;
    unsigned long end = (unsigned long)&user_end;
    unsigned long process = (unsigned long)&user_process;
    unsigned long size = (end - begin);

    int error = move_to_user_mode(begin, size, process - begin);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }
}

/*
void shell() {
  char working_directory[64] = "/\0";
  while (1) {
    call_syscall_write(UART_GREEN_COLOR);
    call_syscall_write("demoos:");
    call_syscall_write(UART_BLUE_COLOR);
    call_syscall_write(working_directory);
    call_syscall_write(UART_WHITE_COLOR);
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
    } else if (memcmp(buffer, "tree", 4) == 0) {
      handle_tree(buffer, working_directory);
    } else if (memcmp(buffer, "clear", 5) == 0) {
      call_syscall_write(UART_CLEAR_SCREEN);
    } else if (memcmp(buffer, "write", 5) == 0) {
      handle_write(buffer, working_directory);
    } else if (memcmp(buffer, "show", 4) == 0) {
      handle_show(buffer, working_directory);
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
    call_syscall_write("  tree       - Show directory tree\n");
    call_syscall_write("  mkdir      - Create a directory in the working directory\n");
    call_syscall_write("  write      - Creates a file and writes the given content in it\n");
    call_syscall_write("  show       - Shows the content of the given file\n");
    call_syscall_write("  clear      - Clears the screen\n");
}

void handle_ls(char *buffer, char *working_directory) {
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

    FatEntryInfo *info;
    while (1) {
        memset(info->name, 0, 64);
        int result = call_syscall_get_next_entry(fd, info);
        if (result != 1) {
            break;
        }

        if (info->is_dir) {
            call_syscall_write("\x1b[34m");
            call_syscall_write(info->name);
            call_syscall_write("\x1b[0m");
        } else {
            call_syscall_write(info->name);
        }

        call_syscall_write("\n");
    }

    call_syscall_write("\n");
  }
}

void handle_pwd(char *working_directory) {
  call_syscall_write(working_directory);
  call_syscall_write("\n");
}

void handle_mkdir(char *buffer, char *working_directory) {
  char command[32] = {0};
  char dir_name[32] = {0};
  strsplit(buffer, ' ', command, dir_name);

  if (*dir_name == 0) {
    uart_puts("[SHELL] Error: please specify the directory name.\n");
    return;
  }

  char temp[64];
  memset(temp, 0, 64);
  strcat(temp, working_directory);
  if (dir_name[0] != '/') {
    strcat(temp, "/");
  }
  strcat(temp, dir_name);

  int fd = syscall_create_dir(temp);
  if (fd == -1) {
    call_syscall_write("[SHELL] Cannot create '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
  }
}

void handle_cd(char *buffer, char *working_directory) {
  char command[32] = {0};
  char destination[32] = {0};
  char temp[MAX_PATH] = {0};

  strsplit(buffer, ' ', command, destination);

  int destination_len = strlen(destination);
  if (destination[destination_len - 1] != '/') {
    strcat(destination, "/");
  }

  if (destination[0] == '/') {
    // If the path is absolute I don't need to use working directory
    memcpy(temp, destination, MAX_PATH);
  } else {
    // If the path is relative, I append the working directory
    memcpy(temp, working_directory, MAX_PATH);
    int len = strlen(temp);

    if (len > 2 && temp[len - 1] != '/') {
      temp[len] = '/';
      temp[len + 1] = '\0';
    }
    strcat(temp, destination);
  }

  normalize_path(temp);

  if (call_syscall_open_dir(temp) == -1) {
    call_syscall_write("[SHELL] Error: cannot change directory to '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
    return;
  }

  memcpy(working_directory, temp, MAX_PATH);
}

void handle_show(char* buffer, char* working_directory) {
  char command[32] = {0};
  char destination[32] = {0};
  char temp[MAX_PATH] = {0};

  strsplit(buffer, ' ', command, destination);

  if (destination[0] == '/') {
    // If the path is absolute I don't need to use working directory
    memcpy(temp, destination, MAX_PATH);
  } else {
    // If the path is relative, I append the working directory
    memcpy(temp, working_directory, MAX_PATH);
    int len = strlen(temp);

    if (len > 2 && temp[len - 1] != '/') {
      temp[len] = '/';
      temp[len + 1] = '\0';
    }
    strcat(temp, destination);
  }

  normalize_path(temp);

  int fd = call_syscall_open_file(temp, FAT_READ);
  if (fd == -1) {
    call_syscall_write("[SHELL] Cannot open '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
  }

  if (fd > -1) {
    char file_content[256] = {0};
    int read_bytes;
    int error = call_syscall_read_file(fd, file_content, 256, &read_bytes);
    if (error) {
      call_syscall_write("[SHELL] Cannot read '");
      call_syscall_write(temp);
      call_syscall_write("'.\n");
    } else {
      call_syscall_write(file_content);
      call_syscall_write("\n");
    }

    call_syscall_close_file(fd);
  }
}

void normalize_path(char* path) {
  int read = 0, write = 0;

  while (path[read] != '\0') {
    if (path[read] == '.' &&
        (path[read + 1] == '/' || path[read + 1] == '\0')) {
      // I skip the "./" because it's useless to build the path
      read += (path[read + 1] == '/') ? 2 : 1;
      continue;
    } else if (path[read] == '.' && path[read + 1] == '.' &&
               (path[read + 2] == '/' || path[read + 2] == '\0')) {
      read += (path[read + 2] == '/') ? 3 : 2;

      write--;
      while (write > 0 && path[write - 1] != '/') {
        write--;
      }

      continue;
    }

    path[write] = path[read];
    write++;
    read++;
  }

  path[write] = '\0';
  if (write == 0) {
    path[write] = '/';
    path[write + 1] = '\0';
  }
}

void print_tree(const char *path, int depth) {
  int fd = syscall_open_dir(path);
  if (fd == -1) {
    return;
  }
  FatEntryInfo info;
  while (1) {
    memset(info.name, 0, 64);
    int result = call_syscall_get_next_entry(fd, &info);
    if (result != 1) {
      break;
    }
    if ((memcmp(info.name, ".", 2) == 0) || (memcmp(info.name, "..", 3) == 0)) {
      continue;
    }
    for (int i = 0; i < depth; i++) {
      call_syscall_write("   ");
    }
    if (info.is_dir) {
      call_syscall_write("\x1b[34m");
      call_syscall_write(info.name);
      call_syscall_write("/\x1b[0m\n");
    } else {
      call_syscall_write(info.name);
      call_syscall_write("\n");
    }
    if (info.is_dir) {
      char child_path[64] = {0};
      strcpy(child_path, path);
      int len = strlen(child_path);
      if (child_path[len - 1] != '/') {
        strcat(child_path, "/");
      }
      strcat(child_path, info.name);
      normalize_path(child_path);
      print_tree(child_path, depth + 1);
    }
  }
}
void handle_tree(char *buffer, char *working_directory) {
  char command[32] = {0};
  char target[64] = {0};
  char fullpath[64] = {0};
  strsplit(buffer, ' ', command, target);
  if (target[0] == 0) {
    memcpy(fullpath, working_directory, 64);
  } else if (target[0] == '/') {
    memcpy(fullpath, target, 64);
  } else {
    memcpy(fullpath, working_directory, 64);
    int len = strlen(fullpath);
    if (len > 1 && fullpath[len - 1] != '/') {
      strcat(fullpath, "/");
    }
    strcat(fullpath, target);
  }
  normalize_path(fullpath);
  int fd = syscall_open_dir(fullpath);
  if (fd == -1) {
    call_syscall_write("[SHELL] Error: cannot open directory '");
    call_syscall_write(fullpath);
    call_syscall_write("'.\n");
    return;
  }
  call_syscall_write(fullpath);
  call_syscall_write("\n");
  print_tree(fullpath, 0);
}

void handle_write(char* buffer, char* working_directory) {
  char file_name[16] = {0};
  char file_content[32] = {0};

  char* p = buffer;
  while (*p != '\0' && *p != ' ') {
    p++;
  }

  p++;

  int i = 0;
  while (*p != '\0' && *p != ' ') {
    file_name[i] = *p;
    p++;
    i++;
  }

  file_name[i] = '\0';

  p++;

  i = 0;
  while (*p != '\0') {
    file_content[i] = *p;
    p++;
    i++;
  }

  file_content[i] = '\0';

  char file_path[MAX_PATH] = {0};
  strcat(file_path, working_directory);
  strcat(file_path, file_name);

  int fd = call_syscall_open_file(file_path, FAT_CREATE | FAT_WRITE);
  if (fd == -1) {
    call_syscall_write("[SHELL] Error: cannot open file '");
    call_syscall_write(file_path);
    call_syscall_write("'.\n");
  }

  if (fd != -1 && i > 0) {
    int written_bytes;
    int error = call_syscall_write_file(fd, file_content, 256, &written_bytes);
    if (error) {
      call_syscall_write("[SHELL] Error: cannot write on file '");
      call_syscall_write(file_path);
      call_syscall_write("'.\n");
    }
  }

  call_syscall_close_file(fd);
}
 */
