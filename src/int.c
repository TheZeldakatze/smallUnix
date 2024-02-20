/*
 * int.c
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#include <main.h>
#include <int.h>
#include <console.h>
#include <task.h>
#include <syscall.h>
#include <utils.h>

#define GDT_ENTRIES 6

#define GDT_FLAG_PRESENT 0x80
#define GDT_FLAG_DATA    0x02
#define GDT_FLAG_CODE    0x0a
#define GDT_FLAG_TSS     0x09
#define GDT_FLAG_RING0   0x00
#define GDT_FLAG_RING3   0x60
#define GDT_FLAG_SEGMENT 0x10
#define GDT_FLAG_4K     0x800
#define GDT_FLAG_32BIT  0x400

#define IDT_ENTRIES 256
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_GATE    0xe
#define IDT_RING3        0x60

#define IDT_MACRO_ISR(N) idt_set_entry(N, (unsigned) int_stub_##N, 0x8, IDT_FLAG_PRESENT | IDT_FLAG_GATE)
#define IDT_MACRO_IRQ(N) idt_set_entry(N, (unsigned) int_stub_##N, 0x8, IDT_FLAG_PRESENT | IDT_FLAG_GATE)
#define IDT_MACRO_SYSCALL(N) idt_set_entry(N, (unsigned) int_stub_##N, 0x8, IDT_FLAG_PRESENT | IDT_FLAG_GATE | IDT_RING3)

static unsigned long long gdt[GDT_ENTRIES];
static unsigned long tss[32] = {0, 0, 0x10};

struct table_pointer {
	unsigned short limit;
	void* pointer;
} __attribute__((packed));

struct table_pointer gdt_pointer = {
	.pointer = gdt,
	.limit = GDT_ENTRIES * 8 -1
};



struct idt_entry {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char always0;
	unsigned char flags;
	unsigned short offset_high;
} __attribute__((packed));

struct idt_entry idt[IDT_ENTRIES];
struct table_pointer idt_pointer = {
	.pointer = idt,
	.limit = IDT_ENTRIES * sizeof(struct idt_entry) -1
};

//   -------------------ISR----------------- //
extern void int_stub_0();
extern void int_stub_1();
extern void int_stub_2();
extern void int_stub_3();
extern void int_stub_4();
extern void int_stub_5();
extern void int_stub_6();
extern void int_stub_7();
extern void int_stub_8();
extern void int_stub_9();
extern void int_stub_10();
extern void int_stub_11();
extern void int_stub_12();
extern void int_stub_13();
extern void int_stub_14();
extern void int_stub_15();
extern void int_stub_16();
extern void int_stub_17();
extern void int_stub_18();
extern void int_stub_19();
extern void int_stub_20();
extern void int_stub_21();
extern void int_stub_22();
extern void int_stub_23();
extern void int_stub_24();
extern void int_stub_25();
extern void int_stub_26();
extern void int_stub_27();
extern void int_stub_28();
extern void int_stub_29();
extern void int_stub_30();
extern void int_stub_31();
extern void int_stub_32();
extern void int_stub_33();
extern void int_stub_34();
extern void int_stub_35();
extern void int_stub_36();
extern void int_stub_37();
extern void int_stub_38();
extern void int_stub_39();
extern void int_stub_40();
extern void int_stub_41();
extern void int_stub_42();
extern void int_stub_43();
extern void int_stub_44();
extern void int_stub_45();
extern void int_stub_46();
extern void int_stub_47();
extern void int_stub_48();

char *exception_name[] = {
	"Division through zero",
	"Debug",
	"Non Maskable interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"out of Bounds",
	"Invalid Optcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined",
	"Undefined"
};

static int taskswitch_countdown;

static void gdt_set_entry(int id,unsigned int base, unsigned int limit, int flags) {
	gdt[id] = limit & 0xffffLL;
	gdt[id] |= (base & 0xffffffLL) << 16;
	gdt[id] |= (flags & 0xffLL) << 40;
	gdt[id] |= ((limit >> 16) & 0xfLL) << 48;
	gdt[id] |= ((flags >> 8 )& 0xffLL) << 52;
	gdt[id] |= ((base >> 24) & 0xffLL) << 56;
}

static void idt_set_entry(int id, unsigned long handler, unsigned short selector, unsigned char flags) {
	idt[id].always0 = 0;
	idt[id].flags = flags;
	idt[id].selector = selector;
	idt[id].offset_low = (handler & 0xFFFF);
	idt[id].offset_high = (handler >> 16) & 0xFFFF;
}

void int_install() {
	// initialize the gdt
	gdt[0] = 0;
	gdt_set_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32BIT | GDT_FLAG_CODE | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING0);
	gdt_set_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32BIT | GDT_FLAG_DATA | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING0);
	gdt_set_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32BIT | GDT_FLAG_CODE | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
	gdt_set_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32BIT | GDT_FLAG_DATA | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
	gdt_set_entry(5, (unsigned long) tss, sizeof(tss), GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

	// load it
	asm volatile("lgdt %0" : : "m" (gdt_pointer));
	asm volatile(
		"mov $0x10, %ax;"
		"mov %ax, %ds;"
		"mov %ax, %es;"
		"mov %ax, %ss;"
		"ljmp $0x8, $.1;"
		".1:"
	);
	asm volatile("ltr %%ax" : : "a" (5 << 3));

	// move the irqs
	outb(0x20, 0x11);
	outb(0x21, 0x20);
	outb(0x21, 0x04);
	outb(0x21, 0x01);
	outb(0x21, 0x11);
	outb(0x21, 0x28);
	outb(0x21, 0x02);
	outb(0x21, 0x01);
	outb(0x21, 0x0);
	outb(0x21, 0x0);

	// initialize the idt
	IDT_MACRO_ISR(0);  // exceptions
	IDT_MACRO_ISR(1);
	IDT_MACRO_ISR(2);
	IDT_MACRO_ISR(3);
	IDT_MACRO_ISR(4);
	IDT_MACRO_ISR(5);
	IDT_MACRO_ISR(6);
	IDT_MACRO_ISR(7);
	IDT_MACRO_ISR(8);
	IDT_MACRO_ISR(9);
	IDT_MACRO_ISR(10);
	IDT_MACRO_ISR(11);
	IDT_MACRO_ISR(12);
	IDT_MACRO_ISR(13);
	IDT_MACRO_ISR(14);
	IDT_MACRO_ISR(15);
	IDT_MACRO_ISR(16);
	IDT_MACRO_ISR(17);
	IDT_MACRO_ISR(18);
	IDT_MACRO_ISR(19);
	IDT_MACRO_ISR(20);
	IDT_MACRO_ISR(21);
	IDT_MACRO_ISR(22);
	IDT_MACRO_ISR(23);
	IDT_MACRO_ISR(24);
	IDT_MACRO_ISR(25);
	IDT_MACRO_ISR(26);
	IDT_MACRO_ISR(27);
	IDT_MACRO_ISR(28);
	IDT_MACRO_ISR(29);
	IDT_MACRO_ISR(30);
	IDT_MACRO_ISR(31);
	IDT_MACRO_IRQ(32); // irqs
	IDT_MACRO_IRQ(33);
	IDT_MACRO_IRQ(34);
	IDT_MACRO_IRQ(35);
	IDT_MACRO_IRQ(36);
	IDT_MACRO_IRQ(37);
	IDT_MACRO_IRQ(38);
	IDT_MACRO_IRQ(39);
	IDT_MACRO_IRQ(40);
	IDT_MACRO_IRQ(41);
	IDT_MACRO_IRQ(42);
	IDT_MACRO_IRQ(43);
	IDT_MACRO_IRQ(44);
	IDT_MACRO_IRQ(45);
	IDT_MACRO_IRQ(46);
	IDT_MACRO_IRQ(47);
	IDT_MACRO_SYSCALL(48); // syscall

	// load it
	asm volatile("lidt %0" : : "m" (idt_pointer));

	// set the taskswitch countdown
	taskswitch_countdown = 10;
}

struct cpu_state* int_handler(struct cpu_state* cpu) {
	struct cpu_state* new_state = cpu;

	// Exception
	if(cpu->int_no < 32) {
		kputs("EXCEPTION: \n");
		kputs(exception_name[cpu->int_no]);
		kputs(", Code: ");
		kputn(cpu->error);
		task_debug_printTaskList();
		kputs("Current Task was: ");
		kputn(get_current_task_pid());

		asm volatile("cli; hlt");
	}
	else

	// irq
	if(cpu->int_no < 48) {
		if(cpu->int_no == 32) {

			/* only do a task switch every 10th interrupt */
			if(taskswitch_countdown-- < 0) {
				taskswitch_countdown = 1;
				new_state = schedule(cpu);
				tss[1] = (unsigned long) (new_state + 1);
			}
		}

		// send an EOI (end of interrupt)
		if(cpu->int_no >39)
			outb(0xa0, 0x20);
		outb(0x20, 0x20);
	}
	else

	// syscall
	if(cpu->int_no == 48) {
		new_state = handle_syscall(cpu);
		tss[1] = (unsigned long) (new_state + 1);
	}

	return new_state;
}
