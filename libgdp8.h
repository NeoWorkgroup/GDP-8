/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Neo_Chen: This will be the most useless Virtual Machine ever!
 * ENCODING: UTF-8
 * LANGUAGE: zh_TW, en_US
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#define AC(x) ac[x].word
#define MQ mq.word
#define STATUS status.word
#define MEM(x) memory[x].word
#define L l
#define PC pc
#define TO_ADDRESS(x) \
	(x % (1024*1024))

#define INDIRECT(x) \
	(memory[x % (1024*1024)].word)

#define POWTWO(exp) \
	(1 << exp)

/* 暫存器：
 * AC:	累加器(Accumulator)， 也被 PSH, POP, CALL, RET 當作是 Stack Pointer
 * MQ:	倍數/商數(Multiplier Quotient)
 * PC:	程式計數器(Program Counter)
 * L:	The Link
 * STATUS:	狀態
 *
 * Description of Instructions:
 * 
 * Group 0, Data Multiplication, with Accumulator:
 * 	OPR:	OPeRator, has many function
 * 	AND:	logic AND
 * 	OR:	logic OR
 * 	LOAD:	LOAD a word into accumulator
 * 	DEP:	DEPosit a word into memory
 *	PSH:	PUSH a word into Stack
 *	POP:	POP a word off Stack
 * Group 1, Data Multiplication, without Accumulator:
 * 	LLOAD:	LOAD the lower 16 bits into the Link
 *	LADD:	ADD the word into the Link
 *	LDEP:	DEPOSIT the Link into the word (upper 16 bits is unchanged)
 * Group 2, Arithmetic Operation (Integer):
 * 	ADD:	ADD a word into accumulator
 * 	SUB:	SUBtract a word from accumulator
 * 	MUL:	accumulator *= word
 * 	DVI:	accumulator /= word
 * 	POW:	accumulator=pow(accumulator, word)
 * 	MOD:	accumulator = accumulator % word
 * 	SQRT:	accumulator=square_root(accumulator)
 * 	LRTS:	Logical Right Shift
 * 	ARTS:	Arithmetic Right Shift
 *	NMI:	Normalize
 * Group 3, Flow Control:
 * 	JMP:	JUMP to the Address
 * 	JMS:	JUMP to the Subroutine
 * 	CMP:	Compare the Word and Accumulator, if equal, L=0xFFFF, if not, L=0x0000
 * 	CJMP:	Conditional JUMP
 * 	CJMS:	Conditional JUMP to Subroutine
 *	CSKIP:	Conditional Skip Next Instruction
 *	CALL:	Push PC into Stack, and jump to the address
 *	RET:	Pop PC from Stack, Push the word, and jump to the saved PC
 * Group 4, I/O:
 * 	IN:	Read from port <L>, and save it into the address
 * 	OUT:	Write the word to port <L>
 * Group 5, Special:
 * 	EUM:	Enter Usermode at the address
 * 	HLT:	Halt, will resume at the address
 * 	HCF:	Halt, and Catch Fire -- there's no way to resume
 * 	BUG:	Tell the VM that we are toasted at the address
 */

enum OpCode
{
/* Group 0, Data Multiplication, with Accumulator */
	OPR, AND, OR, XOR,
	LOAD, DEP, PSH, POP,
/* Group 1, Data Multiplication, without Accumulator */
	LLOAD, LADD, LDEP,
/* Group 2, Arithmetic Operation (Integer), with Accumulator */
	ADD, SUB, MUL, DVI,
	POW, MOD, SQRT, LRTS,
	ARTS, NMI,
/* Group 3, Flow Control, without Accumulator */
	JMP, JMS, CMP, CJMP,
	CJMS, CSKIP, CALL, RET,
/* Group 4, I/O, with Accumulator, or with the Link */
	IN, OUT, LIN, LOUT,
/* Group 5, Special, without Accumulator */
	EUM, HLT, HCF, BUG=0xFF
};

/* 指令格式 */
typedef struct
{
	uint32_t opcode	:8;	/* OpCode Number */
	uint32_t accumulator	:3;	/* Which Accumulator to Use */
	uint32_t indirect	:1;	/* Indirect Reference */
	uint32_t address	:20;	/* Address */
} ac_instruction_t;

typedef struct
{
	uint32_t opcode	:8;	/* OpCode Number */
	uint32_t order		:1;	/* Whether Increment before Indirect or not */
	uint32_t increment	:1;	/* Increment after reference */
	uint32_t decrement	:1;	/* Decrement after reference */
	uint32_t indirect	:1;	/* Indirect Reference */
	uint32_t addr		:20;	/* Address */
} instruction_t;

/* 可能很常用的 OPR */
typedef struct
{
	uint32_t opcode	:8;	/* Must be OPR */
	uint32_t indirect	:1;	/* Indirect Reference (From Accumulator) */
	uint32_t accumulator	:3;	/* Which Accumulator to Use */
	uint32_t clear	:1;	/* Clear */
	uint32_t clear_link	:1;	/* Clear the Link */
	uint32_t clear_mq	:1;	/* Clear the MQ */
	uint32_t move_mq	:1;	/* Move the Word into MQ */
	uint32_t swap_mq	:1;	/* Swap the Word and MQ */
	uint32_t swap_link	:1;	/* Swap the Link and the lower half of the Word */
	uint32_t rotr	:1;	/* Rotate the Word Right */
	uint32_t rotl	:1;	/* Rotate the Word Left */
	uint32_t rott	:1;	/* Rotate 2 Bits, if both rotr and rotl isn't set, swap lower and upper 16Bit of the Word */
	uint32_t increment	:1;	/* Increment the Word, if both increment and decrement is set, increment the Link */
	uint32_t decrement	:1;	/* Decrement the Word */
	uint32_t swap_lower	:1;	/* Swap the lower 16 Bit of the Word and the Link */
	uint32_t swap_upper	:1;	/* Swap the upper 16 Bit of the Word and the Link */
	uint32_t reverse_bits	:1;	/* Reverse the bits of the Word */
	uint32_t reverse_link_bits	:1;	/* Reverse the bits of the Link */
	uint32_t reverse	:1;	/* Reverse the effects of condition */
	uint32_t if_non_zero	:1;	/* Skip if the Word is 0x00000000 */
	uint32_t if_link_non_zero	:1;	/* Skip if the Link is 0x0000 */
	uint32_t if_negative	:1;	/* Skip if the Word is negative */
	uint32_t buttons	:1;	/* Store the content of Panel Buttons into the Word */
} opr_instruction_t;

/* I/O 指令格式 */
typedef struct
{
	uint32_t opcode	:8;	/* Either IN or OUT */
	uint32_t indirect	:1;	/* Indirect Reference (From Accumulator[0]) */
	uint32_t flags	:3;	/* Yes, it is "Flags" */
	uint32_t address	:20;	/* Data Address */
} io_instruction_t;

/* 狀態格式
 * Status Code:
 * 	0:	No Error
 * 	1:	System Call
 * 	2:	EUM, HCF, BUG, IN, OUT Executed in Usermode (Trap)
 * 	3:	Device Error
 * 	4:	Illegal Instruction
 * 	5:	BUG Triggered in Usermode
 * 	6:	HLT in Usermode (Ends the program)
 * 	7:	WE ARE TOASTED!! (BUG in Kernel Space: KERNEL PANIC)
 */

#define ENOP	00
#define ESCALL	01
#define ETRAP	02
#define EDEV	03
#define EINST	04
#define EBUG	05
#define EHLT	06
#define EPANIC	07

typedef struct
{
	uint32_t crt	:1;	/* Compare Return / Greater Than */
	uint32_t intr	:1;	/* Encountered Interrupt */
	uint32_t suf	:6;	/* Saved User Flag */
	uint32_t um	:1;	/* In Usermode */
	uint32_t stat	:3;	/* Status Code */
	uint32_t addr	:20;	/* Current Address */
} status_t;

typedef union
{
	ac_instruction_t ac_inst;
	instruction_t inst;
	opr_instruction_t opr_inst;
	io_instruction_t io_inst;
	status_t status;
	uint32_t word;
} word_u;

/* Corefile Format:
 * "01234:56789ABC"
 * 20 Bit : 32 Bit Hexdecimal
 * Any Invaild Input will be ignored
 * TODO: Make it support Comment
 */

unsigned int read_core(FILE *fp)
{
	uint32_t word=0;
	uint32_t addr=0;
	int readed=0;
	while(fscanf(fp, "%x:%x\n", &addr, &word) != EOF)
	{
		memory[addr].word=word;
		readed++;
	}
	return readed;
}

uint32_t rotl(uint32_t word, uint8_t count)
{
	return (word << count) | (word >> (32 - count));
}

uint32_t rotr(uint32_t word, uint8_t count)
{
	return (word >> count)|(word << (32 - count));
}

void interrupt(uint8_t scode, uint8_t umflags, uint32_t addr)
{
	save_status=status;
	MEM(0x00000)=PC;
	PC=0x1;
}
