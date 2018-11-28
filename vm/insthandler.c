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

void op_nop_exec(struct CPU *cpu, struct Instruction *inst)
{
	return;
}

void op_nop_decode(memory_t *memory, struct Instruction *inst)
{
	inst->op=*memory;
}
