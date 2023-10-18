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
	unsigned char* user_stack;
	struct cpu_state *state;
	void **pagelist;
	long pagelistCounter;
	struct task_t *parent;
	unsigned char is_forked;
	struct task_t *next;
};

struct task_t *create_task(void* entry);
struct cpu_state *schedule(struct cpu_state *current_state);
void init_multitasking(struct multiboot *mb);

extern struct cpu_state *kill_current_task();
extern int get_current_task_pid();
extern struct task_t* fork_current_task(struct cpu_state* current_state);
extern struct cpu_state *exec_current_task();

#endif /* INCLUDE_TASK_H_ */
