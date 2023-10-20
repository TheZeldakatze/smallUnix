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

#endif /* INCLUDE_UTILS_H_ */
