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
#define exec(name)	op_ ## name ## _exec
#define io(name)	io_ ## name

struct Handler handler[256] =
{
	[NOP] =
	{
		.defined=	1,
		.size=		1,
		.exec=		exec(nop),
		.decode=	decode(nop)
	},
	[HLT] =
	{
		.defined=	1,
		.size=		1,
		.exec=		exec(hlt),
		.decode=	decode(hlt)
	},
	[IO] =
	{
		.defined=	1,
		.size=		4,
		.exec=		exec(io),
		.decode=	decode(io)
	},
	[J] =
	{
		.defined=	1,
		.size=		4,
		.exec=		exec(j),
		.decode=	decode(j)
	},
	[JI] =
	{
		.defined=	1,
		.size=		4,
		.exec=		exec(j),
		.decode=	decode(j)
	}
};

struct IOHandler iohandler[256] =
{
	[CPU] =
	{
		.defined=	1,
		.exec=		io(cpu)
	},
	[CONSOLE] =
	{
		.defined=	1,
		.exec=		io(console)
	}
};
