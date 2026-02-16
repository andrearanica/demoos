#ifndef __MM_H
#define __MM_H

#define VA_START            0xffff000000000000

#define PHYS_MEMORY_SIZE    0x40000000

#define PAGE_MASK           0xfffffffffffff000
#define PAGE_SHIFT          12
#define TABLE_SHIFT         9
#define SECTION_SHIFT       (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE           (1 << PAGE_SHIFT)
#define SECTION_SIZE        (1 << SECTION_SHIFT)

#define PGD_SIZE            (3 * PAGE_SIZE)

#define PGD_SHIFT			PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			PAGE_SHIFT + TABLE_SHIFT

#define LOW_MEMORY (2 * SECTION_SIZE)   // Kernel memory
#define HIGH_MEMORY 0x3F000000          // IO registers memory

#define PTRS_PER_TABLE	    (1 << TABLE_SHIFT)

#ifndef __ASSEMBLER__

extern unsigned long pgd;

#endif

#endif
