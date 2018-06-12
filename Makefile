CC = clang
CFLAGS_OPT = -Ofast -gfull -gdwarf-5 -Wall -Wextra -pipe -static -march=native -fPIE -flto -I.
CFLAGS=-O2 -gfull -pipe -I.

all: gdp8 gdp8-static coretest

gdp8:
	$(CC) $(CFLAGS) $(LDFLAGS) gdp8.c -o gdp8
gdp8-static:
	$(CC) $(CFLAGS_OPT) $(LDFLAGS) gdp8.c -o gdp8.static
coretest:
	$(CC) $(CFLAGS) $(LDFLAGS) coretest.c -o coretest
clean:
	rm -fv gdp8 coretest
