/*
 * task.h
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#ifndef INCLUDE_TASK_H_
#define INCLUDE_TASK_H_

#include <main.h>

/* A description of how tasks are stored and loaded
 *
 * when a task is loaded from an address, it will be loaded to the next free
 * address space. Because every program is a pie-executable, it can be run from
 * there whithout doing any kind of swapping.
 *
 * During execution, the programs can get new pages that are not connected with
 * their initial load area. This allows for programs to do dynamic memory
 * management without an mmu. However, we have to account for that and save
 * each used page
 *
 * If a program has to be swapped in (for example because it is fork()ed),
 * there exists the swaplist that stores each target page address together with
 * it's store address. These can be kmemswap()ed in when the program has to be
 * executed
 * */

struct task_t {
	int pid;
	unsigned char* stack;
	struct cpu_state *state;
	void **pagelist;
	long pagelistCounter;
	int parent_pid;
	int forkspace_pid; /** Forkspace describes a process that was fork()ed but has the same
		base code stuff (i.e. exec() or simelar was not run yet)
		we have to copy the new pages over in order to make fork() work. Now this is probably
		resource intensive */
	void **swaplist;
	int swaplistCounter;
	unsigned char *image_start;
	unsigned char *stack_store;
	struct task_t *next;
};

struct task_t *create_task(void* entry);
void fork_current_task(struct cpu_state *cpu);
struct cpu_state *schedule(struct cpu_state *current_state);
void init_multitasking(struct multiboot *mb);

extern struct cpu_state *kill_current_task();
extern int get_current_task_pid();

#endif /* INCLUDE_TASK_H_ */
