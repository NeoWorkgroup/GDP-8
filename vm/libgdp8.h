/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

typedef uint8_t byte_t;
typedef uint8_t bit_t;
typedef uint64_t word_t;
typedef uint32_t addr_t;
typedef uint8_t reg_t;
typedef int32_t offset_t;
typedef uint8_t memory_t;

/* Definition */

enum InstructionList
{
	NOP, HLT, BRK, SLP, WAIT,
	IDLE, INT, EUM, SYS, IO, IRET,
	LD, ST, LI, LIW,
	ADD, SUB, INC, DEC, MUL, DIV, MOD,
	ADDI, SUBI, INCI, DECI,
	AND, OR, NOT, XOR,
	RTR, RTL, SHR, SHL,
	RTBR, RTBL, SHBR, SHBL,
	SWP, MOV, PUSH, POP,
	CMP, TCH, STB, CLB, CLR,
	J, JS, JR, JSR, RS,
	C, CR, R,
	INST_OPS
};

struct Register
{
	word_t r[256];
	addr_t pc;
	addr_t sp;
	addr_t sra;
	addr_t ipc;
	byte_t iv;
	byte_t status;
};

struct InternalRegister
{
	bit_t usermode;
	bit_t interrupt;
	word_t display;
};

struct arg_u
{
	reg_t reg;
};

struct arg_d
{
	reg_t dst;
	reg_t src;
};

struct arg_if
{
	reg_t reg;
	uint16_t value;
};

struct arg_iw
{
	reg_t reg;
	word_t value;
};

struct arg_ls
{
	bit_t ind;
	reg_t index;
	bit_t indirect;
	bit_t size;
};

struct arg_jc
{
	bit_t indirect;
	addr_t address;
};

struct arg_rjc
{
	bit_t indirect;
	bit_t condition;
	offset_t offset;
};

struct arg_io
{
	byte_t dev;
	byte_t op;
	reg_t reg;
};

struct arg_sys
{
	byte_t num;
	reg_t reg1;
	reg_t reg2;
};

union arg_union
{
	struct arg_u	_u;
	struct arg_d	_d;
	struct arg_if	_if;
	struct arg_iw	_iw;
	struct arg_ls	_ls;
	struct arg_jc	_jc;
	struct arg_rjc	_rjc;
	struct arg_io	_io;
};

struct Instruction
{
	byte_t op;
	union arg_union arg;
};

struct CPU
{
	struct Register reg;
	struct InternalRegister ireg;
	memory_t *mem;
};

struct InterpretHandler
{
	byte_t size;
	void(*handler)(struct CPU *cpu, struct Instruction *inst);
};
