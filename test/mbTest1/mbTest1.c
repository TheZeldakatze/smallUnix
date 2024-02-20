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
#define SYSCALL_EXEC 6
#define SYSCALL_WAIT 7

extern void exit(int code);
extern int write(int fd, const void *buf, int count);
extern int read(int fd, void *buf, int count);
extern int exec(char* path);

int i = 0, j = 0;

int main() {
	write(0, "HELLO WORLD! now with write ... \n", 33);
	int num = write(1, "Reading what read() will return for now:\n", 41);

	char buf[30];
	int count = read(1, &buf, 30);
	write(1, buf, count);

	i = 0;
	j = 0;
	putc('0' + i++);
	putc('0' + i++);
	putc('0' + i++);
	putc('0' + i++);

	int nchild = 0;

	write(0, "forking...\n", 11);
	nchild += fork() != 0;
	write(0, "forked...\n", 10);
	if(!nchild) {
		write(0, "I'm a child!\n", 13);
		putc('0' + i++);
		putc('0' + j++);



		//
		nchild += fork() != 0;
		if(!nchild) {
			putc('0' + fork());
			nchild += fork() != 0;
			nchild += fork() != 0;
			nchild += fork() != 0;
			exec("");
			write(1, "Hello World, again!\n", 21);
			//for(int i = 0; i<10000000000; i++);
		}
		else {
			write(1, "World Hello, again!\n", 21);
		}
	} else {
		write(0, "I'm a parent!\n", 14);
		putc('0' + i++);
		putc('0' + j++);

		//while(1) {
		//	write(0, "Hello", 5);
		//	for(int i = 0; i<1000000;i++);
		//}

	}

	puts("Waiting for my ");
	putc('0' + nchild);
	puts("children...\n");
	for(int i = 0; i<nchild; i++) {
		int n = 0;
		wait(&n);
	}

	//asm volatile("int $48" : : "a" (SYSCALL_EXIT), "b" (0));
	exit(0);
	write(0, "HELLO WORLD! now with write ... \n", 33);
}

int strlen(char *s) {
	int n = 0;
	while(s[n]) n++;
	return n;
}

void puts(char *s) {
	int l = strlen(s);
	write(0, s, l);
}

void putc(char* c) {
	char s[1];
	s[0] = c;
	write(1, s, 1);
}

int fork() {

	// save the registers
	asm volatile("push %eax");

	// do the call
	asm volatile("int $48" : : "a" (SYSCALL_FORK));

	// get the return value
	int ret = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));

	// restore the registers
	asm volatile("pop %eax");

	return ret;
}

int exec(char* path) {
	// save the registers
	asm volatile("push %eax");
	asm volatile("push %ebx");

	asm volatile("int $48" : : "a" (SYSCALL_EXEC), "b" (path));

	// get the return value
	int ret = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));

	// restore the registers
	asm volatile("pop %ebx");
	asm volatile("pop %eax");

	return ret;
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
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");

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
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");

	return ret;
}

int waitpid(int pid, int *wstatus, int options) {
	// save the registers
	asm volatile("push %eax");
	asm volatile("push %ebx");
	asm volatile("push %ecx");

	asm volatile("int $48" : : "a" (SYSCALL_WAIT), "b" (pid), "c" (options));

	// get the return value
	int ret = 0, status = 0;
	asm volatile("movl %%eax, %0" : "=r"(ret));
	asm volatile("movl %%ebx, %0" : "=r"(status));
	*wstatus = status;

	// restore the registers
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");

	return ret;
}

int wait(int *wstatus) {
	return waitpid(-1, wstatus, 0);
}
