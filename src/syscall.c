/*
 * syscall.c
 *
 *  Created on: 19.11.2022
 *      Author: maite
 */

#include <syscall.h>
#include <task.h>

struct cpu_state* handle_syscall(struct cpu_state* cpu) {
	switch(cpu->eax) {
		case SYSCALL_EXIT: { // exit
			kputs("A task exited!");
			return kill_current_task();
		}
		case SYSCALL_FORK: {
			kputs("fork(): not implemented");
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
	}

	return cpu;
}
