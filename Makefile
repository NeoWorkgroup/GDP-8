CC=	cc
WFLAGS=	-Wno-unused-parameter
CFLAGS=	-O2 -g -pipe -Wall -Wextra -pedantic -I./vm $(WFLAGS)
LDFLAGS=
LEX=	lex
VMOBJS= vm/main.o vm/handler.o vm/insthandler.o vm/cpu.o vm/io.o
ALLOBJS=	$(VMOBJS) asm/asm.o asm/lex.yy.c
EXEC=	gdp8 assembler hex2bin asmlex
all:	$(EXEC)
gdp8: $(VMOBJS)
	$(CC) $(LDFLAGS) $(VMOBJS) -o gdp8

assembler: asm/asm.o
	$(CC) $(LDFLAGS) asm/asm.o -o assembler

asmlex: asm/gdp8.c
	$(CC) $(CFLAGS) asm/gdp8.c -o asmlex -ll

hex2bin: asm/hex2bin.o
	$(CC) $(LDFLAGS) asm/hex2bin.o -o hex2bin

clean:
	rm -rfv $(ALLOBJS) $(EXEC)

syntax:
	$(CC) $(CFLAGS) -fsyntax-only */*.c */*.h
