	.section ".text.boot"

	.globl _start

_start:
	mrs 	x5, mpidr_el1
	and	x5, x5, #3
	cbz	x5, 2f
1:	wfe
	b	1b
2:	ldr 	x5, =_start
	mov	sp, x5
	ldr	x5, =__bss_start
	ldr	w6, =__bss_size
3:	cbz	w6, 4f
	str	xzr, [x5], #8
	sub 	w6, w6, #1
	cbnz	w6, 3b
4:	bl	kernel_main
	b	1b

