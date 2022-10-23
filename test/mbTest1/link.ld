ENTRY(_start)
OUTPUT_FORMAT(binary)

SECTIONS
{
	/* the program should be linked to 2MB */
	. = 0x200000;
	
	
	
	.text : {
		*(start)
		*(.text)
	}
	.data ALIGN(4096) : {
		*(.data)
	}
	.rodata ALIGN(4096) : {
		*(.rodata)
	}
	.bss ALIGN(4096) : {
		*(.bss)
	}
}