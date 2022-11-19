/*
 * mbTest1.c
 *
 *  Created on: 20.10.2022
 *      Author: maite
 */

#define SYSCALL_EXIT 0
#define SYSCALL_FORK 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5

extern void exit(int code);
extern int write(int fd, const void *buf, int count);
extern int read(int fd, void *buf, int count);

int i = 0;

int main() {
	write(0, "HELLO WORLD! now with write ... \n", 33);
	int num = write(1, "Reading what read() will return for now:\n", 41);

	char buf[30];
	int count = read(1, &buf, 30);
	write(1, buf, count);

	//asm volatile("int $48" : : "a" (SYSCALL_EXIT), "b" (0));
	exit(0);
	write(0, "HELLO WORLD! now with write ... \n", 33);
}

int write(int fd, const void *buf, int count) {

	// save the registers
	asm volatile("push %eax");
	asm volatile("push %ebx");
	asm volatile("push %ecx");

	// do the call
	asm volatile("int $48" : : "a" (SYSCALL_WRITE), "b" (buf), "c" (count));

	// get the return value
	int ret = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));

	// restore the registers
	asm volatile("pop %eax");
	asm volatile("pop %ebx");
	asm volatile("pop %ecx");

	return ret;
}

int read(int fd, void *buf, int count) {
	// save the registers
	asm volatile("push %eax");
	asm volatile("push %ebx");
	asm volatile("push %ecx");

	asm volatile("int $48" : : "a" (SYSCALL_READ), "b" (buf), "c" (count));

	// get the return value
	int ret = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));

	// restore the registers
	asm volatile("pop %eax");
	asm volatile("pop %ebx");
	asm volatile("pop %ecx");

	return ret;
}
