CC=clang
CFLAGS=-O2 -finline -g -pipe -Wall -Wextra -I. -I../vm
PROG_PREFIX=
# Use this if you are interested in debugging Compilers
#CFLAGS = -Ofast -gfull -gdwarf-5 -Wall -Wextra -pipe -static -march=native -fPIE -flto -I.

all: asm
syntax:
	$(CC) $(CFLAGS) -fsyntax-only *.c
asm:	asm.o
	$(CC) $(LDFLAGS) -o $(PROG_PREFIX)asm \
		asm.o
asm.o:
	$(CC) $(LDFLAGS) $(CFLAGS) -c $(.PREFIX).c

clean:
	-rm -fv asm *.o
