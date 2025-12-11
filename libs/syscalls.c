#include "syscalls.h"
#include "../drivers/uart/uart.h"
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

// Creates a dir in '/mnt/' and returns 0 if there are no errors
int syscall_create_dir(char* dir_relative_path) {
    Dir dir;
    char* complete_path = "/mnt/";
    strcat(complete_path, dir_relative_path);

    int error = fat_dir_create(&dir, complete_path);
    return error;
}

// Opens a dir in '/mnt/' and returns 0 if there are no errors
int syscall_open_dir(char* dir_relative_path) {
    Dir dir;
    char* complete_path = "/mnt/";
    strcat(complete_path, dir_relative_path);

    int error = fat_dir_open(&dir, complete_path);
    return error;
}

// Opens a file in '/mnt/' and returns 0 if there are no errors
int syscall_open_file(char* file_relative_path, uint8_t flags) {
    File file;
    char* complete_path = "/mnt/";
    strcat(complete_path, file_relative_path);

    int error = fat_file_open(&file, complete_path, flags);
    return error;
}

// Closes a file in '/mnt/' and returns 0 if there are no errors
int syscall_close_file(char* file_relative_path) {
    // FIXME missing implementation
    return 1;
}

void* const sys_call_table[] = {syscall_write, syscall_malloc, syscall_clone, syscall_exit, syscall_create_dir, syscall_open_dir, syscall_open_file, syscall_close_file};
