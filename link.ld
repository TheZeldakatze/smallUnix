ENTRY(_start)

/* for grub reasons we have to do that multiboot thing here */
SECTIONS
{
    . = 0x100000;
    
    /* define the kernel start */
		kernel_start = .;

    .text : {
        *(multiboot)
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
    
    /* define the kernel end */
    . = ALIGN(4096);
		kernel_end = .;
}