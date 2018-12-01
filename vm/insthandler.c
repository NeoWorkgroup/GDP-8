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

DECODE_DEFINE(nop)
{
	inst->op = NOP;
}

EXEC_DEFINE(nop)
{
	return;
}

DECODE_DEFINE(hlt)
{
	inst->op = HLT;
}

EXEC_DEFINE(hlt)
{
	IREG(cpu).halted=1;
}

DECODE_DEFINE(io)
{
	inst->op = IO;
	inst->arg.io.dev =	getbyte(memory + 1);
	inst->arg.io.op =	getbyte(memory + 2);
	inst->arg.io.reg =	getbyte(memory + 3);
}

EXEC_DEFINE(io)
{
	iohandler[INST(cpu).arg.io.dev].exec(cpu);
}

DECODE_DEFINE(iret)
{
	inst->op = IRET;
}

EXEC_DEFINE(iret)
{
	PC(cpu) = REG(cpu).ipc;
	IREG(cpu).usermode = IREG(cpu).iusermode;
	IREG(cpu).interrupt = NO_INTERRUPT;
}

DECODE_DEFINE(j)
{
	inst->op = J;
	inst->arg.jc.address = getaddress(memory + 1);
	if(memory[0] & 0x01) /* Indirect */
		inst->arg.jc.indirect=1;
	else
		inst->arg.jc.indirect=0;
}

EXEC_DEFINE(j)
{
	PC(cpu) = getrealaddr(cpu, INST(cpu).arg.jc.address, INST(cpu).arg.jc.indirect);
}

#define is_index(x)	(((x) & 0x08) >> 3)
#define index(x)	(((x) & 0xF0) >> 4)
#define is_indirect(x)	(((x) & 0x04) >> 2)
#define size(x)		(((x) & 0x03))

#define WORD		0
#define ADDR		1
#define BYTE		2

DECODE_DEFINE(ld)
{
	inst->op = LD;
	inst->arg.ls.ind =	is_index(getbyte(memory + 1));
	inst->arg.ls.index =	index(getbyte(memory + 1));
	inst->arg.ls.indirect =	is_indirect(getbyte(memory + 1));
	inst->arg.ls.size =	size(getbyte(memory + 1));
	inst->arg.ls.address =	getaddress(memory + 3);
}

EXEC_DEFINE(ld)
{
	addr_t target_address=0;
	word_t word=0;
	if(INST(cpu).arg.ls.ind)
	{
		target_address = getrealaddr(cpu,
			INST(cpu).arg.ls.address + R(cpu, INST(cpu).arg.ls.index),
			INST(cpu).arg.ls.indirect);
	}
	else
	{
		target_address = getrealaddr(cpu,
			INST(cpu).arg.ls.address,
			INST(cpu).arg.ls.indirect);
	}

	switch(INST(cpu).arg.ls.size)
	{
		case WORD:
			word = getword(MEM(cpu) + target_address);
			break;
		case ADDR:
			word = getaddress(MEM(cpu) + target_address);
			break;
		case BYTE:
			word = getbyte(MEM(cpu) + target_address);
			break;
	}

	R(cpu, INST(cpu).arg.ls.reg) = word;
}

DECODE_DEFINE(st)
{
	inst->op = ST;
	inst->arg.ls.ind =	is_index(getbyte(memory + 1));
	inst->arg.ls.index =	index(getbyte(memory + 1));
	inst->arg.ls.indirect =	is_indirect(getbyte(memory + 1));
	inst->arg.ls.size =	size(getbyte(memory + 1));
	inst->arg.ls.address =	getaddress(memory + 3);
}

EXEC_DEFINE(st)
{
	addr_t target_address=0;
	word_t word = R(cpu, INST(cpu).arg.ls.reg);
	if(INST(cpu).arg.ls.ind)
	{
		target_address = getrealaddr(cpu,
			INST(cpu).arg.ls.address + R(cpu, INST(cpu).arg.ls.index),
			INST(cpu).arg.ls.indirect);
	}
	else
	{
		target_address = getrealaddr(cpu,
			INST(cpu).arg.ls.address,
			INST(cpu).arg.ls.indirect);
	}

	switch(INST(cpu).arg.ls.size)
	{
		case WORD:
			putword(MEM(cpu) + target_address, word);
			break;
		case ADDR:
			putaddress(MEM(cpu) + target_address, (addr_t)word);
			break;
		case BYTE:
			putbyte(MEM(cpu) + target_address, (byte_t)word);
			break;
	}
}

DECODE_DEFINE(li)
{
	inst->op = LI;
	inst->arg.iq.reg = getbyte(memory + 1);
	inst->arg.iq.value = getquarter(memory + 2);
}

EXEC_DEFINE(li)
{
	R(cpu, INST(cpu).arg.iq.reg) = INST(cpu).arg.iq.value;
}

DECODE_DEFINE(inc)
{
	inst->op = INC;
	inst->arg.u.reg = getbyte(memory + 1);
}

EXEC_DEFINE(inc)
{
	R(cpu, INST(cpu).arg.u.reg)++;
}

DECODE_DEFINE(int)
{
	inst->op = INT;
	inst->arg.u.reg = getbyte(memory + 1); /* Actually not register */
}

EXEC_DEFINE(int)
{
	if(IREG(cpu).interrupt == INTERRUPT_DISABLED)
		panic("?INTDISABLE");
	REG(cpu).iv = INST(cpu).arg.u.reg;
	REG(cpu).ipc = PC(cpu);
	IREG(cpu).iusermode = IREG(cpu).usermode;
	IREG(cpu).interrupt=TRIGGERED_INTERRUPT;
}
