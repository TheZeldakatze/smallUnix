.extern int_handler

.macro int_stub n
.global int_stub_\n
int_stub_\n:
	pushl $0
	pushl $\n
	jmp int_common
.endm

.macro int_stub_error n
.global int_stub_\n
int_stub_\n:
	pushl $\n
	jmp int_common
.endm

int_stub 0
int_stub 1
int_stub 2
int_stub 3
int_stub 4
int_stub 5
int_stub 6
int_stub 7
int_stub_error 8
int_stub 9
int_stub_error 10
int_stub_error 11
int_stub_error 12
int_stub_error 13
int_stub_error 14
int_stub 15
int_stub 16
int_stub_error 17
int_stub 18
int_stub 19
int_stub 20
int_stub 21
int_stub 22
int_stub 23
int_stub 24
int_stub 25
int_stub 26
int_stub 27
int_stub 28
int_stub 29
int_stub 30
int_stub 31
int_stub 32
int_stub 33
int_stub 34
int_stub 35
int_stub 36
int_stub 37
int_stub 38
int_stub 39
int_stub 40
int_stub 41
int_stub 42
int_stub 43
int_stub 44
int_stub 45
int_stub 46
int_stub 47
int_stub 48


int_common:
	// save the registers
	push %ebp
	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	// call the interrupt handler
	push %esp
	call int_handler
	mov %eax, %esp

	// load the registers
	pop %eax
	pop %ebx
	pop %ecx
	pop %edx
	pop %esi
	pop %edi
	pop %ebp

	add $8, %esp // remove the error code

	// return
	iret
