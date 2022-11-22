ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

PHDRS
{
   code_segment PT_LOAD;
   data_segment PT_LOAD;
}

SECTIONS
{
	/* the program should be linked to 4kb */
	. = 0x1000;
	
	
	
	.text : {
		*(start)
		*(.text)
		*(end)
	} :code_segment
	.data ALIGN(4096) : {
		*(.data)
	} :data_segment
	.rodata ALIGN(4096) : {
		*(.rodata)
	} :data_segment
	.bss ALIGN(4096) : {
		*(.bss)
	} :data_segment
}