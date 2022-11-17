/*
 * start.S
 *
 *  Created on: 23.10.2022
 *      Author: maite
 */


.global _start
.extern main

//.section start
_start:
	call main

exit:
	push %ebx
	mov %eax, 1

	int $48

	// print A
	mov %eax, 0
	mov %ebx, 'A'
	int $48

	jmp exit;

.section .text
