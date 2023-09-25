#ifndef _MAIN_H
#define _MAIN_H

#define PAGE_SIZE 4096

struct cpu_state {
    unsigned long eax, ebx, ecx, edx, esi, edi, ebp;
    unsigned long int_no, error;
    unsigned long eip, cs, eflags, esp, ss;
} __attribute__((packed));

struct multiboot {
	unsigned long flags, mem_lower, mem_upper, bootdevice, cmdline, mods_count;
	void* mods_addr;
	unsigned long syms[4], memmap_length;
	void* memmap_addr;
} __attribute__((packed));

struct multiboot_memmap {
	unsigned long size;
	unsigned long long base,length;
	unsigned long type;
} __attribute__((packed));

struct multiboot_module {
	unsigned long start, end;
	char* name;
	unsigned long reserved;
} __attribute__((packed));

extern void kmain();
extern void panic(char* msg);

#endif
