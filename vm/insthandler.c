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
	panic("?HLT\n");
}

DECODE_DEFINE(hlt)
{
	inst->op=HLT;
}

EXEC_DEFINE(io)
{
	iohandler[cpu->ireg.inst.arg.io.dev].exec(cpu);
}

DECODE_DEFINE(io)
{
	/* TODO: We need a better decoder and array bound checking */
	inst->op=IO;
	inst->arg.io.dev=	memory[1];
	inst->arg.io.op=	memory[2];
	inst->arg.io.reg=	memory[3];
}
