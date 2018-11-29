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

EXEC_DEFINE(nop)
{
	return;
}

DECODE_DEFINE(nop)
{
	inst->op=NOP;
}

EXEC_DEFINE(hlt)
{
	panic("?HLT");
}

DECODE_DEFINE(hlt)
{
	inst->op=HLT;
}
