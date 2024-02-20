/*
 * task.h
 *
 *  Created on: 19.10.2022
 *      Author: maite
 */

#ifndef INCLUDE_TASK_H_
#define INCLUDE_TASK_H_

#define TASK_RUN_STATE_RUNNING  0
#define TASK_RUN_STATE_SLEEPING 1
#define TASK_RUN_STATE_ZOMBIE   2
#define TASK_RUN_STATE_WAITEXEC 3
#define TASK_RUN_STATE_IDLETASK 4
#define TASK_RUN_STATE_WAITPID  5
#define TASK_RUN_STATE_DECONSTRUCT 6

#include <main.h>

#define TASK_PAGE_LIST_NODE_SIZE ( (4096-(sizeof((void*)))) / sizeof(void*)  - 1)

struct forked_task_page {
	void* runLocation;
	void* storeLocation;
};

struct task_t {
	int pid, run_state;
	unsigned char* stack;
	unsigned char* user_stack, *user_stack_run_address;
	struct cpu_state *state;

	void** pageList;
	unsigned char pageListLength; /* in 4kb-Pages */
	struct forked_task_page* forkedPages;
	unsigned char forkedPageListLength;

	unsigned int waitpid_num;

	struct task_t *parent;
	unsigned char is_forked;
	struct task_t *next;
};

struct task_t *create_task(void* entry);
struct cpu_state *schedule(struct cpu_state *current_state);
void init_multitasking(struct multiboot *mb);

extern struct cpu_state *kill_current_task(struct cpu_state *c);
extern int get_current_task_pid();
extern struct task_t* fork_current_task(struct cpu_state* current_state);
extern struct cpu_state *exec_current_task();

#endif /* INCLUDE_TASK_H_ */
