CC = clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I./vm
LDFLAGS=
VMOBJS= vm/main.o vm/inst.o vm/insthandler.o vm/cpu.o
all: gdp8 assembler
gdp8: $(VMOBJS)
	$(CC) $(LDFLAGS) $(VMOBJS) -o gdp8

assembler: asm/asm.o
	$(CC) $(LDFLAGS) asm/asm.o -o assembler

clean:
	rm -rfv $(VMOBJS) gdp8 assembler

syntax:
	$(CC) $(CFLAGS) -fsyntax-only */*.c */*.h
