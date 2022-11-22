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
extern int fork();

int acoolvariable = 2;
char buf[1];

int main() {
	write(0, "HELLO WORLD! now with write ... \n", 33);
	int num = write(1, "Reading what read() will return for now:\n", 41);
	//for(int j = 0; j<100000000; j++);
	char buf[30];
	int count = read(1, &buf, 30);
	write(1, buf, count);

	//asm volatile("int $48" : : "a" (SYSCALL_EXIT), "b" (0));
	write(1, "Testing fork()\n", 15);
	int ret = fork();
	if(ret == 0) {
		write(1, "parent: \n", 9);

	}
	else
		write(1, "child\n",6);

	while(acoolvariable<5) {
			buf[0] = '0' + acoolvariable++;
			write(1, &buf, 1);
	}
	while(1);
	exit(0);
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

int fork() {
	// save the registers
	asm volatile("push %eax");
	asm volatile("push %ebx");

	asm volatile("int $48" : : "a" (SYSCALL_FORK));

	// get the return value
	int ret = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));

	// restore the registers
	asm volatile("pop %eax");
	asm volatile("pop %ebx");
	return ret;
}
