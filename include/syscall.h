/*
 * syscall.h
 *
 *  Created on: 19.11.2022
 *      Author: maite
 */

#ifndef INCLUDE_SYSCALL_H_
#define INCLUDE_SYSCALL_H_

#define SYSCALL_EXIT 0
#define SYSCALL_FORK 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_EXEC 6
#define SYSCALL_WAIT 7

struct cpu_state* handle_syscall(struct cpu_state* cpu);

#endif /* INCLUDE_SYSCALL_H_ */
