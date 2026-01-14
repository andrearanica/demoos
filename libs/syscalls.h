#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#define __NR_SYSCALLS 13

#ifndef __ASSEMBLER__

#include "./fat32/fat.h"

void syscall_write(char* buffer);
int syscall_copy_process();
int syscall_create_dir(char* dir_relative_path);
int syscall_open_dir(char* dir_relative_path);
int syscall_open_file(char* file_relative_path, uint8_t flags);
int syscall_close_file(int file_descriptor);
int syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
int syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
void syscall_yield();
int syscall_input(char* buffer, int len);

typedef struct {
    uint32_t size;
    char name[64];
    bool is_dir;
} FatEntryInfo;
int syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

void call_syscall_write(char* buffer);
unsigned long call_syscall_malloc();
int call_syscall_clone(unsigned long function, unsigned long argument, unsigned long stack);
void call_syscall_exit();

int call_syscall_create_dir(char* dir_relative_path);
int call_syscall_open_dir(char* dir_relative_path);
int call_syscall_open_file(char* file_realtive_path, uint8_t flags);
int call_syscall_close_file(int file_descriptor);
int call_syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

void call_syscall_yield();
int call_syscall_input(char* buffer, int len);

#endif
#endif
