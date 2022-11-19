/*
 * start.S
 *
 *  Created on: 23.10.2022
 *      Author: maite
 */


.global _start
.global exit
.extern main

_start:
	call main

exit:
	pop %ebx
	mov $0, %eax
	int $48

	jmp exit

.section .text
