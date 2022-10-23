.global _start
.extern kmain;

.section multiboot
#define MB_MAGIC 0x1badb002
#define MB_FLAGS 0x0
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)

.align 4
.int    MB_MAGIC
.int    MB_FLAGS
.int    MB_CHECKSUM

.section .text


.extern kmain

#define MB_MAGIC 0x1badb002
#define MB_FLAGS 0x0
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)
.align 4
.int    MB_MAGIC
.int    MB_FLAGS
.int    MB_CHECKSUM

_start:
  mov $kernel_stack, %esp
  push %ebx
  call kmain
die:
  cli
  hlt
  jmp die

.section .bss
.space 8192
kernel_stack:
