#include "allocator.h"
#include "mm.h"
#include "scheduler.h"
#include "../arch/mmu.h"
#include "../drivers/uart/uart.h"

unsigned long map_table(unsigned long* table, unsigned long index_shift, unsigned long virtual_address, int* new_table_entry_created);
void map_table_entry(unsigned long* pte, unsigned long virtual_address, unsigned long page_physical_address);

static unsigned short memory_pages[N_PAGES] = {0};

unsigned long allocate_kernel_page() {
    unsigned long page = get_free_page();
    if (page == 0) {
        return 0;
    }
    return page + VA_START;
}

unsigned long allocate_user_page(struct PCB* process, unsigned long virtual_address) {
    unsigned long page = get_free_page();
    if (page == 0) {
        return 0;
    }
    map_page(process, virtual_address, page);
    return page + VA_START;
}

// Returns the first free page available in the whole memory
unsigned long get_free_page() {
  for (int i = 0; i < N_PAGES; i++) {
    if (memory_pages[i] == 0) {
      memory_pages[i] = 1;
      unsigned long page_physical_address = LOW_MEMORY + (i * PAGE_SIZE);
      memzero(page_physical_address + VA_START, PAGE_SIZE);
      return page_physical_address;
    }
  }
  return 0;
}

// Sets the given page as free
void free_page(unsigned long p) {
  memory_pages[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}


void map_page(struct PCB* process, unsigned long virtual_address, unsigned long page_physical_address) {
    // If the process doesn't have a PGD, I create it in a free page
    if (!process->mm.pgd) {
        process->mm.pgd = get_free_page();
        memzero(process->mm.pgd + VA_START, PAGE_SIZE);
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = process->mm.pgd;
    }
    unsigned long pgd = process->mm.pgd;

    int new_table_entry_created = 0;
    unsigned long* pgd_virtual_address = (unsigned long*)(pgd + VA_START);

    unsigned long pud = map_table(pgd_virtual_address, PGD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pud;
    }

    unsigned long* pud_virtual_address = (unsigned long*)(pud + VA_START);
    unsigned long pmd = map_table(pud_virtual_address, PUD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pmd;
    }

    unsigned long* pmd_virtual_address = (unsigned long*)(pmd + VA_START);
    unsigned long pte = map_table(pmd_virtual_address, PMD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pte;
    }

    unsigned long* pte_virtual_address = (unsigned long*)(pte + VA_START);
    map_table_entry(pte_virtual_address, virtual_address, page_physical_address);

    struct user_page p = {page_physical_address, virtual_address};
    process->mm.user_pages[process->mm.n_user_pages++] = p;
}

// Creates an entry in the given page table that points to the next page table for the given virtual address
// Returns the address of the next table which is pointed by the entry
unsigned long map_table(unsigned long* table, unsigned long index_shift, unsigned long virtual_address, int* new_table_entry_created) {
    // First I extract the index of the given virtual address inside the given table
    unsigned long index = virtual_address >> index_shift;
    index = index & (PTRS_PER_TABLE - 1);

    if (!table[index]) {
        // If the table doesn't have an entry in the index, I need to create it
        *new_table_entry_created = 1;
        unsigned long next_level_table = get_free_page();
        memzero(next_level_table + VA_START, PAGE_SIZE);
        unsigned long entry = next_level_table | MM_TYPE_PAGE_TABLE;
        table[index] = entry;
        return next_level_table;
    } else {
        // Otherwhise the page with the given virtual address is already written in the page table
        *new_table_entry_created = 0;
    }

    unsigned long next_table_address = table[index] & PAGE_MASK;
    return next_table_address;
}

// Writes an entry in the PTE which points to a physical address for the given virtual address
void map_table_entry(unsigned long* pte, unsigned long virtual_address, unsigned long page_physical_address) {
    unsigned long index = virtual_address >> PAGE_SHIFT;
    index = index & (PTRS_PER_TABLE - 1);
    unsigned long entry = page_physical_address | MMU_PTE_FLAGS;
    pte[index] = entry;
}

static int index = -1;

// This function handles the page fault exceptions
int do_mem_abort(unsigned long address, unsigned long esr) {
    unsigned long dfs = (esr & 0b111111);
    if ((dfs / 0b111100) == 0b100) {
        unsigned long page = get_free_page();
        if (page == 0) {
            return -1;
        }
        map_page(current_process, address & PAGE_MASK, page);
        index++;
        if (index > 2) {
            return -1;
        }
        return 0;
    }
    return -1;
}

// Converts the virtual address of the current process in the corresponding physical address
unsigned long user_to_kernel_address(unsigned long user_virtual_address) {
    unsigned long* pgd = (unsigned long*)(current_process->mm.pgd + VA_START);

    unsigned long pud_phys = pgd[(user_virtual_address >> PGD_SHIFT) & (PTRS_PER_TABLE - 1)];
    if (!(pud_phys & 1)) {
        return 0;
    }
    unsigned long* pud = (unsigned long*)((pud_phys & PAGE_MASK) + VA_START);

    unsigned long pmd_phys = pud[(user_virtual_address >> PUD_SHIFT) & (PTRS_PER_TABLE - 1)];
    if (!(pmd_phys & 1)) {
        return 0;
    }
    unsigned long* pmd = (unsigned long*)((pmd_phys & PAGE_MASK) + VA_START);

    unsigned long pte_phys = pmd[(user_virtual_address >> PMD_SHIFT) & (PTRS_PER_TABLE - 1)];
    if (!(pte_phys & 1)) {
        return 0;
    }
    unsigned long* pte = (unsigned long*)((pte_phys & PAGE_MASK) + VA_START);

    unsigned long page_physical_address = pte[(user_virtual_address >> PAGE_SHIFT) & (PTRS_PER_TABLE - 1)];
    if (!(page_physical_address & 1)) {
        return 0;
    }

    unsigned long page_offset = user_virtual_address & (PAGE_SIZE - 1);

    return (page_physical_address & PAGE_MASK) + page_offset + VA_START;
}

int copy_virtual_memory(struct PCB* destination_process) {
    struct PCB* source_process = current_process;
    for (int i = 0; i < source_process->mm.n_user_pages; i++) {
        unsigned long kernel_virtual_address = allocate_user_page(destination_process, source_process->mm.user_pages[i].virtual_address);
        if (kernel_virtual_address == 0) {
            return -1;
        }

        memzero(kernel_virtual_address, PAGE_SIZE);

        unsigned long destination_address = kernel_virtual_address;
        unsigned long source_address = (source_process->mm.user_pages[i].physical_address + VA_START);
        memcpy((void*)destination_address, (void*)source_address, PAGE_SIZE);
    }
    return 0;
}