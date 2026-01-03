ENTRY(_start)

SECTIONS
{
	. = 0x80000;
	.text :
	{
		KEEP(*(.text.boot))
		*(.text)
	}
	. = ALIGN(4096);

	__font_start = .;
	.rodata :
	{
		font/font.o
		*(.rodata)
	}
	. = ALIGN(4096);

	.data :
	{
		*(.data)
	}
	. = ALIGN(4096);

	__bss_start = .;
	.bss :
	{
		*(.bss)
	}
	. = ALIGN(4096);
	__bss_end = .;
	__bss_size = __bss_end - __bss_start;
	. = ALIGN(0x00001000);
	pgd = .;
	.data.pgd : { . += (3 * (1 << 12)); }
}
