/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "libgdp8.h"
#include "insthandler.h"

#define decode(name)	op_ ## name ## _decode
#define execute(name)	op_ ## name ## _exec
#define io(name)	io_ ## name

#define handler_add(name, op, instsize)		\
	[op] =					\
	{					\
		.defined=	1,		\
		.size=		instsize,	\
		.exec=		execute(name),	\
		.decode=	decode(name)	\
	}

#define io_add(name, dev)			\
	[dev] =					\
	{					\
		.defined=	1,		\
		.exec=		io(name)	\
	}

struct Handler handler[256] =
{
	handler_add(nop, NOP, 1),
	handler_add(hlt, HLT, 1),
	handler_add(int, INT, 2),
	handler_add(io, IO, 4),
	handler_add(iret, IRET, 1),
	handler_add(j, J, 4),
	handler_add(j, JI, 4),
	handler_add(ld, LD, 6),
	handler_add(st, ST, 6),
	handler_add(li, LI, 4),
	handler_add(inc, INC, 2),
	handler_add(dec, DEC, 2)
};

struct IOHandler iohandler[256] =
{
	io_add(cpu, CPU),
	io_add(console, CONSOLE)
};
