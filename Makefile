CC = clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I./vm
LDFLAGS=
VMOBJS= vm/main.o vm/inst.o vm/insthandler.o vm/cpu.o
all: $(VMOBJS) gdp8 assembler
gdp8:
	$(CC) $(CFLAGS) $(LDFLAGS) $(VMOBJS) -o gdp8

assembler:
	$(CC) $(CFLAGS) $(LDFLAGS) asm/asm.c -o assembler

clean:
	rm -rfv $(VMOBJS) gdp8 assembler

syntax:
	$(CC) $(CFLAGS) -fsyntax-only */*.c */*.h
