/*
 * utils.c
 *
 *  Created on: 01.11.2022
 *      Author: maite
 */

#include <utils.h>

void kmemcpy(void *dest, void *src, int n) {
	// TODO optimize. for forked processes, this function has to be fast to swap pages
	char *csrc, *cdst;
	csrc = (char*) src;
	cdst = (char*) dest;
	for(int i = 0; i<n; i++)
		cdst[i] = csrc[i];
}

void kmemset(void *dest, unsigned char val, int n) {
	char *cdst;
	cdst = (char*) dest;
	for(int i=0; i<n; i++)
		cdst[i] = val;
}

void kmemswap(void *dest, void *src, int n) {
	char *a, *b, *a_end;
	a = (char*)dest;
	b = (char*)src;
	a_end = (char*) (a + n);



	while(a < a_end) {
		char tmp = *a;
		*a = *b;
		*b = tmp;
		a++;
		b++;
	}
}

/*
 * based on https://www.geeksforgeeks.org/implement-itoa/
 * */
void kitoa(int num, char* str) {
	unsigned char negative = 0;
	int stringI = 0;
	int input = num;

	/* return 0 if the number is 0 */
	if(num == 0) {
		str[0] = '0';
		str[1] = '\0';
	}

	/* continue with a positive number and append the sign later */
	if(num<0) {
		num = -num;
		negative = 1;
	} else if(num == 0)
		str[stringI++] = '0'; /* if it is 0, use the sign */

	/* process the individual digit */
	while(input != 0) {
		int remainder = input % 10;
		if(remainder < 0)
			remainder = -remainder;
		str[stringI++] = '0' + remainder;
		input = input / 10;
	}

	if(negative == 1)
		str[stringI++] = '-';

	str[stringI] = '\0';

	// reverse the string
	int s = 0, e = stringI-1;
	while(s<e) {
		char t = str[s];
		str[s] = str[e];
		str[e] = t;
		e--;
		s++;
	}
}

void kputn(int n) {
	char *buf[30];
	kitoa(n, buf);
	kputs(buf);
}

inline void outb(unsigned short port, unsigned char data) {
	asm volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

inline unsigned char inb(unsigned short port) {
	char ret;
	asm volatile ("inb %1, %0" : "=a"(ret) : "Nd" (port));
	return ret;
}
