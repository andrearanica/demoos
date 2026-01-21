#include "allocator.h"
#include "mm.h"
#include "scheduler.h"
#include "../arch/mmu.h"

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
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = process->mm.pgd;
    }
    unsigned long pgd = process->mm.pgd;

    int new_table_entry_created = 0;
    unsigned long pgd_virtual_address = (unsigned long)(pgd + VA_START);

    unsigned long pud = map_table(&pgd_virtual_address, PGD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pud;
    }

    unsigned long pud_virtual_address = (unsigned long)(pud + VA_START);
    unsigned long pmd = map_table(&pud_virtual_address, PUD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pmd;
    }

    unsigned long pmd_virtual_address = (unsigned long)(pmd + VA_START);
    unsigned long pte = map_table(&pmd_virtual_address, PMD_SHIFT, virtual_address, &new_table_entry_created);
    if (new_table_entry_created) {
        // The PUD table has been created and I need to track it
        process->mm.kernel_pages[++process->mm.n_kernel_pages] = pte;
    }

    unsigned long pte_virtual_address = (unsigned long)(pte + VA_START);
    map_table_entry(&pte_virtual_address, virtual_address, page_physical_address);

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
        unsigned long entry = next_level_table | MM_TYPE_PAGE_TABLE;
        table[index] = entry;
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
