/*
 * task.h
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#ifndef INCLUDE_TASK_H_
#define INCLUDE_TASK_H_

#include <main.h>

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
	struct task_t *next;
};

struct task_t *create_task(void* entry);
struct cpu_state *schedule(struct cpu_state *current_state);
void init_multitasking(struct multiboot *mb);

extern struct cpu_state *kill_current_task();
extern int get_current_task_pid();

#endif /* INCLUDE_TASK_H_ */
