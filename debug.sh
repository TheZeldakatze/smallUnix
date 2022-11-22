#!/bin/bash
qemu-system-i386 -d guest_errors -kernel bin/kernel.bin -S -gdb tcp::3234 -initrd test/mbTest1/mbTest1.bin &
gdbgui -g "gdb -ex 'target remote tcp::3234'" bin/kernel.bin &
fg %qemu-system-i386