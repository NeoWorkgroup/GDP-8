CC = clang
CFLAGS=-O2 -g -pipe -Wall -Wextra -I.
LDFLAGS=
OBJS= main.o inst.o insthandler.o cpu.o
all: $(OBJS) gdp8
gdp8:
	$(CC) $(LDFLAGS) $(OBJS) -o gdp8

clean:
	rm -fv $(OBJS) gdp8

syntax:
	$(CC) $(CFLAGS) -fsyntax-only *.c *.h
