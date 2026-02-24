#ifndef __USER_SYSCALLS_H
#define __USER_SYSCALLS_H

#include "../libs/fat32/fat.h"
#include "../libs/ipc.h"
#include "../libs/syscalls.h"

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
// FIXME use another parameter instead of FatEntryInfo which is a kernel struct
int call_syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

void call_syscall_yield();
int call_syscall_input(char* buffer, int len);
int call_syscall_fork();

void call_syscall_send_message(int destination_pid, char* body);
void call_syscall_receive_message(char* body);

#endif
