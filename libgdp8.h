/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ENCODING: UTF-8
 * LANGUAGE: zh_TW, en_US
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define AC ac.word
#define MQ mq.word
#define STATUS status.word
#define MEM(x) memory[x].word
#define PC pc
#define L l
#define CF cf
#define DF df
#define SETL(x) \
	(l=(x%2))

#define TO_MEM(f, a) \
	((f << 16) | a)

#define POWTWO(exp) \
	(1 << exp)

/* 暫存器：
 * AC:	累加器(Accumulator)， 也被 PSH, POP, CALL, RET 當作是 Stack Pointer
 * MQ:	倍數/商數(Multiplier Quotient)
 * PC:	程式計數器(Program Counter)
 * L:	The Link
 * STATUS:	狀態
 */

/* 指令格式：
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   4   |1|1|        10         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  O P  |I|C|    A  D  D  R     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct
{
	uint16_t opcode	:4;	/* OpCode Number */
	uint16_t indirect	:1;	/* Indirect */
	uint16_t current	:1;	/* Use Current Page */
	uint16_t addr	:10;	/* Address inside Page */
} instruction_t;

/* OPR 指令格式：
 * +-+-+-+-+---+---+---+---+---+---+---+---+---+---+---+---+
 * |   4   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
 * +-+-+-+-+---+---+---+---+---+---+---+---+---+---+---+---+
 * |  O P  | I |   |   |   |   |   |   |   |   |   |   |   |
 * +-+-+-+-+---+---+---+---+---+---+---+---+---+---+---+---+
 */

/* Corefile Format:
 * "01234:56789ABC"
 * 20 Bit : 16 Bit Hexdecimal
 * Any Invaild Input will be ignored
 */

void load_core(FILE *fp)
{
	uint16_t word=0;
	uint16_t addr=0;
	uint8_t field=0;
	uint32_t readed=0;
	while(fscanf(fp, "%x:%x\n", &addr, &word) != EOF)
	{
		field=(addr&0xFF0000) >> 16;
		addr&=0x00FFFFFF;
		memory[TO_MEM(field, addr)]=word;
		readed++;
	}
}

void dump_core(FILE *fp)
{
	uint32_t word=0;
	uint16_t addr=0;
	for(addr=0; addr =< 0xffff; addr++)
	{
		fprintf(fp, "%06x:%04hx\n", addr, word)
	}
}

uint16_t rotl(uint16_t word, uint8_t count)
{
	return (word << count) | (word >> (16 - count));
}

uint16_t rotr(uint16_t word, uint8_t count)
{
	return (word >> count)|(word << (16 - count));
}

void interrupt(uint32_t addr)
{
	MEM(0x00000)=addr;
	PC=0x1;
}
