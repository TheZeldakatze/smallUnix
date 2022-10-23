/*
 * task.c
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#include <task.h>
#include <pmm.h>

const int PAGELIST_SITE_ENTRIES = PAGE_SIZE / sizeof(void*) - 1;
const int PAGELIST_SITE_NEXT    = PAGE_SIZE / sizeof(void*);

static void task_a() {
	while(1) {
		asm("int $0x30" : : "a" (0), "b" ('0'));
		for(int i = 0; i<10000000;i++);
	}
}

static void task_b() {
	while(1) {
		kputs("1");
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

	void* start = startPage;
	for(int i = 0; i<pagelistNr; i++) {
		pagelistSite[pagelistLine] = start;
		start += PAGE_SIZE;
		task->pagelistCounter++;
		pagelistLine++;
		if(pagelistLine >= PAGELIST_SITE_NEXT) {
			if(pagelistSite[PAGELIST_SITE_NEXT] == ((void*) 0))
				pagelistSite[PAGELIST_SITE_NEXT] = pmm_alloc();
			pagelistSite = pagelistSite[PAGELIST_SITE_NEXT];
			pagelistNr++;
			pagelistLine = 0;
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
	new_task->pid   = NEXT_PID++;
	new_task->pagelist = pmm_alloc();
	//new_task->pagelist[PAGELIST_SITE_NEXT] = (void*) pmm_alloc();
	new_task->pagelistCounter = 0;

	// add the task to the list
	new_task->next = first_task;
	first_task = new_task;

	// add created pages to the page list
	//task_addPageToPagelist(new_task, (void*) stack);

	return new_task;
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

struct cpu_state *schedule(struct cpu_state *current_state) {
	// initialize the first task switch
	if(current_task != (void*) 0) {
		current_task->state = current_state;
	} else {
		current_task = first_task;
	}

	// advance in the list
	if(current_task->next != (void*) 0)
		current_task = current_task->next;
	else
		current_task = first_task;

	return current_task->state;
}

struct task_t *load_program(void* start, void* end) {
	unsigned long length = end - start;
	int pagesUsed = length / 4096 + (length % 4096 != 0);
	unsigned char *target = 0x200000; //pmm_alloc_range(pagesUsed);

	char* source = (char*) start;

	for(int i = 0; i<length; i++) {
		target[i] = source[i];
	}

	struct task_t* task = create_task(target);
	//task_addPageToPagelist_range(task, target, pagesUsed);
	return task;
}

void init_multitasking(struct multiboot *mb) {
	create_task((void*) task_a);
	//create_task((void*) task_b);
	struct multiboot_module* mod = mb->mods_addr;
		load_program((void*) mod[0].start, (void*) mod[0].end);
		//load_program((void*) mod[0].start, (void*) mod[0].end);
		//load_program((void*) mod[0].start, (void*) mod[0].end);
}
