# RISCV environment variable must be set

CC=$(RISCV)/bin/riscv64-unknown-elf-gcc
OBJCOPY=$(RISCV)/bin/riscv64-unknown-elf-objcopy
CFLAGS=-march=rv32imac -mabi=ilp32 -O2 -std=gnu11 -Wall -I. -nostartfiles -fno-common -fno-builtin
LFLAGS=-static -nostdlib -T ld.scr

TARGET=kozos

$(TARGET): startup.s intr.S main.c interrupt.c serial.c lib.c kozos.c syscall.c memory.c \
		consdrv.c command.c
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

.PHONY: clean
clean::
	rm -rf $(TARGET) *~
