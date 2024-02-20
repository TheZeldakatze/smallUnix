/*
 * syscall.c
 *
 *  Created on: 19.11.2022
 *      Author: maite
 */

#include <syscall.h>
#include <task.h>

extern struct task_t *current_task;

struct cpu_state* handle_syscall(struct cpu_state* cpu) {
	switch(cpu->eax) {
		case SYSCALL_EXIT: { // exit
			char buf[100];
			kitoa(current_task->pid, &buf);
			kputs(buf);
			kputs(" has finished!");
			if(current_task->pid == 1) {
				kputs("No remaining Task!");
				task_debug_printTaskList();
			}

			return kill_current_task(cpu);
		}
		case SYSCALL_FORK: {
			cpu->eax = fork_current_task(cpu)->pid;
			break;
		}
		case SYSCALL_READ: {
			// get the buffer and size
			char *buf = (char*) cpu->ebx;
			int size  = (int)   cpu->ecx;

			// TODO do proper
			// write a test string to the program
			char *test = "A simple message from read() \n";
			kmemcpy(buf, test, 30);

			// return the read size in eax
			cpu->eax = 30;

			break;
		}
		case SYSCALL_WRITE: {
			// get the buffer and size
			char *buf = (char*) cpu->ebx;
			int size  = (int)   cpu->ecx;

			// TODO do proper
			// write the written stuff to the console
			int writtenbytes = 0;
			for(int i = 0; i<size; i++) {
				kputc(buf[i]);
				writtenbytes++;
			}

			// return the written size in eax
			cpu->eax = writtenbytes;

			break;
		}
		case SYSCALL_OPEN: {

			break;
		}
		case SYSCALL_CLOSE: {

			break;
		}
		case SYSCALL_EXEC: {
			struct cpu *ret = exec_current_task();
			if(ret == ((void*) 0))
				cpu->eax = -1;
			else {
				cpu = ret;
			}
			break;
		}
		case SYSCALL_WAIT: {
			current_task->waitpid_num = cpu->ebx;
			current_task->run_state   = TASK_RUN_STATE_WAITPID;
			cpu = schedule(cpu);
			break;
		}
	}

	return cpu;
}
