/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "libgdp8.h"
#define DECODE_DEFINE(name) \
	void op_ ## name ## _decode(memory_t *memory, struct Instruction *inst)
#define EXEC_DEFINE(name) \
	void op_ ## name ## _exec(struct CPU *cpu)

#define INST_DEFINE(name) \
	void op_ ## name ## _decode(memory_t *memory, struct Instruction *inst); \
	void op_ ## name ## _exec(struct CPU *cpu)

#define IO_DEFINE(name) \
	void io_ ## name(struct CPU *cpu)

struct Handler handler[256];
struct IOHandler iohandler[256];
INST_DEFINE(nop);
INST_DEFINE(hlt);
INST_DEFINE(int);
INST_DEFINE(io);
INST_DEFINE(iret);
INST_DEFINE(j);
INST_DEFINE(ld);
INST_DEFINE(st);
INST_DEFINE(li);
INST_DEFINE(inc);
INST_DEFINE(dec);
IO_DEFINE(cpu);
IO_DEFINE(console);
