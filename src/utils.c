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
