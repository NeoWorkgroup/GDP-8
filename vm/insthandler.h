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

struct Handler handler[256];
void op_nop_exec(struct CPU *cpu, struct Instruction *inst);
void op_nop_decode(memory_t *memory, struct Instruction *inst);
