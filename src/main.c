#include <main.h>
#include <int.h>
#include <pmm.h>
#include <task.h>
#include <console.h>

void kmain(struct multiboot *mb) {
	pmm_install(mb);
	kputs("smallUnix (c) Maite Gamper\nLoading Kernel ... \n");

	int_install();
	init_multitasking(mb);

	asm volatile("sti");
	while(1);
}
