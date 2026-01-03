#ifndef __MM_H
#define __MM_H

#define VA_START            0xffff000000000000

#define PHYS_MEMORY_SIZE    0x40000000

#define PAGE_SHIFT          12
#define TABLE_SHIFT         9
#define SECTION_SHIFT       (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE           (1 << PAGE_SHIFT)
#define SECTION_SIZE        (1 << SECTION_SHIFT)

#define PGD_SIZE            (3 * PAGE_SIZE)

#define PTRS_PER_TABLE	    (1 << TABLE_SHIFT)

extern unsigned long pgd;

#endif
