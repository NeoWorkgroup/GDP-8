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

struct Handler handler[256] =
{
	[NOP] =
	{
		.defined=	1,
		.size=		1,
		.exec=		op_nop_exec,
		.decode=	op_nop_decode
	}
};
