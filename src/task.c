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

static int NEXT_PID = 0;

struct task_t *first_task, *current_task, *destroyTaskQueue;

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

	new_task->pageList = pmm_alloc();
	new_task->pageListLength = 1;
	new_task->forkedPageListLength = 0;
	kmemset(new_task->pageList, 0, 4096);

	new_task->parent = (void*) 0;
	new_task->is_forked = 0;

	// add the task to the list
	new_task->next = first_task;
	first_task = new_task;

	return new_task;
}

static void mark_destroy_task(struct task_t *task) {
	// append this task to the to-be-destroyed queue
	task->next = destroyTaskQueue;
	destroyTaskQueue = task;

	/* kill every child */
	struct task_t* t = first_task;
	while(t != ((void*) 0)) {
		if(t->parent == task) {
			/* remove it from the list */
			if(t == first_task) {
				first_task = t->next;
			} else {
				// find the previous task
				struct task_t *previous_task = first_task;
				while(previous_task->next != t) {
					previous_task = previous_task->next;
				}

				// cut it out of the list
				previous_task->next = t->next;
			}

			/* we're doing a recursive call... */
			mark_destroy_task(t);

			t = first_task;
		}
		else
			t = t->next;
	}
}

static void destroy_tasks() {
	/* if the queue is empty, return */
	if(destroyTaskQueue == ((void*) 0))
		return;

	struct task_t *task = destroyTaskQueue;
	while(!(task == ((void*) 0))) {
		/* free the stacks */
		pmm_free(task->stack);
		pmm_free(task->user_stack);

		/* free the memory */
		for(int i = 0; i<((task->pageListLength * 4096) / sizeof(void*)); i++) {
			if(!(task->pageList[i] == ((void*) 0))) {
				pmm_free(task->pageList[i]);
			}
		}
		for(int i = 0; i<task->pageListLength;i++) {
			pmm_free(task->pageList + 4096*i);
		}

		// copy back and free the forked pages
		if(task->forkedPageListLength > 0) {
			for(int i = 0; i<((task->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; i++) {
				if(task->forkedPages[i].runLocation != (void*) 0) {
					kmemcpy(task->forkedPages[i].runLocation, task->forkedPages[i].storeLocation, 4096);
					pmm_free(task->forkedPages[i].storeLocation);
				}
			}
			for(int i = 0; i<task->forkedPageListLength;i++) {
				pmm_free(task->forkedPages + 4096*i);
			}
		}

		// free the structure itself
		struct task *next = task->next;
		pmm_free(task);
		task = next;
		destroyTaskQueue = next;
	}
}

int get_current_task_pid() {
	return current_task->pid;
}

static inline struct cpu_state *schedule_next_program(unsigned char destroyOldTasks) {
	// advance in the list
	if(current_task->next != (void*) 0)
		current_task = current_task->next;
	else
		current_task = first_task;

	// now clean up the old tasks
	if(destroyOldTasks == 1)
		destroy_tasks();

	// if the current program is forked, it expects the stack at the exact same position as the parent
	// so we'll swap it whilst the process is loaded and swap it back afterwards
	if(current_task->is_forked) {
		kmemswap(current_task->user_stack, current_task->parent->user_stack, 4096);

		/* swap the memory of the program */
		if(current_task->forkedPageListLength > 0) {
			for(int i = 0; i<((current_task->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; i++) {
				if(current_task->forkedPages[i].runLocation != (void*) 0) {
					kmemswap(current_task->forkedPages[i].runLocation, current_task->forkedPages[i].storeLocation, 4096);
				}
			}
		}
	}

	return current_task->state;
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

	// clean up the old task
	mark_destroy_task(old_task);


	// run the first task
	current_task = first_task;

	return schedule_next_program(0);
}

inline static void forkedPages_addAndCopyPage(struct task_t *task, void* runLocation) {
	// add the pages to the forked pages list
	unsigned char found = 0;
	while(found == 0) {
		for(int j = 0; j<((task->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; j++) {
			if(task->forkedPages[j].runLocation == (void*) 0) {
				// found a slot for a page
				found = 1;

				// write the page and copy it to the store location
				void* storeLocation = pmm_alloc();
				task->forkedPages[j].runLocation = runLocation;
				task->forkedPages[j].storeLocation = storeLocation;
				kmemcpy(storeLocation, runLocation, 4096);

				return;
			}
		}

		// if found == 0, the forked pages array is full
		// TODO move and copy
		if(found == 0) {
			// copy to a new array
			struct forked_task_page* newForkedPageList = pmm_alloc_range(task->forkedPageListLength+1);
			kmemcpy(newForkedPageList, task->forkedPages, task->forkedPageListLength);

			// free the old array
			for(int i = 0; i<task->forkedPageListLength; i++) {
				pmm_free(task->forkedPages + i * 4096);
			}

			task->forkedPages = newForkedPageList;
			task->forkedPageListLength++;
		}
	}
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

	// create the forked memory store area
	newTask->forkedPages = pmm_alloc();
	newTask->forkedPageListLength = 1;
	kmemset(newTask->forkedPages, 0, 4096);

	// copy the stack
	kmemcpy(newTask->user_stack, currTask->user_stack, 0x1000);

	// copy the memory over into the forked memory area
	for(int i = 0; i<((currTask->pageListLength * 4096) / sizeof(void*)) - 1; i++) {
		if(currTask->pageList[i] != ((void*) 0)) {
				forkedPages_addAndCopyPage(newTask, currTask->pageList[i]);
		}
	}

	// copy the forked memory
	if(currTask->forkedPageListLength > 0) {
		for(int i = 0; i<((currTask->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; i++) {
			if(currTask->forkedPages[i].runLocation != (void*) 0) {

				// since the task is currently loaded, we can copy from the run location
				forkedPages_addAndCopyPage(newTask, currTask->forkedPages[i].runLocation);
			}
		}
	}

	// tell the processes which one is parent and who's child
	currTask->state->eax = newTask->pid;
	newTask->state->eax = 0;

	return newTask;
}

unsigned char task_hasChildren(struct task_t* task) {
	struct task_t* t = first_task;
	while(t != ((void*) 0)) {
		if(t->parent == task) {
			return 1;
		}

		t = t->next;
	}

	return 0;
}

struct cpu_state *exec_current_task() {
	if(task_hasChildren(current_task))
		return (void*) 0;

	// if the task was forked, move everything back
	if(current_task->is_forked) {
		kmemcpy(current_task->parent->user_stack, current_task->user_stack, 0x1000);
		//for(int i = 0; current_task->pagelistCounter; i++)
		//	kmemcpy(current_task->parent->pagelist[i], current_task->pagelist[i], 0x1000);
	}

	// free the old memory

	for(int i = 0; i<((current_task->pageListLength * 4096) / sizeof(void*)); i++) {
		if(!(current_task->pageList[i] == ((void*) 0))) {
			pmm_free(current_task->pageList[i]);
		}
	}

	// copy back and free the forked pages
	if(current_task->forkedPageListLength > 0) {
		for(int i = 0; i<((current_task->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; i++) {
			if(current_task->forkedPages[i].runLocation != (void*) 0) {
				kmemcpy(current_task->forkedPages[i].runLocation, current_task->forkedPages[i].storeLocation, 4096);
				pmm_free(current_task->forkedPages[i].storeLocation);

				// remove the reference
				current_task->forkedPages[i].runLocation   = (void*) 0;
				current_task->forkedPages[i].storeLocation = (void*) 0;
			}
		}
	}

	// load the new program
	load_program(task2_start, task2_end, current_task);

	// load the next program
	return schedule_next_program(0);
}

struct cpu_state *schedule(struct cpu_state *current_state) {
	// initialize the first task switch
	if(current_task != (void*) 0) {
		current_task->state = current_state;
		if(current_task->is_forked) {
			kmemswap(current_task->user_stack, current_task->parent->user_stack, 4096);

			/* copy the memory of the program */
			if(current_task->forkedPageListLength > 0) {
				for(int i = 0; i<((current_task->forkedPageListLength * 4096) / sizeof(struct forked_task_page)) - 1; i++) {
					if(current_task->forkedPages[i].runLocation != (void*) 0) {
						kmemswap(current_task->forkedPages[i].runLocation, current_task->forkedPages[i].storeLocation, 4096);
					}
				}
			}
		}
	} else {
		current_task = first_task;
	}

	schedule_next_program(1);

	return current_task->state;
}

struct task_t *load_program(void* start, void* end, struct task_t* old_task) {
	struct elf_header *program_header = (struct elf_header*) start;
	struct elf_program_header_entry *program_header_entry = (void*) (start + program_header->program_header_tbl_offset);


	// we first have to go over the elf_structure to get the memory requirements
	void *targetEndAddr = (void*) 0;
	for(int i = 0; i<program_header->program_header_entry_count; i++, program_header_entry++) {

			// only load the LOAD segments
			if(program_header_entry->type != ELF_PH_TYPE_LOAD)
				continue;

			// the memsize must not be smaller than the file size
			if(program_header_entry->filesize > program_header_entry->memsize) {
				kputs("PANIC: [ELF] filesz > memsize");
				while(1);
			}

			void *targetSegEnd = program_header_entry->vaddr + program_header_entry->memsize;
			if(targetEndAddr < targetSegEnd)
				targetEndAddr = targetSegEnd;

	}

	// first we have to reserve a memory area for the elf image to be loaded to
	unsigned long length = targetEndAddr;
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

		// we essentially have to clean up the task and create a new one
		// in the same location
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

	// add the pages to the task
	// TODO does not check if the limit of pages has been reached
	for(int i = 0; i<pagesUsed; i++) {
		void* currentPage = target + i*4096;
		task->pageList[i] = currentPage;
	}

	// load the segments
	program_header_entry = (void*) (start + program_header->program_header_tbl_offset);
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
