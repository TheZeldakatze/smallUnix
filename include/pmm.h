/*
 * pmm.h
 *
 *  Created on: 20.10.2022
 *      Author: maite
 */

#ifndef INCLUDE_PMM_H_
#define INCLUDE_PMM_H_

#include <main.h>

extern const void kernel_start;
extern const void kernel_end;

extern void pmm_install(struct multiboot *mb);
extern void* pmm_alloc();
extern void* pmm_alloc_range(int pages);
extern void pmm_free(void* addr);
extern void pmm_clear_page(void* addr);

#endif /* INCLUDE_PMM_H_ */
