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

#define AC ac
#define MQ mq
#define STATUS status
#define MEM(x) memory[x]
#define PC pc
#define L status.l
#define CF cf
#define DF df

#define SETL(x) \
	(l=(x%2))
/* Combine Field and Address */
#define FADDR(f, a) \
	((f << 16) | a)

/* Get Page Address's Real Address */
#define PADDR(pc, a) \
	((pc&0xff0000)|a)

/* Get Real Address */
#define ADDR(field, pc, addr) \
	((field << 16)|((pc&0xff0000)|addr))

/* Power of 2 */
#define POWTWO(exp) \
	(1 << exp)

/* Registers:
 * AC:	Accumulator
 * MQ:	Multiplier Quotient
 * PC:	Program Counter
 * L:	The Link
 * STATUS:
 */

/* Normal Instruction Format:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   4   |1|1|1|      10         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  O P  |I|C|M|    A D D R      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * I:	Indirect
 * C:	Current Page
 * M:	Use MQ
 */

/* OPR Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       | 1 | 1 |   2   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     | I | M |  G:0  |   |   |   |   |   |   |   |   | Group 1
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     | I | M |  G:1  |   |   |   |   |   |   |   |   | Group 2
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     | I | M |  G:2  |   |   |   |   |   |   |   |   | Group 3
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     | I | M |  G:3  |   |   |   |   |   |   |   |   | Group 4
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * I:	Indirect
 * M:	Use MQ
 * G:	Group 1, 2, 3, or 4
 */

/* IOT Instruction Format:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   4   |       8       |   4   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  O P  |  D E V I C E  |C O D E|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/* Status Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 1 | 1 | 1 | 1 | 1 |     3     |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | L |G T|BIT|DIT|U M|  S T A T  |        EFFECTIVE FIELD        |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * L:	The Link
 * GT:	Greater Than
 * BIT:	Bus Interrupt
 * DIT:	Disabled Interrupt
 * UM:	Usermode
 * STAT:	Status Code
 *
 * Status Code:
 * 0:	No Error
 * 1:	Encountered Interrupt
 * 2:	Trap
 * 3:	Illegal Instruction
 * 4:	HLT in Usermode
 * 5:	IOT, OSR in Usermode
 * 6:	Cross Field JMP, JMS Usermode
 * 7:	Illegal IOT Port, or Code
 *
 * Saved Field Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |               8               |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      C O D E   F I E L D      |      D A T A   F I E L D      |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 */

/* Corefile Format:
 * "012345:6789"
 * 24 Bit : 16 Bit Hexdecimal
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
		memory[FADDR(field, addr)]=word;
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
