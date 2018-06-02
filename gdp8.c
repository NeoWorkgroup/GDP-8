/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Neo_Chen: This will be the most useless Virtual Machine ever!
 * ENCODING: UTF-8
 * LANGUAGE: zh_TW, en_US
 */

/* 有沒有從命名看出我在向什麼公司的什麼電腦致敬？ */

/* 發展方向：
 * 多累加器，32 Bit 機器字，20 Bit 定址
 * 中斷，指令權限區分
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#ifdef DEBUG
#  define DEBUG_OUTPUT \
	fprintf(stderr, "PC=%x AC0=%x AC1=%x AC2=%x AC3=%x AC4=%x AC5=%x AC6=%x AC7=%x MQ=%x L=%hx\nST=%hx\n" \
			,   pc, ac[0], ac[1], ac[2], ac[3], ac[4], ac[5], ac[6], ac[7],   mq,  link,   status);
#else
#  define DEBUG_OUTPUT
#endif

#ifndef MEMSIZE
#  define MEMSIZE 1024*1024*4
#endif

#define POWTWO(exp) \
	(1 << exp)

/* 暫存器：
 * AC:	累加器(Accumulator)， 也被 PSH, POP, CALL, RET 當作是 Stack Pointer
 * MQ:	倍數/商數(Multiplier Quotient)
 * PC:	程式計數器(Program Counter)
 * L:	The Link
 * STATUS:	狀態
 */

/* Group 0, Data Multiplication, with Accumulator:
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
 * 	BUG:	Tell the VM that we are toasted
 */
enum OpCode
{
/* Group 0, Data Multiplication, with Accumulator */
	OPR=0x00, AND=0x01, OR=0x02, XOR=0x03,
	LOAD=0x04, DEP=0x05, PSH=0x06, POP=0x07,
/* Group 1, Data Multiplication, without Accumulator */
	LLOAD=0x08, LADD=0x09, LDEP=0x0A
/* Group 2, Arithmetic Operation (Integer) */
	ADD=0x0B, SUB=0x0C, MUL=0x0D, DVI=0x0E,
	POW=0x0F, MOD=0x10, SQRT=0x11, LRTS=0x12,
	ARTS=0x13, NMI=0x14,
/* Group 3, Flow Control */
	JMP=0x15, JMS=0x16, CMP=0x17, CJMP=0x18,
	CJMS=0x19, CSKIP=0x1A, CALL=0x1B, RET=0x1C,
/* Group 4, I/O */
	IN=0x1D, OUT=0x1E,
/* Group 5, Special */
	EUM=0x1F, HLT=0x20, HCF=0x21, BUG=0xFF
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
	uint32_t increment	:1;	/* Increment after reference */
	uint32_t decrement	:1;	/* Decrement after reference */
	uint32_t order		:1;	/* Whether Increment before Indirect or not */
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
	uint32_t flags	:3;
	uint32_t address	:20;	/* Data Address */
} io_instruction_t;

/* 狀態格式
 * Status Code:
 * 	0:	No Error
 * 	1:	System Call
 * 	2:	HLT, HCF, BUG, IN, OUT Executed in Usermode
 * 	3:	Device Error
 * 	4:	Illegal Instruction
 */

typedef struct
{
	uint32_t gt	:1;	/* Greater Than */
	uint32_t intr	:1;	/* Encountered Interrupt */
	uint32_t suf	:6;	/* Saved User Flag */
	uint32_t um	:1;	/* In Usermode */
	uint32_t stat	:3;	/* Status Code */
	uint32_t addr	:20;	/* Current Address */
} status_t;

typedef struct
{
	uint32_t upper	:16;
	uint32_t lower	:16;
} half_t;

typedef union
{
	ac_instruction_t ac_inst;
	instruction_t inst;
	opr_instruction_t opr_inst;
	io_instruction_t io_inst;
	half_t half;
	uint32_t word;
} word_u;

/* 32 Bit, Word Size */
word_u ac[8], mq;
status_t status, saved_status;
word_u *memory;
/* 16 Bit, Special Register*/
uint16_t l=0;
/* 20 Bit, Memory Addressing */
uint32_t pc=0;


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

uint32_t to_address(uint32_t word)
{
	return word%MEMSIZE;
}

uint32_t indirect(uint32_t word)
{
	return memory[word%MEMSIZE].word;
}

uint32_t rotl(uint32_t word, uint8_t count)
{
	return (word << count) | (word >> (32 - count));
}

uint32_t rotr(uint32_t word, uint8_t count)
{
	return (word >> count)|(word << (32 - count));
}

void interrupt(void)
{
	memory[0x00000].word=pc;
	pc=0x1;
}

void interpret(word_u code)
{
	uint32_t temp=0x00000000;
	uint16_t half_temp=0x0000;
	if(code.inst.opcode == OPR)
	{
		if(code.opr_inst.clear)
		{
			if(code.opr_inst.indirect)
				memory[ac[code.opr_inst.accumulator].word%MEMSIZE].word=0;
			else
				ac[code.opr_inst.accumulator].word=0;
		}
		if(code.opr_inst.clear_link)
			l=0;
		if(code.opr_inst.clear_mq)
			mq.word=0;
		if(code.opr_inst.move_mq)
		{
			ac[code.opr_inst.accumulator].word=0;
			mq.word=0;
		}
		if(code.opr_inst.swap_mq)
		{
			temp=ac[code.opr_inst.accumulator].word;
			ac[code.opr_inst.accumulator].word=mq.word;
			mq.word=temp;
		}
		if(code.opr_inst.swap_link)
		{
			if(code.opr_inst.indirect)
			{
				temp=l;
				l=(memory[to_address(ac[code.opr_inst.accumulator].word)].word)%POWTWO(16);
				memory[ac[code.opr_inst.accumulator].word%MEMSIZE].half.lower=temp;
			}
			else
			{
				temp=l;
				l=ac[code.opr_inst.accumulator].half.lower;
				ac[code.opr_inst.accumulator].half.lower=temp;
			}
		}
		else if(code.opr_inst.rotl)
		{
			if(code.opr_inst.indirect)
				memory[to_address(ac[code.opr_inst.accumulator].word)].word =
					rotl(memory[to_address(ac[code.opr_inst.accumulator].word)].word, code.opr_inst.rott? 2: 1);
			else
				ac[code.opr_inst.accumulator].word =
					rotl(ac[code.opr_inst.accumulator].word, code.opr_inst.rott? 2: 1);
		}
		else if(code.opr_inst.rotr)
		{
			if(code.opr_inst.indirect)
				memory[to_address(ac[code.opr_inst.accumulator].word)].word =
					rotr(memory[to_address(ac[code.opr_inst.accumulator].word)].word, code.opr_inst.rott? 2: 1);
			else
				ac[code.opr_inst.accumulator].word =
					rotr(ac[code.opr_inst.accumulator].word, code.opr_inst.rott? 2: 1);
		}
		if(code.opr_inst.rott && (! code.opr_inst.rotr) && (! code.opr_inst.rotl))
		{
			if(code.opr_inst.indirect)
				memory[to_address(ac[code.opr_inst.accumulator].word)].word =
					rotl(memory[to_address(ac[code.opr_inst.accumulator].word)].word, 16);
			else
				ac[code.opr_inst.accumulator].word = rotl(ac[code.opr_inst.accumulator].word, 16);
		}
		if(code.opr_inst.increment&&code.opr_inst.decrement)
			l++;
		else if(code.opr_inst.indirect)
		{
			if(code.opr_inst.increment)
				memory[to_address(ac[code.opr_inst.accumulator].word)].word++;
			if(code.opr_inst.decrement)
				memory[to_address(ac[code.opr_inst.accumulator].word)].word--;
		}
		else
		{
			if(code.opr_inst.increment)
				ac[code.opr_inst.accumulator].word++;
			if(code.opr_inst.decrement)
				ac[code.opr_inst.accumulator].word--;
		}

	}

}

int main (int argc, char **argv)
{
	word_u *memory=calloc(1024*1024, 4);
	FILE *corefile;
	int opt;
	memory=malloc(MEMSIZE); /* 20 Bit Addressing, 4 Bytes per Word */
	while((opt = getopt(argc, argv, "hf:s:")) != -1)
	{
		switch(opt)
		{
			case 's':
				sscanf(optarg, "%x", &pc);
				break;
			case 'f':
				if((corefile = fopen(optarg, "r")) == NULL)
				{
					perror(argv[0]);
					exit(8);
				}
				read_core(corefile);
				break;
			case 'h':
				printf("Usage: %s [-h] [-f file] [-s address]\n", argv[0]);
				break;
			default:
				fprintf(stderr,"%s: %s\n", argv[0], optarg);
				exit(0);
		}
	}
	return 0;
}

