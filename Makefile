CC = clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I./vm
LDFLAGS=
VMOBJS= vm/main.o vm/handler.o vm/insthandler.o vm/cpu.o vm/io.o
all: gdp8 assembler hex2bin
gdp8: $(VMOBJS)
	$(CC) $(LDFLAGS) $(VMOBJS) -o gdp8

assembler: asm/asm.o
	$(CC) $(LDFLAGS) asm/asm.o -o assembler

hex2bin: asm/hex2bin.o
	$(CC) $(LDFLAGS) asm/hex2bin.o -o hex2bin

clean:
	rm -rfv $(VMOBJS) gdp8 assembler hex2bin

syntax:
	$(CC) $(CFLAGS) -fsyntax-only */*.c */*.h
