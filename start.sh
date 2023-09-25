#!/bin/sh
qemu-system-i386 -d guest_errors -kernel bin/kernel.bin -initrd test/mbTest1/mbTest1.bin,test/mbTest2/mbTest2.bin
