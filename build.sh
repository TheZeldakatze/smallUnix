#!/bin/sh
make all
make -C test/mbTest1/ clean || true
make -C test/mbTest1/ mbTest1.bin

make -C test/mbTest2/ clean || true
make -C test/mbTest2/ mbTest2.bin
