/*
 * mbTest1.c
 *
 *  Created on: 20.10.2022
 *      Author: maite
 */

extern void exit(int code);

int i = 0;

int main() {
	pputs("This is the second test program!");

	for (; i < 5; i++) {
	        pputc('0' + i);
	}

	asm volatile("int $48" : : "a" (1), "b" (0));
	//exit(0);
}

void pputs(char *c) {
	int i = 0;
	while(c[i] != '\0') {
		pputc(c[i]);
		i++;
	}
}

void pputc(char c) {

	// save eax and ebx
	asm volatile("push %eax");
	asm volatile("push %ebx");

	// do the syscall
	asm volatile("int $48" : : "a" (0), "b" (c));

	// load eax and ebx
	asm volatile("pop %ebx");
	asm volatile("pop %eax");

	for(int i = 0; i<10000000;i++);
}
