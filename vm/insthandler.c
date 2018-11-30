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
	IREG(cpu).halted=1;
}

DECODE_DEFINE(hlt)
{
	inst->op=HLT;
}

EXEC_DEFINE(io)
{
	iohandler[INST(cpu).arg.io.dev].exec(cpu);
}

DECODE_DEFINE(io)
{
	inst->op=IO;
	inst->arg.io.dev=	memory[1];
	inst->arg.io.op=	memory[2];
	inst->arg.io.reg=	memory[3];
}

DECODE_DEFINE(j)
{
	inst->op=J;
	inst->arg.jc.address=getaddress(memory + 1);
	if(memory[0] & 0x01) /* Indirect */
		inst->arg.jc.indirect=1;
	else
		inst->arg.jc.indirect=0;
}

EXEC_DEFINE(j)
{
	PC(cpu) = getrealaddr(cpu, INST(cpu).arg.jc.address, INST(cpu).arg.jc.indirect);
}
