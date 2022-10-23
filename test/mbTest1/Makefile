SRCS = $(shell find -name '*.[cS]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = ../../crossCompiler/bin/i686-elf-gcc
LD = ../../crossCompiler/bin/i686-elf-ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector --no-pic -nostdinc -c -I include
LDFLAGS = -melf_i386 -Tlink.ld

mbTest1.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean