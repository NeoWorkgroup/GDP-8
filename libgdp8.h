/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define AC(x) ac[x]
#define MQ mq
#define STATUS st
#define MEM(x) memory[x]
#define PC pc
#define L ((st&0x8000)>>15)
#define SC sc
#define CF ((field & 0xFF00) >> 8)
#define DF (field & 0x00FF)

#define SETL(x) \
	(l=(x%2))
/* Combine Field and Address */
#define FADDR(f, a) \
	((f << 16) | a)

/* Get Page Address's Real Address */
#define PADDR(pc, a) \
	((pc&0xffff00)|a)

/* Get Real Address */
#define ADDR(field, pc, addr) \
	((field << 16)|((pc&0xffff00)|addr))

/* Power of 2 */
#define POWTWO(exp) \
	(1 << exp)

/* Device Handler Function Pointer */
typedef uint16_t (*DEV_HANDLER)(uint8_t, uint8_t);
/* First uint8_t is Device Number,
   Second uint8_t is CODE */

/* Registers:
 * AC0 ~ AC3:	Accumulator
 * MQ:	Multiplier Quotient
 * PC:	Program Counter
 * L:	The Link
 * STATUS: Status
 */

/* Normal Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       |   2   | 1 | 1 |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      O P      |   A   | I | C |            A D D R            |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * I:	Indirect
 * C:	Current Page
 * A:	Which Accumulator
 */

/* IOT Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       |               8               |       4       |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      O P      |          D E V I C E          |    C O D E    |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 */

/* OPR Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       |   2   |   2   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     |   A   |  G:0  | I |CLW|CLL|RVW|RVL|ROR|ROL|TWO| ==> Group 1
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     |   A   |  G:1  | I |CLW|ICW|SMW|SZW|SNL|REV|OSR| ==> Group 2
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     |   A   |  G:2  | I |CLW|MTW|WTM| Arithmetic OP | ==> Group 3
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |     O   P     |   A   |  G:3  | I |INC|DEC|RNL|RNR|   N U M   | ==> Group 4
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * I:	Indirect
 * A:	Which Accumulator
 * G:	Group 1, 2, 3, or 4
 *
 * WORD:	if I=0, use AC, if I=1, use *AC
 *
 * CLW:	Clear WORD
 * CLL:	Clear L
 *
 * RVW:	Reverse WORD
 * RVL:	Reverse L
 *
 * ROR:	Rotate Right
 * ROL:	Rotate Left
 * TWO:	Rotate 2 Bits, if(ROR==0 && ROL==0), Swap upper and lower half
 *
 * ICW:	Increment WORD
 * OSR:	OR the content of Panel Buttons into WORD
 *
 * SMW:	Skip if WORD is negative
 * SZW:	Skip if WORD is 0x0000
 * SNL:	Skip if L is 0
 * REV: Reverse the Condition of SMA, SZA, and SNL
 *
 * MTW:	Store MQ into WORD, and Clear MQ
 * WTM:	Store WORD into MQ, and Clear WORD
 *
 * INC:	Increment by NUM
 * DEC:	Decrement by NUM
 * RNL: Rotate NUM Bits Left
 * RNR:	Rotate NUM Bits Right
 *
 *
 * Arithmetic OP:
 *	CODE	MNEMONIC	COMMENT
 * 	0:	NOOP		No Operation
 * 	1:	WCS		Load WORD%POWTWO(5) into SC
 * 	2:	SUB		Subtract
 * 	3:	MUL		Multiply
 * 	4:	DVI		Divide
 * 	5:	NMI		Normalize
 * 	6:	SHL		Shift Left
 * 	7:	ASR		Arithmetic Right Shift
 * 	8:	LSR		Logical Right Shift
 * 	9:	SCA		AC |= SC
 * 	A:	DAD		Double Precision Add
 * 	B:	DST		Double Precision Store
 * 	C:	DPSZ		Double Precision Skip if 0
 * 	D:	DPIC		Double Precision Increment
 * 	E:	DRV		Double Precision Reverse
 * 	F:	SWM		Subtract WORD from MQ
 */

/* Status Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 1 | 1 | 1 | 1 | 1 |     3     |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | L |G T|INT|DIT|U M|  S T A T  |      INTERRUPTING DEVICE      |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * L:	The Link
 * GT:	Greater Than
 * INT:	Device Interrupt
 * DIT:	Disabled Interrupt
 * UM:	Usermode
 * STAT:	Status Code
 * INTERRUPTING DEVICE:	The Device which is requesting a interrupt
 *
 * Status Code:
 * 0:	Nothing
 * 1:	Clock Interrupt
 * 2:	Device Interrupt
 * 3:	System Call
 * 4:	Trap
 * 5:	Usermode Interrupt
 * 6:	STP, HLT, OSR, Cross Field JMP, and JMS in Usermode
 * 7:	BUG in Kernel Mode, KERNEL PANIC
 *
 * Field Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |               8               |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      C O D E   F I E L D      |      D A T A   F I E L D      |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 */

/* Instructions:
 *
 * OP	NAME	USE AC?		DESCRIPTION
 *
 * 0	AND	YES		AND AC with the Target
 * 1	ADD	YES		ADD the Accumulator with the Target
 * 2	ISZ	NO		ADD the Target, and skip next instruction if result is zero
 * 3	DEP	YES		Deposit the Accumulator into the Target
 * 4	JMS	NO		Jump to the Subroutine
 * 5	JMP	NO		Jump to the Target
 * 6	IOT	YES		I/O Transfer
 * 7	OPR	YES		Operation
 * 8	PUSH	YES		Push into Stack
 * 9	POP	YES		Pop off Stack
 * A	CALL	YES		Call function
 * B	RET	YES		Return
 * C	EUM	NO		Enter Usermode at the Address
 * D	INT	YES		Interrupt, same Format as IOT
 * E	SYS	YES		System Call, same Format as IOT, triggers a Interrupt
 * F	STP	NO		Halt, Halt and Catch Fire, or Report Bug
 */

/* Corefile Format:
 * "012345:ABCD"
 * 24 Bit : 16 Bit Hexdecimal
 * Any Invaild Input will be ignored
 */

enum INSTRUCTIONS
{
	AND, ADD, ISZ, DEP,
	JMS, JMP, IOT, OPR,
	PUSH, POP, CALL, RET,
	EUM, INT, SYS, STP
}

void load_core(FILE *fp)
{
	extern uint16_t *memory;
	uint16_t word=0;
	uint32_t addr=0;
	uint8_t field=0;
	uint32_t readed=0;
	while(fscanf(fp, "%x:%hx\n", &addr, &word) != EOF)
	{
		field=(addr&0xFF0000) >> 16;
		addr&=0x00FFFFFF;
		MEM(FADDR(field, addr))=word;
		readed++;
	}
}

void dump_core(FILE *fp)
{
	uint16_t word=0;
	uint32_t addr=0;
	for(addr=0; addr <= 0xFFFFFF; addr++)
	{
		fprintf(fp, "%06x:%04hx\n", addr, word);
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

/* Rotate L with WORD */
void lwrotr(uint16_t *word)
{
	extern uint16_t st;
	uint16_t temp=*word;
	*word=((temp >> 1) | (st & 0x8000));
	st=((st & 0x7FFF) | ((temp & 0x0001) << 15));
	return;
}

void lwrotl(uint16_t *word)
{
	extern uint16_t st;
	uint16_t temp=*word;
	*word=((temp << 1) | (st >> 15));
	st((st & 0x7FFF) | ((temp & 0x8000) << 15));
	return;
}

/* Interrupt */
void interrupt(uint32_t orig_address, unsigned int code)
{
	extern uint16_t *memory, pc;
	extern uint16_t field, st, sst, sfield;
	sst=st;
	sfield=field;
	st=(st & 0xF800) | (code << 8);
	field=0x0000;
	/* Same effect as JMS */
	MEM(0x000000)=orig_address;
	PC=0x1;
	return;
}
