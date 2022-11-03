#!/bin/sh
make all
make -C test/mbTest1/ clean || true
make -C test/mbTest1/ mbTest1.bin
