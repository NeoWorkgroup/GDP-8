CC = clang
CFLAGS_OPT = -Ofast -gfull -gdwarf-5 -Wall -Wextra -pipe -static -march=native -fPIE -flto -I.
CLFAGS=-O2 -gfull -pipe -I.

all: gdp8

gdp8:
	$(CC) $(CFLAGS) $(LDFLAGS) gdp8.c -o gdp8
gdp8-static:
	$(CC) $(CFLAGS_OPT) $(LDFLAGS) gdp8.c -o gdp8

clean:
	rm -v gdp8
