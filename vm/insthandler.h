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
#define IO_DEFINE(name) \
	void io_ ## name(struct CPU *cpu)

struct Handler handler[256];
struct IOHandler iohandler[256];
DECODE_DEFINE(nop);
EXEC_DEFINE(nop);
DECODE_DEFINE(hlt);
EXEC_DEFINE(hlt);
DECODE_DEFINE(io);
EXEC_DEFINE(io);
IO_DEFINE(cpu);
IO_DEFINE(console);
