#!/bin/sh
qemu-system-i386 -d guest_errors -kernel bin/kernel.bin -initrd test/mbTest1/mbTest1.bin
#-initrd test/vmm_test/vmm_test.bin,test/vmm_test/vmm_test.bin
