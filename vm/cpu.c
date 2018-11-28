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

/* Fetch / Decode instructing */
ssize_t fetch(memory_t *memory, struct Instruction *inst)
{
	byte_t opcode = memory[0];
	if(handler[opcode].defined == 0)
		return EOF;
	else
		handler[opcode].decode(memory, inst);
	return handler[opcode].size;
}
