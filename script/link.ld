ENTRY(_start)

SECTIONS
{
	. = 0x80000;
	.text :
	{
		KEEP(*(.text.boot))
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

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
}

