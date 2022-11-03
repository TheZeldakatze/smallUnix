SRCS = $(shell find -name '*.[cS]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = ../../crossCompiler/bin/i686-elf-gcc
LD = ../../crossCompiler/bin/i686-elf-ld

ASFLAGS = -m32 -pie -fpic
CFLAGS = -m32 -Wall -g -fno-stack-protector -c -I include -pie -fpic
LDFLAGS = -melf_i386 -Tlink.ld -pie

mbTest1.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS) mbTest1.bin

.PHONY: clean