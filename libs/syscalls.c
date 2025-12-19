#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "../drivers/sd/sd_filesystem.h"
#include "fork.h"
#include "allocator.h"
#include "./fat32/fat.h"
#include "scheduler.h"
#include "utils.h"
#include "../drivers/irq/controller.h"

void syscall_write(char* buffer) {
    uart_puts(buffer);
}

int syscall_clone(unsigned long stack) {
    return fork(0, 0, 0, stack);
}

unsigned long syscall_malloc() {
    unsigned long address = get_free_page();
    if (!address) {
        return -1;
    }
    return address;
}

void syscall_exit() {
    exit_process();
}

int syscall_create_dir(char* dir_relative_path) {
    char complete_path[128];
    strcpy(complete_path, "/mnt/\0");
    strcat(complete_path, dir_relative_path);

    int file_descriptor = -1;
    for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
        if (current_process->files[i] == NULL) {
            file_descriptor = i;
            break;
        }
    }

    Dir* dir = (Dir*) get_free_page();
    int error = fat_dir_create(dir, complete_path);
    if (error) {
        return -1;
    }

    current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
    current_process->files[file_descriptor]->d = dir;

    return file_descriptor;
}

int syscall_open_dir(char* dir_relative_path) {
    char complete_path[128];
    strcpy(complete_path, "/mnt/\0");
    strcat(complete_path, dir_relative_path);

    int file_descriptor = -1;
    for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
        if (current_process->files[i] == NULL) {
            file_descriptor = i;
            break;
        }
    }

    Dir* dir = (Dir*) get_free_page();
    int error = fat_dir_open(dir, complete_path);
    if (error) {
        return -1;
    }

    current_process->files[file_descriptor] = (FatResource*) get_free_page();
    current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
    current_process->files[file_descriptor]->d = dir;

    return file_descriptor;
}

int syscall_open_file(char* file_relative_path, uint8_t flags) {
    char complete_path[128];
    strcpy(complete_path, "/mnt/\0");
    strcat(complete_path, file_relative_path);

    int file_descriptor = -1;
    for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
        if (current_process->files[i] == NULL) {
            file_descriptor = i;
            break;
        }
    }

    File* file = (File*) get_free_page();
    int error = fat_file_open(file, complete_path, flags);

    if (error) {
        return -1;
    }

    current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FILE;
    current_process->files[file_descriptor]->f = file;
    return file_descriptor;
}

int syscall_close_file(int file_descriptor) {
    FatResource* fat_resource = current_process->files[file_descriptor];
    File* file = fat_resource->f;
    int error = fat_file_close(file);
    return error;
}

int syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes) {
    FatResource* fat_resource = current_process->files[file_descriptor];
    File* file = fat_resource->f;
    int error = fat_file_write(file, buffer, len, bytes);
    return error;
}

int syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes) {
    FatResource* fat_resource = current_process->files[file_descriptor];
    File* file = fat_resource->f;
    fat_file_seek(file, 0, FAT_SEEK_START);
    int error = fat_file_read(file, buffer, len, bytes);
    return error;
}

void syscall_yield() {
    schedule();
}

struct PCB* uart_owner = NULL;

int syscall_input(char* buffer, int len) {
    int current_len = 0;

    while (uart_owner != NULL && uart_owner != current_process) {
        schedule();
    }

    uart_owner = current_process;

    while (1) {
        if (uart_buffer[0] == 0) {
            current_process->state = PROCESS_WAITING_UART_INPUT;
            schedule();
        }

        while (uart_buffer[uart_tail] != 0) {
            char c;
            c = uart_buffer[uart_tail];
            uart_putc(c);

            uart_tail = (uart_tail + 1) % UART_BUFFER_SIZE;

            if (c == '\r' || c == '\n') {
                uart_owner = NULL;
                return current_len;
            }

            if (current_len < len) {
                *buffer = c;
                buffer++;
                current_len++;
            }
        }
    }
}

int syscall_get_next_entry(int file_descriptor, FatEntryInfo *entry_info) {
    FatResource* fat_resource = current_process->files[file_descriptor];
    Dir* dir = fat_resource->d;

    DirInfo dir_info;
    int error = fat_dir_read(dir, &dir_info);

    if (error) {
        return -1;
    }

    int size = dir_info.name_len < 255? dir_info.name_len : 255;
    for (int i = 0; i < size; i++) {
        entry_info->name[i] = dir_info.name[i];
    }
    entry_info->is_dir = (dir_info.attr & FAT_ATTR_DIR) ? 1 : 0;

    error = fat_dir_next(dir);
    if (error) {
        return -1;
    }

    return 1;
}

void* const sys_call_table[] = {syscall_write, syscall_malloc, syscall_clone, syscall_exit, syscall_create_dir, syscall_open_dir, syscall_open_file, syscall_close_file, syscall_write_file, syscall_read_file, syscall_yield, syscall_input, syscall_get_next_entry};
