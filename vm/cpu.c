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
#include "libgdp8.h"
#include "insthandler.h"

#ifdef __linux__
#include <endian.h>
#else
#include <sys/endian.h>
#endif

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

addr_t getrealaddr(struct CPU *cpu, addr_t address, bit_t indirect)
{
	addr_t dest=0;
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

void cpu_destroy(struct CPU *cpu)
{
	free(MEM(cpu));
	free(cpu);
}

void cpu_mainloop(struct CPU *cpu, addr_t address)
{
	int ret=0;
	PC(cpu)=address;
	while(IREG(cpu).halted != 1)
	{
		if(cpu->reg.pc >= (1<<24))
			goto pc_too_large;
		ret = fetch(MEM(cpu) + PC(cpu), &INST(cpu));
		if(ret == EOF)
			goto err;
		else
			PC(cpu) += ret;
		handler[INST_OP(cpu)].exec(cpu);
	}
	return;
pc_too_large:
	panic("?PC\n");
err:
	panic("?ILL\n");
}
