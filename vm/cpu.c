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

void cpu_init(struct CPU *cpu)
{
 	memset(cpu, 0x00, sizeof(struct CPU));
	/* Allocate memory */
	if((cpu->mem = malloc(1 << 24)) == NULL)
	{
		perror("?MEM");
		exit(8);
	}
}

void cpu_mainloop(struct CPU *cpu, addr_t address)
{
	int ret=0;
	while(cpu->reg.pc < (1<<24))
	{
		ret = fetch(cpu->mem + cpu->reg.pc, &(cpu->ireg.inst));
		if(ret == EOF)
			goto err;
		else
			cpu->reg.pc += ret;
		handler[cpu->ireg.inst.op].exec(cpu);
	}
	return;
err:
	panic("?ILL\n");
}
