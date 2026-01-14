.globl set_pgd
set_pgd:
    msr ttbr0_el1, x0
    tlbi vmalle1is
    DSB ISH
    isb
    ret
