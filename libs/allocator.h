#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#define PAGE_SHIFT 12                   // Page dimension (2^12)
#define TABLE_SHIFT 9                   // Number of entry of the page table (2^9)
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define N_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PROCESS_SIZE 4096

#include <stddef.h>
#include "scheduler.h"
#include "mm.h"

unsigned long get_free_page();
void free_page(unsigned long p);
void memzero(unsigned long src, unsigned long n);
int memcmp(const void *src1, const void *src2, size_t n);
void memset(void *dest, int c, size_t count);
void memcpy(void *dest, void *src, size_t count);
void map_page(struct PCB* process, unsigned long virtual_address, unsigned long page);

int copy_virtual_memory(struct PCB* destination_process);
unsigned long allocate_kernel_page();
unsigned long allocate_user_page(struct PCB* process, unsigned long virtual_address);

void set_pgd(unsigned long pgd);

unsigned long user_to_kernel_address(unsigned long user_virtual_address);

#endif
