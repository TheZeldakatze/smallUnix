/*
 * utils.h
 *
 *  Created on: 01.11.2022
 *      Author: maite
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

extern void kmemcpy(void *dest, void *src, int n);
extern void kmemset(void *dest, unsigned char val, int n);
extern void kmemswap(void *dest, void *src, int n);
extern void kitoa(int num, char* str);
extern void kputn(int n);
extern inline void outb(unsigned short port, unsigned char data);
extern inline unsigned char inb(unsigned short port);

#endif /* INCLUDE_UTILS_H_ */
