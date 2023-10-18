/*
 * task.c
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#include <console.h>
#include <task.h>
#include <pmm.h>
#include <elf.h>
#include <utils.h>
#include <console.h>

// predefines
struct task_t *load_program(void* start, void* end, struct task_t* old_task);

static void task_idle() {
	while(1) {
		//asm("int $0x30" : : "a" (0), "b" ('0'));
		for(int i = 0; i<10000000;i++);
	}
}

static int NEXT_PID = 1;

struct task_t *first_task, *current_task;

// TODO remove
static void *task2_start, *task2_end;

struct task_t *create_task(void* entry) {
	struct task_t *new_task   = pmm_alloc();
	unsigned char* stack      = pmm_alloc();
	unsigned char* user_stack = pmm_alloc();

	struct cpu_state new_state = {
		.eax = 0,
		.ebx = 0,
		.ecx = 0,
		.edx = 0,
		.esi = 0,
		.edi = 0,
		.ebp = 0,
		.esp = (unsigned long) user_stack+4096,
		.eip = (unsigned long) entry,
		.cs = 0x08,
		.eflags = 0x202,

		.cs  = 0x18 | 0x03,
		.ss  = 0x20 | 0x03,
	};

	// put the cpu_state on the process stack.
	struct cpu_state* state = (void*) (stack + 4096 - sizeof(new_state));
	*state = new_state;

	new_task->state = state;
	new_task->stack = stack;
	new_task->user_stack = user_stack;
	new_task->pid   = NEXT_PID++;
	new_task->pagelist = pmm_alloc();
	new_task->pagelistCounter = 0;

	new_task->parent = (void*) 0;
	new_task->is_forked = 0;

	// add the task to the list
	new_task->next = first_task;
	first_task = new_task;

	return new_task;
}

static inline struct cpu_state *schedule_next_program() {
	// advance in the list
	if(current_task->next != (void*) 0)
		current_task = current_task->next;
	else
		current_task = first_task;

	if(current_task->is_forked) {
		kmemswap(current_task->user_stack, current_task->parent->user_stack, 4096);
	}

	return current_task->state;
}

static void deconstruct_task(struct task_t *task) {
	// TODO
}

int get_current_task_pid() {
	return current_task->pid;
}

struct cpu_state *kill_current_task() {
	struct task_t *old_task = current_task;
	if(first_task == current_task) {
		// cut it out of the list
		first_task = current_task->next;
	} else {

		// find the previous task
		struct task_t *previous_task = first_task;
		while(previous_task->next != current_task) {
			previous_task = previous_task->next;
		}

		// cut it out of the list
		previous_task->next = current_task->next;
	}

	// advance in the list
	if(current_task->next != (void*) 0)
		current_task = current_task->next;
	else
		current_task = first_task;

	// clean up the old task
	deconstruct_task(old_task);

	return current_task->state;
}

struct task_t* fork_current_task(struct cpu_state* current_state) {
	// save the current task
	*current_task->state = *current_state;

	// create the task
	struct task_t* currTask = current_task;
	struct task_t* newTask = create_task(currTask->state->eip);

	// copy the state
	newTask->is_forked = 1;
	newTask->parent = currTask;
	*newTask->state = *currTask->state;

	// copy the memory
	kmemcpy(newTask->user_stack, currTask->user_stack, 0x1000);

	// tell the processes which one is parent and who's child
	currTask->state->eax = newTask->pid;
	newTask->state->eax = 0;

	return newTask;
}

struct cpu_state *exec_current_task() {
	// if the task was forked, move everything back
	if(current_task->is_forked) {
		kmemcpy(current_task->parent->user_stack, current_task->user_stack, 0x1000);
		//for(int i = 0; current_task->pagelistCounter; i++)
		//	kmemcpy(current_task->parent->pagelist[i], current_task->pagelist[i], 0x1000);

		// for this task, we need to allocate a new stack
	}

	// free the old memory
	//for(int i = 0; current_task->pagelistCounter; i++)
	//	pmm_free(current_task->pagelist[i]);

	// load the new program
	load_program(task2_start, task2_end, current_task);

	// load the next program
	return schedule_next_program();
}

struct cpu_state *schedule(struct cpu_state *current_state) {
	// initialize the first task switch
	if(current_task != (void*) 0) {
		current_task->state = current_state;
		if(current_task->is_forked) {
			kmemswap(current_task->user_stack, current_task->parent->user_stack, 4096);
		}
	} else {
		current_task = first_task;
	}

	schedule_next_program();

	return current_task->state;
}

struct task_t *load_program(void* start, void* end, struct task_t* old_task) {
	struct elf_header *program_header = (struct elf_header*) start;

	// first we have to reserve a memory area for the elf image to be loaded to
	unsigned long length = end - start;
	int pagesUsed = length / 4096 + (length % 4096 != 0);
	unsigned char *target = pmm_alloc_range(pagesUsed);

	// check for a valid elf magic
	if(program_header->magic != ELF_MAGIC) {
		kputs("Error loading program! Invalid elf magic");
		return (void*) 0;
	}

	// create the task structure
	struct task_t* task;
	if(old_task == (void*) 0)
		task = create_task((void*) (program_header->entry_posititon + target));
	else {
		task = old_task;
		task->is_forked = 0;

		kmemset(task->user_stack, 0, 4096);

		struct cpu_state new_state = {
			.eax = 0,
			.ebx = 0,
			.ecx = 0,
			.edx = 0,
			.esi = 0,
			.edi = 0,
			.ebp = 0,
			.esp = (unsigned long) task->user_stack+4096,
			.eip = (unsigned long) program_header->entry_posititon + target,
			.cs = 0x08,
			.eflags = 0x202,

			.cs  = 0x18 | 0x03,
			.ss  = 0x20 | 0x03,
		};

		struct cpu_state* state = (void*) (task->stack + 4096 - sizeof(new_state));
		*state = new_state;
		task->state = state;
	}

	// load the segments
	struct elf_program_header_entry *program_header_entry = (void*) (start + program_header->program_header_tbl_offset);
	for(int i = 0; i<program_header->program_header_entry_count; i++, program_header_entry++) {

		// only load the LOAD segments
		if(program_header_entry->type != ELF_PH_TYPE_LOAD)
			continue;

		// the memsize must not be smaller than the file size
		if(program_header_entry->filesize > program_header_entry->memsize) {
			kputs("PANIC: [ELF] filesz > memsize");
			while(1);
		}

		void *src, *dst;
		src = ((void*) (program_header_entry->p_offset + start));
		dst = ((void*) (program_header_entry->vaddr + target)); // we have to offset by the image area in physical memory

		kmemset(dst, 0, program_header_entry->memsize); // TODO we could make it more efficient by
														// only setting the remainder after kmemcpy to the page end
		kmemcpy(dst, src, program_header_entry->filesize);
	}

	// now fix the section table
	/* this is a incomplete stub that will at some point be required to use dynamic libraries
	struct elf_section_table_entry *section_table_entry = (void*) (start + program_header->section_header_tbl_offset);

	// find the global symbol table
	int global_symbol_table_index = -1;
	for(int i = 0; i<program_header->section_header_count; i++) {
		if(section_table_entry[i].type == ELF_SH_DYNSYM) {
			global_symbol_table_index = i;
			break;
		}
	}
	struct elf_sym *global_symbol_table = (struct elf_sym*) (start + section_table_entry[global_symbol_table].offset);
	char *global_strings = start + section_table_entry[section_table_entry[global_symbol_table_index].link].offset;

	// do the relocation
	for(int i = 0; i<program_header->section_header_count; i++) {
		if(section_table_entry[i].type == ELF_SH_REL) {
			struct elf_rel *rel = (struct elf_rel*) (start + (section_table_entry + i));

		}
	}*/

	//task_addPageToPagelist_range(task, target, pagesUsed);

	return task;
}

void init_multitasking(struct multiboot *mb) {
	create_task((void*) task_idle);
	//create_task((void*) task_b);
	struct multiboot_module* mod = mb->mods_addr;
		load_program((void*) mod[0].start, (void*) mod[0].end, (void*) 0);

		// TODO REMOVE!
		task2_start = (void*) mod[1].start;
		task2_end   = (void*) mod[1].end;
		int size = 16;
		void *target = pmm_alloc_range(size);
		kmemcpy(target, task2_start, task2_end-task2_start);
		task2_start = target;
		task2_end = size*4096 + task2_start;

		//load_program((void*) mod[1].start, (void*) mod[1].end, (void*) 0);
		//load_program((void*) mod[0].start, (void*) mod[0].end);
}
