/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cpu.h"

#ifdef __linux__
#include <endian.h>
#else
#include <sys/endian.h>
#endif

struct Handler handler[256];
struct IOHandler iohandler[256];

#define DECODE_DEFINE(name) \
	void op_ ## name ## _decode(memory_t *memory, struct Instruction *inst)
#define EXEC_DEFINE(name) \
	void op_ ## name ## _exec(struct CPU *cpu)
#define IO_DEFINE(name) \
	void io_ ## name(struct CPU *cpu)

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

DECODE_DEFINE(dec)
{
	inst->op = DEC;
	inst->arg.u.reg = getbyte(memory + 1);
}

EXEC_DEFINE(dec)
{
	R(cpu, INST(cpu).arg.u.reg)--;
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
	if(IREG(cpu).usermode && INST(cpu).arg.u.reg != 0x80)
		panic("?INTUM");
	REG(cpu).iv = INST(cpu).arg.u.reg;
	REG(cpu).ipc = PC(cpu);
	IREG(cpu).iusermode = IREG(cpu).usermode;
	IREG(cpu).interrupt=TRIGGERED_INTERRUPT;
}

IO_DEFINE(cpu)
{
	switch(INST(cpu).arg.io.op)
	{
		case CPU_IO_EI:
			IREG(cpu).interrupt=NO_INTERRUPT;
			break;
		case CPU_IO_DI:
			IREG(cpu).interrupt=INTERRUPT_DISABLED;
			break;
		case CPU_IO_ECLK:
		case CPU_IO_DCLK:
			break;
		case CPU_IO_PSR:
			R(cpu, INST(cpu).arg.io.reg) = IREG(cpu).panelswitch;
			break;
		case CPU_IO_DPY:
			IREG(cpu).display = R(cpu, INST(cpu).arg.io.reg);
			break;
	}
}

IO_DEFINE(console)
{
	switch(INST(cpu).arg.io.op)
	{
		case CONSOLE_IO_OUT:
			fputc((int)(R(cpu, INST(cpu).arg.io.reg) & 0xFF), stdout);
			break;
		case CONSOLE_IO_IN:
			R(cpu, INST(cpu).arg.io.reg)=(word_t)fgetc(stdin);
			break;
		case CONSOLE_IO_BEL:
			fputc('\a', stdout);
			break;
		case CONSOLE_IO_RST:
			fputs("\033)0\033[H\033[J", stdout);
			break;
	}
}

#define _decode(name)	op_ ## name ## _decode
#define _execute(name)	op_ ## name ## _exec
#define _io(name)	io_ ## name

#define handler_add(name, op, instsize)		\
	[op] =					\
	{					\
		.defined=	1,		\
		.size=		instsize,	\
		.exec=		_execute(name),	\
		.decode=	_decode(name)	\
	}

#define io_add(name, dev)			\
	[dev] =					\
	{					\
		.defined=	1,		\
		.exec=		_io(name)	\
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

void panic(const char *msg)
{
	fputs(msg, stderr);
	exit(144);
}

/* Fetch & Decode instruction */
int fetch(memory_t *memory, struct Instruction *inst)
{
	byte_t opcode = memory[0];
	if(!handler[opcode].defined)
		return EOF;
	else
		handler[opcode].decode(memory, inst);
	return handler[opcode].size;
}

addr_t getaddress(memory_t *memory)
{
	addr_t address=0;
	memcpy(&address, memory, 3);
	return le32toh(address);
}

word_t getword(memory_t *memory)
{
	word_t word=0;
	memcpy(&word, memory, sizeof(word_t));
	return le64toh(word);
}

quart_t getquarter(memory_t *memory)
{
	quart_t quarter=0;
	memcpy(&quarter, memory, sizeof(quart_t));
	return le16toh(quarter);
}

byte_t getbyte(memory_t *memory)
{
	byte_t byte;
	memcpy(&byte, memory, sizeof(byte_t));
	return byte;
}

void putword(memory_t *memory, word_t word)
{
	word_t target = htole64(word);
	memcpy(memory, &target, 8);
}

void putaddress(memory_t *memory, addr_t address)
{
	addr_t target = htole32(address & ADDRMASK);
	memcpy(memory, &target, 3);
}

void putbyte(memory_t *memory, byte_t byte)
{
	memcpy(memory, &byte, sizeof(byte_t));
}

addr_t getrealaddr(struct CPU *cpu, addr_t address, bit_t indirect)
{
	addr_t dest=0;
	address &= (1<<24) - 1;
	if(indirect)
		dest=getaddress(MEM(cpu) + address);
	else
		dest=address;
	return dest;
}

struct CPU *cpu_init(void)
{
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	/* Allocate memory */
	if((MEM(cpu) = malloc(1 << 24)) == NULL)
	{
		perror("?MEM");
		exit(8);
	}
	return cpu;
}

struct CPU *cpu_add(struct CPU *mastercpu)
{
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	MEM(cpu) = MEM(mastercpu);
	return cpu;
}

void cpu_destroy(struct CPU *cpu)
{
	free(MEM(cpu));
	free(cpu);
}

void cpu_remove(struct CPU *cpu)
{
	free(cpu);
}

void cpu_mainloop(struct CPU *cpu, addr_t address)
{
	int ret=0;
	PC(cpu)=address;
	while(IREG(cpu).halted != 1)
	{
		if(cpu->reg.pc > ADDRMASK)
			goto pc_too_large;
		ret = fetch(MEM(cpu) + PC(cpu), &INST(cpu));
		if(ret == EOF)
			goto err;
		else
			PC(cpu) += ret;
		handler[INST(cpu).op].exec(cpu);
		if(IREG(cpu).interrupt == TRIGGERED_INTERRUPT)
		{
			IREG(cpu).interrupt=ONINTERRUPT;
			PC(cpu) = 0xF00000 + (REG(cpu).iv << 4);
		}
	}
	return;
pc_too_large:
	panic("?PC\n");
err:
	panic("?ILL\n");
}
