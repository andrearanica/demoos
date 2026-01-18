ENTRY(_start)

SECTIONS
{
	. = 0x800000;
	.text : AT(0x80000)
	{
		KEEP(*(.text.boot))
		*(.text)
	}
	. = ALIGN(4096);

	user_begin = .;
	.text.user : { build/user* (.text) }
	.rodata.user : { build/user* (.rodata) }
	.data.user : { build/user* (.data) }
	.bss.user : { build/user* (.bss) }
	user_end = .;

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

    . = ALIGN(0x8);
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
	.data.pgd : { . += (5 * (1 << 12)); }
}
