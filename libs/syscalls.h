#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#define __NR_SYSCALLS 13

// Each system call is defined by a number used by the assembly
#define SYSCALL_WRITE_NUMBER            0
#define SYSCALL_MALLOC_NUMBER           1
#define SYSCALL_CLONE_NUMBER            2
#define SYSCALL_EXIT_NUMBER             3
#define SYSCALL_CREATE_DIR_NUMBER       4
#define SYSCALL_OPEN_DIR_NUMBER         5
#define SYSCALL_OPEN_FILE_NUMBER        6
#define SYSCALL_CLOSE_FILE_NUMBER       7
#define SYSCALL_WRITE_FILE_NUMBER       8
#define SYSCALL_READ_FILE_NUMBER        9
#define SYSCALL_YIELD_NUMBER            10
#define SYSCALL_INPUT_NUMBER            11
#define SYSCALL_GET_NEXT_ENTRY_NUMBER   12

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
