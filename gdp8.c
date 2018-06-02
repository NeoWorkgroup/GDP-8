/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Neo_Chen: This will be the most useless Virtual Machine ever!
 * ENCODING: UTF-8
 * LANGUAGE: zh_TW, en_US
 */

/* 有沒有從命名看出我在向什麼公司的什麼電腦致敬？ */

/* 大致架構：
 * 多累加器，32 Bit 機器字，20 Bit 定址
 * 中斷，指令權限區分，無虛擬記憶體
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#define POWTWO(exp) \
	(1 << exp)

/* 暫存器：
 * AC:	累加器(Accumulator)， 也被 PSH, POP, CALL, RET 當作是 Stack Pointer
 * MQ:	倍數/商數(Multiplier Quotient)
 * PC:	程式計數器(Program Counter)
 * L:	The Link
 * STATUS:	狀態
 */

/* Description of Instructions:
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
 * 	BUG:	Tell the VM that we are toasted
 */

enum OpCode
{
/* Group 0, Data Multiplication, with Accumulator */
	OPR=0x00, AND=0x01, OR=0x02, XOR=0x03,
	LOAD=0x04, DEP=0x05, PSH=0x06, POP=0x07,
/* Group 1, Data Multiplication, without Accumulator */
	LLOAD=0x08, LADD=0x09, LDEP=0x0A,
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

typedef union
{
	ac_instruction_t ac_inst;
	instruction_t inst;
	opr_instruction_t opr_inst;
	io_instruction_t io_inst;
	uint32_t word;
} word_u;

/* 32 Bit, Word Size */
word_u ac[8], mq;
#define AC(x) ac[x].word
#define MQ mq.word
status_t status, saved_status;
#define STATUS status.word
word_u *memory;
#define MEM(x) memory[x].word
/* 16 Bit, Special Register*/
uint16_t l=0;
#define L l
/* 20 Bit, Memory Addressing */
uint32_t pc=0;
#define PC pc


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
	return word%1024*1024;
}

#define TO_ADDRESS(x) \
	(x % (1024*1024))

#define INDIRECT(x) \
	(memory[x % (1024*1024)].word)

uint32_t rotl(uint32_t word, uint8_t count)
{
	return (word << count) | (word >> (32 - count));
}

uint32_t rotr(uint32_t word, uint8_t count)
{
	return (word >> count)|(word << (32 - count));
}

uint16_t lrotl(uint32_t word, uint8_t count)
{
	return (word << count) | (word >> (16 - count));
}

uint16_t lrotr(uint32_t word, uint8_t count)
{
	return (word << count) | (word >> (16 - count));
}

void interrupt(void)
{
	MEM(0x00000)=PC;
	PC=0x1;
}

void opr_word_increment(word_u code, short int that_ac)
{
	if(code.opr_inst.increment&&code.opr_inst.decrement)
		l++;
	else if(code.opr_inst.indirect)
	{
		if(code.opr_inst.increment)
			MEM(TO_ADDRESS(AC(that_ac)))++;
		if(code.opr_inst.decrement)
			MEM(TO_ADDRESS(AC(that_ac)))--;
	}
	else
	{
		if(code.opr_inst.increment)
			AC(that_ac)++;
		if(code.opr_inst.decrement)
			AC(that_ac)--;
	}
}

void opr_bit_rotate(word_u code, short int that_ac)
{
	if(code.opr_inst.rotl)
	{
		if(code.opr_inst.indirect)
			MEM(TO_ADDRESS(AC(that_ac))) =
				rotl(MEM(to_address(AC(that_ac))), code.opr_inst.rott? 2: 1);
		else
			AC(code.opr_inst.accumulator) =
				rotl(AC(that_ac), code.opr_inst.rott? 2: 1);
	}
	else if(code.opr_inst.rotr)
	{
		if(code.opr_inst.indirect)
			MEM(TO_ADDRESS(AC(that_ac))) =
				rotr(MEM(TO_ADDRESS(AC(that_ac))), code.opr_inst.rott? 2: 1);
		else
			AC(that_ac) =
				rotr(AC(that_ac), code.opr_inst.rott? 2: 1);
	}
	if(code.opr_inst.rott && (! code.opr_inst.rotr) && (! code.opr_inst.rotl))
	{
		if(code.opr_inst.indirect)
			MEM(TO_ADDRESS(AC(that_ac))) =
				rotl(MEM(TO_ADDRESS(AC(that_ac))), 16);
		else
			AC(that_ac) = rotl(AC(that_ac), 16);
	}
}

void opr_clear(word_u code, short int that_ac)
{
	if(code.opr_inst.clear)
	{
		if(code.opr_inst.indirect)
			MEM(TO_ADDRESS(AC(that_ac)))=0;
		else
			AC(that_ac)=0;
	}
	if(code.opr_inst.clear_link)
		L=0;
	if(code.opr_inst.clear_mq)
		MQ=0;
}

void opr_move_swap(word_u code, short int that_ac)
{
	uint32_t temp=0;
	if(code.opr_inst.move_mq)
	{
		MQ=AC(that_ac);
		AC(that_ac)=0;
	}
	if(code.opr_inst.swap_mq)
	{
		temp=AC(that_ac);
		AC(that_ac)=MQ;
		MQ=temp;
	}
	if(code.opr_inst.swap_link)
	{
		if(code.opr_inst.indirect)
		{
			temp=L;
			L=(uint16_t) ((MEM(TO_ADDRESS(AC(that_ac)))) & 0x0000FFFF);
			MEM(TO_ADDRESS(AC(that_ac)))= (MEM(TO_ADDRESS(AC(that_ac))) & 0xFFFF0000) + temp;
		}
		else
		{
			temp=L;
			L=AC(that_ac) & 0x0000FFFF;
			AC(that_ac)= (AC(that_ac) & 0xFFFF0000) + temp;
		}
	}
}

void opr_swap_link(word_u code, short int that_ac)
{
	uint32_t temp=0;
	uint16_t half_temp=0;
	if(code.opr_inst.swap_upper)
	{
		if(code.opr_inst.indirect)
		{
			temp = MEM(TO_ADDRESS(AC(that_ac))) & 0xFFFF0000;
			MEM(TO_ADDRESS(AC(that_ac))) = temp + L;
			L = temp >> 16;
		}
		else
		{
			temp = AC(that_ac) & 0xFFFF0000;
			AC(that_ac) = temp + L;
			L = temp >> 16;
		}
	}
	if(code.opr_inst.swap_lower)
	{
		if(code.opr_inst.indirect)
		{
			half_temp = (uint16_t) (MEM(TO_ADDRESS(AC(that_ac))) & 0x0000FFFF);
			MEM(TO_ADDRESS(AC(that_ac))) &= 0xFFFF0000;
			MEM(TO_ADDRESS(AC(that_ac))) += L;
			L = half_temp;
		}
		else
		{
			half_temp = (uint16_t) (AC(that_ac) & 0x0000FFFF);
			AC(that_ac) &= 0xFFFF0000;
			AC(that_ac) += L;
			L = half_temp;
		}
	}
}

void opr_reverse(word_u code, short int that_ac)
{
	if(code.opr_inst.reverse_bits)
	{
		if(code.opr_inst.indirect)
			MEM(TO_ADDRESS(AC(that_ac))) ^= 0x0;
		else
			AC(that_ac) ^= 0x0;
	}
	if(code.opr_inst.reverse_link_bits)
		L ^= 0x0;
}

void opr_skip(word_u code, short int that_ac)
{
	if(code.opr_inst.reverse)
	{
		if(code.opr_inst.if_non_zero)
		{
			if(code.opr_inst.indirect)
				if(MEM(TO_ADDRESS(AC(that_ac))) == 0)
					++PC;
			else
				if(AC(that_ac) == 0)
					++PC;
		}
		if(code.opr_inst.if_link_non_zero)
		{
			if(L == 0)
				++PC;
		}
		if(code.opr_inst.if_negative)
		{
			if(code.opr_inst.indirect)
				if((MEM(TO_ADDRESS(AC(ac_that))) & 0x80000000) == 0)
					++PC;
			else
				if((AC(ac_that) & 0x80000000) == 0)
					++PC;
		}
	}
	else
	{
		if(code.opr_inst.if_non_zero)
		{
			if(code.opr_inst.indirect)
				if(MEM(TO_ADDRESS(AC(that_ac))) != 0)
					++PC;
			else
				if(AC(that_ac) != 0)
					++PC;
		}
		if(code.opr_inst.if_link_non_zero)
			if(L)
				++PC;
		if(code.opr_inst.if_negative)
		{
			if(code.opr_inst.indirect)
				 if((MEM(TO_ADDRESS(AC(ac_that))) & 0x80000000) != 0)
					++PC;
			else
				if((AC(ac_that) & 0x80000000) != 0)
					++PC;
		}
	}
}

void interpret(word_u code)
{
	if(code.inst.opcode == OPR)
	{
		opr_clear(code, code.opr_inst.accumulator);
		opr_move_swap(code, code.opr_inst.accumulator);
		opr_bit_rotate(code, code.opr_inst.accumulator);
		opr_word_increment(code, code.opr_inst.accumulator);
		opr_swap_link(code, code.opr_inst.accumulator);
		opr_reverse(code, code.opr_inst.accumulator);
		opr_skip(code, code.opr_inst.accumulator);
		if(code.opr_inst.buttons) /* Currently Does nothing */
			AC(code.opr_inst.accumulator)=0;
	return;
	}
	switch(code.inst.opcode)
	{
		case AND:
		case OR:
		case XOR:
		case LOAD:
		case DEP:
		case PSH:
		case POP:
		default:
			return;
	}
}

int main (int argc, char **argv)
{
	memory=calloc(1024*1024, 4);
	FILE *corefile;
	int opt;
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

