SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC := $(wildcard $(SRC_DIR)/*.[cS])
OBJ := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%.o, $(SRC))

CC = crossCompiler/bin/i686-elf-gcc
LD = crossCompiler/bin/i686-elf-ld

ASFLAGS =
CFLAGS = -Wall -g -fno-stack-protector -I./include
LDFLAGS = -melf_i386 -T link.ld

all: bin/kernel.bin

bin/kernel.bin: $(OBJ) | $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.S.o: $(SRC_DIR)/%.S | $(OBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm $(OBJS)

.PHONY: all clean