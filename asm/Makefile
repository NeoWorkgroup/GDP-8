CC=clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I.
PROG_PREFIX=
# Use this if you are interested in debugging Compilers
#CFLAGS = -Ofast -gfull -gdwarf-5 -Wall -Wextra -pipe -static -march=native -fPIE -flto -I.

all: asm
syntax:
	$(CC) $(CFLAGS) -fsyntax-only *.c
asm:
	$(CC) $(LDFLAGS) -o $(PROG_PREFIX)asm \
		asm.o
asm.o:
	$(CC) $(LDFLAGS) -c $(PREFIX).c

clean:
	-rm -fv asm *.o
