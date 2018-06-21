CC = clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I.

# Use this if you are interested in debugging Compilers
#CFLAGS = -Ofast -gfull -gdwarf-5 -Wall -Wextra -pipe -static -march=native -fPIE -flto -I.

all: gdp8
syntax:
	$(CC) $(CFLAGS) -fsyntax-only *.c
gdp8:	gdp8.o io.o inst.c
	$(CC) $(LDFLAGS) -o gdp8 \
		gdp8.o io.o inst.o
gdp8.o:
	$(CC) $(CFLAGS) -c ${.PREFIX}.c
io.o:
	$(CC) $(CFLAGS) -c ${.PREFIX}.c
inst.o:
	$(CC) $(CFLAGS) -c ${.PREFIX}.c

coretest:
	$(CC) $(CFLAGS) $(LDFLAGS) coretest.c -o coretest

clean:
	-rm -fv gdp8 coretest *.o
