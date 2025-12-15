#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "../drivers/sd/sd_filesystem.h"
#include "fork.h"
#include "allocator.h"
#include "./fat32/fat.h"
#include "utils.h"

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

    int error = fat_dir_create(&dir, complete_path);
    return error;
}

int syscall_open_dir(char* dir_relative_path) {
    char complete_path[128];
    strcpy(complete_path, "/mnt/\0");
    strcat(complete_path, dir_relative_path);

    int error = fat_dir_open(&dir, complete_path);
    return error;
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

    File* file = (struct File*) get_free_page();
    int error = fat_file_open(file, complete_path, flags);

    if (error) {
        return -1;
    }

    current_process->files[file_descriptor] = file;
    return file_descriptor;
}

int syscall_close_file(int file_descriptor) {
    File* file = current_process->files[file_descriptor];
    int error = fat_file_close(file);
    return error;
}

int syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes) {
    File* file = current_process->files[file_descriptor];
    int error = fat_file_write(file, buffer, len, bytes);
    return error;
}

int syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes) {
    File* file = current_process->files[file_descriptor];
    fat_file_seek(file, 0, FAT_SEEK_START);
    int error = fat_file_read(file, buffer, len, bytes);
    return error;
}

void syscall_yield() {
    schedule();
}

void* const sys_call_table[] = {syscall_write, syscall_malloc, syscall_clone, syscall_exit, syscall_create_dir, syscall_open_dir, syscall_open_file, syscall_close_file, syscall_write_file, syscall_read_file, syscall_yield};
