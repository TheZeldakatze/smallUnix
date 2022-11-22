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

const int PAGELIST_SITE_ENTRIES = PAGE_SIZE / sizeof(void*) - 2 * sizeof(void*);
const int PAGELIST_SITE_NEXT    = PAGE_SIZE / sizeof(void*) - 1 * sizeof(void*);

const int SWAPLIST_SITE_ENTRIES = PAGE_SIZE / (sizeof(void*) * 2) - 3 * sizeof(void*);
const int SWAPLIST_SITE_NEXT    = PAGE_SIZE / sizeof(void*) - 1 * sizeof(void*);

static void task_idle() {
	while(1) {
		//asm("int $0x30" : : "a" (0), "b" ('0'));
		for(int i = 0; i<10000000;i++);
	}
}

static int NEXT_PID = 0;

struct task_t *first_task, *current_task;

void task_addPageToPagelist(struct task_t *task, void *page) {
	int pagelistNr   = task->pagelistCounter / PAGELIST_SITE_ENTRIES;
	int pagelistLine = task->pagelistCounter % PAGELIST_SITE_ENTRIES;
	void **pagelistSite = task->pagelist;

	for(int i = 0; i<pagelistNr; i++) {
		if(pagelistSite[PAGELIST_SITE_NEXT] == ((void*) 0)) {
			pagelistSite[PAGELIST_SITE_NEXT] = pmm_alloc();
		}

		pagelistSite = pagelistSite[PAGELIST_SITE_NEXT];
	}
	pagelistSite[pagelistLine] = page;

	task->pagelistCounter++;
}

void task_addPageToPagelist_range(struct task_t *task, void *startPage, int count) {
	int pagelistNr   = task->pagelistCounter / PAGELIST_SITE_ENTRIES;
	int pagelistLine = task->pagelistCounter % PAGELIST_SITE_ENTRIES;
	void **pagelistSite = task->pagelist;

	for(int i = 0; i<pagelistNr; i++) {
		if(pagelistSite[PAGELIST_SITE_NEXT] == ((void*) 0)) {
			pagelistSite[PAGELIST_SITE_NEXT] = pmm_alloc();
		}

		pagelistSite = pagelistSite[PAGELIST_SITE_NEXT];
	}

	unsigned char* start = startPage;
	for(int i = 0; i<count; i++) {
		pagelistSite[pagelistLine] = start;
		start += PAGE_SIZE;
		task->pagelistCounter++;
		pagelistLine++;
		if(pagelistLine >= PAGELIST_SITE_ENTRIES) {
			if(pagelistSite[PAGELIST_SITE_NEXT] == ((void*) 0))
				pagelistSite[PAGELIST_SITE_NEXT] = pmm_alloc();
			pagelistSite = pagelistSite[PAGELIST_SITE_NEXT];
			pagelistNr++;
			pagelistLine = 0;
		}
	}
}

void task_copyPagelist(struct task_t* dest_task, struct task_t* src_task) {
	int pagelistNr   = src_task->pagelistCounter / PAGELIST_SITE_ENTRIES;
	int pagelistLine = src_task->pagelistCounter % PAGELIST_SITE_ENTRIES;
	void **pagelistSrc  =  src_task->pagelist;
	void **pagelistDest = dest_task->pagelist;

	// copy the first page
	int lines = (pagelistNr > 0 ? PAGELIST_SITE_ENTRIES : pagelistLine);
	kmemcpy(pagelistDest, pagelistSrc, lines * sizeof(void*));
	dest_task->pagelistCounter += lines;

	for(int i = 1; i<pagelistNr + 1; i++) {
		// go to this page
		pagelistSrc[PAGELIST_SITE_NEXT] = pmm_alloc();
		pagelistSrc  = pagelistSrc [PAGELIST_SITE_NEXT];
		pagelistDest = pagelistDest[PAGELIST_SITE_NEXT];

		// copy it
		int lines = (pagelistNr > i ? PAGELIST_SITE_ENTRIES : pagelistLine);
		kmemcpy(pagelistDest, pagelistSrc, lines * sizeof(void*));
		dest_task->pagelistCounter += lines;
	}
}

void task_addPageToSwaplist_range(struct task_t *task, void *startPage, int count) {

	// calculate the start offsets
	int swaplistNr   = task->swaplistCounter / SWAPLIST_SITE_ENTRIES;
	int swaplistLine = task->swaplistCounter % SWAPLIST_SITE_ENTRIES;
	void **swaplistSite = task->swaplist;

	// move to the start site
	for(int i = 0; i<swaplistNr; i++) {
		if(swaplistSite[SWAPLIST_SITE_NEXT] == (void*) 0) {
			swaplistSite[SWAPLIST_SITE_NEXT] = pmm_alloc();
		}

		swaplistSite = swaplistSite[SWAPLIST_SITE_NEXT];
	}

	// put in every site entry
	unsigned char* start = startPage;
	for(int i = 0; i<count; i++) {

		// add the entry and find a free page
		swaplistSite[swaplistLine*2] = start;
		swaplistSite[swaplistLine*2+1] = pmm_alloc();

		// copy the page
		kmemcpy(swaplistSite[swaplistLine*2+1], swaplistSite[swaplistLine*2], PAGE_SIZE);

		// go to the next line
		start += PAGE_SIZE;
		task->swaplistCounter++;
		swaplistLine++;

		// do we have to go to the next page?
		if(swaplistLine >= SWAPLIST_SITE_ENTRIES) {

			// if necessary, allocate a new page
			if(swaplistSite[SWAPLIST_SITE_NEXT] == (void*) 0)
				swaplistSite[SWAPLIST_SITE_NEXT] = pmm_alloc();
			swaplistSite = swaplistSite[SWAPLIST_SITE_NEXT];
			swaplistNr++;
			swaplistLine = 0;
		}
	}
}

void task_swap(struct task_t *task) {
	// get all the length values for the list
	int swaplistNr   = task->swaplistCounter / SWAPLIST_SITE_ENTRIES;
	int swaplistLine = task->swaplistCounter % SWAPLIST_SITE_ENTRIES;

	// iterate through the array
	void **currentSwaplistPage = task->swaplist;
	for(int i = 0; i<swaplistNr+1; i++) {
		int lineCount = (i < swaplistNr ? SWAPLIST_SITE_ENTRIES : swaplistLine);
		for(int j = 0; j<lineCount; j++) {

			// do the swapping
			kmemswap(currentSwaplistPage[j*2+1], currentSwaplistPage[j*2], PAGE_SIZE);
		}

		// advance to the next page
		currentSwaplistPage = currentSwaplistPage[SWAPLIST_SITE_NEXT];
	}
}

void task_createSwapspace(struct task_t *task) {
	task->swaplist = pmm_alloc();

	// iterate through the pagelist
	int line = 0;
	void **page = task->pagelist;
	for(int i = 0; i<task->pagelistCounter;i++) {
		// add to the swaplist (yes, I know I'm lazy) (TODO optimize)
		task_addPageToSwaplist_range(task, page[line], 1);

		// go to the next line or page
		line++;
		if(line >= PAGELIST_SITE_ENTRIES) {
			page = task->pagelist[PAGELIST_SITE_NEXT];
			line = 0;
		}
	}
}

struct task_t *create_task(void* entry) {
	struct task_t *new_task = pmm_alloc();
	unsigned char* stack    = pmm_alloc();

	struct cpu_state new_state = {
		.eax = 0,
		.ebx = 0,
		.ecx = 0,
		.edx = 0,
		.esi = 0,
		.edi = 0,
		.ebp = 0,
		.eip = (unsigned long) entry,
		.cs = 0x08,
		.eflags = 0x202,
	};

	// put the cpu_state on the process stack.
	struct cpu_state* state = (void*) (stack + 4096 - sizeof(new_state));
	*state = new_state;

	new_task->state = state;
	new_task->stack = stack;
	new_task->stack_store = new_task->stack; // for non-forked programs, the addresses are the same
	new_task->pid   = NEXT_PID++;
	new_task->pagelist = pmm_alloc();
	new_task->pagelistCounter = 0;

	// add the task to the list
	new_task->next = first_task;
	first_task = new_task;

	return new_task;
}

static void deconstruct_task(struct task_t *task) {
	// TODO
}

int get_current_task_pid() {
	return current_task->pid;
}

void fork_current_task(struct cpu_state *cpu) {
	struct task_t *task = create_task((void*) 0);
	task->forkspace_pid = current_task->pid;
	task->stack = current_task->stack;
	task->state = cpu;
	task->state->eax = task->pid;

	task->stack_store = pmm_alloc();

	task_copyPagelist(task, current_task);
	task_createSwapspace(task);

	kmemcpy((char*) task->stack_store, current_task->stack, PAGE_SIZE);
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

struct cpu_state *schedule(struct cpu_state *current_state) {
	// initialize the first task switch
	if(current_task != (void*) 0) {
		current_task->state = current_state;
	} else {
		current_task = first_task;
	}

	if(current_task->forkspace_pid > 0) {
		task_swap(current_task);
		kmemswap((char*) current_task->stack_store, (char*) current_task->stack, PAGE_SIZE);
	}

	// advance in the list
	if(current_task->next != (void*) 0)
		current_task = current_task->next;
	else
		current_task = first_task;

	if(current_task->forkspace_pid > 0) {
		task_swap(current_task);
		kmemswap((char*) current_task->stack_store, current_task->stack, PAGE_SIZE);
	}

	return current_task->state;
}

struct task_t *load_program(void* start, void* end) {
	struct elf_header *program_header = (struct elf_header*) start;

	// check for a valid elf magic
	if(program_header->magic != ELF_MAGIC) {
		kputs("Error loading program! Invalid elf magic");
		return (void*) 0;
	}

	// check how much memory is required
	struct elf_program_header_entry *program_header_entry = (void*) (start + program_header->program_header_tbl_offset);
	unsigned long length;
	unsigned char *lowestLoad = (void*) 0, *highestAddr = (void*) 0;
	for(int i = 0; i<program_header->program_header_entry_count; i++, program_header_entry++) {
		// only load the LOAD segments
		if(program_header_entry->type != ELF_PH_TYPE_LOAD)
			continue;

		// the memsize must not be smaller than the file size
		if(program_header_entry->filesize > program_header_entry->memsize) {
			kputs("PANIC: [ELF] filesz > memsize");
			while(1);
		}

		// get the vaddr
		unsigned char* vaddr = (unsigned char*) program_header_entry->vaddr;
		unsigned char* vaddrEnd = program_header_entry->memsize + vaddr;

		// compare the segment's size
		if(lowestLoad == (void*) 0 || lowestLoad >  vaddr)
			lowestLoad = vaddr;
		if(highestAddr == (void*) 0 || highestAddr < vaddrEnd)
			highestAddr = vaddrEnd;
	}
	length = (unsigned char*) highestAddr - (unsigned char*) lowestLoad;

	// first we have to reserve a memory area for the elf image to be loaded to
	int pagesUsed = length / 4096 + (length % 4096 != 0);
	unsigned char *target = pmm_alloc_range(pagesUsed);

	// load the segments
	program_header_entry = (void*) (start + program_header->program_header_tbl_offset);
	for(int i = 0; i<program_header->program_header_entry_count; i++, program_header_entry++) {

		// only load the LOAD segments
		if(program_header_entry->type != ELF_PH_TYPE_LOAD)
			continue;
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

	// now create the task itself
	struct task_t* task = create_task((void*) (program_header->entry_posititon + target));

	// add all pages used by the program to the pagelist
	task_addPageToPagelist_range(task, target, pagesUsed);

	return task;
}

void init_multitasking(struct multiboot *mb) {
	create_task((void*) task_idle);
	//create_task((void*) task_b);
	struct multiboot_module* mod = mb->mods_addr;
		load_program((void*) mod[0].start, (void*) mod[0].end);
		//load_program((void*) mod[1].start, (void*) mod[1].end);
		//load_program((void*) mod[0].start, (void*) mod[0].end);
}
