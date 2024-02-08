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
	a_end = a + n;

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
	}

	/* process the individual digit */
	while(input != 0) {
		int remainder = input % 10;
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

kputn(int n) {
	char *buf[30];
	kitoa(n, buf);
	kputs(buf);
}
