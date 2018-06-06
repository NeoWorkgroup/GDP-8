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
#include <libgdp8.h>

/* 32 Bit, Word Size */
word_u ac[8], mq;
status_t status, saved_status;
word_u *memory;
/* 16 Bit, Special Register*/
uint16_t l=0;
/* 20 Bit, Memory Addressing */
uint32_t pc=0;


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
				rotl(MEM(TO_ADDRESS(AC(that_ac))), code.opr_inst.rott? 2: 1);
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
			MEM(TO_ADDRESS(AC(that_ac))) ^= 0xFFFFFFFF;
		else
			AC(that_ac) ^= 0xFFFFFFFF;
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
			{
				if(MEM(TO_ADDRESS(AC(that_ac))) == 0)
					++PC;
			}
			else
			{
				if(AC(that_ac) == 0)
					++PC;
			}
		}
		if(code.opr_inst.if_link_non_zero)
		{
			if(L == 0)
				++PC;
		}
		if(code.opr_inst.if_negative)
		{
			if(code.opr_inst.indirect)
			{
				if((MEM(TO_ADDRESS(AC(that_ac))) & 0x80000000) == 0)
					++PC;
			}
			else
			{
				if((AC(that_ac) & 0x80000000) == 0)
					++PC;
			}
		}
	}
	else
	{
		if(code.opr_inst.if_non_zero)
		{
			if(code.opr_inst.indirect)
			{
				if(MEM(TO_ADDRESS(AC(that_ac))) != 0)
					++PC;
			}
			else
			{
				if(AC(that_ac) != 0)
					++PC;
			}
		}
		if(code.opr_inst.if_link_non_zero)
		{
			if(L)
				++PC;
		}
		if(code.opr_inst.if_negative)
		{
			if(code.opr_inst.indirect)
			{
				if((MEM(TO_ADDRESS(AC(that_ac))) & 0x80000000) != 0) /* The highest bit */
					++PC;
			}
			else
			{
				if((AC(that_ac) & 0x80000000) != 0) /* Same as above */
					++PC;
			}
		}
	}
}

void and(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		AC(that_ac) &= MEM(TO_ADDRESS(MEM(that_mem)));
	else
		AC(that_ac) &= MEM(that_mem);
}

void or(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		AC(that_ac) |= MEM(TO_ADDRESS(MEM(that_mem)));
	else
		AC(that_ac) |= MEM(TO_ADDRESS(that_mem));
}

void xor(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		AC(that_ac) ^= MEM(TO_ADDRESS(MEM(that_mem)));
	else
		AC(that_ac) ^= MEM(TO_ADDRESS(that_mem));
}

void load(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		AC(that_ac) = MEM(TO_ADDRESS(MEM(that_mem)));
	else
		AC(that_ac) = MEM(that_mem);
}

void dep(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		MEM(TO_ADDRESS(MEM(that_mem))) = AC(that_ac);
	else
		MEM(that_mem) = AC(that_ac);
}

void psh(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		AC(that_ac) = MEM(TO_ADDRESS(MEM(that_mem)));
	else
		AC(that_ac) = MEM(that_mem);
	AC(that_ac)--;
}

void pop(short int that_ac, int indirect, uint32_t that_mem)
{
	if(indirect)
		MEM(TO_ADDRESS(MEM(that_mem))) = AC(that_ac)++;
	else
		MEM(that_mem) = AC(that_ac)++;
}

void lload(int indirect, uint32_t that_mem)
{
	if(indirect)
		L=(uint16_t) MEM(TO_ADDRESS(MEM(that_mem))) & 0x0000FFFF;
	else
		L=(uint16_t) MEM(that_mem) & 0x0000FFFF;
}

void ladd(int indirect, uint32_t that_mem)
{
	if(indirect)
		L+=(uint16_t) MEM(TO_ADDRESS(MEM(that_mem))) & 0x0000FFFF;
	else
		L+=(uint16_t) MEM(that_mem) & 0x0000FFFF;
}

void ldep(int indirect, uint32_t that_mem)
{
	if(indirect)
		MEM(TO_ADDRESS(MEM(that_mem)))=(uint32_t)L;
	else
		MEM(thar_mem)=(uint32_t)L;
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
	/* Group 0 */
		case AND:
			and(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case OR:
			or(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case XOR:
			xor(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case LOAD:
			load(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case DEP:
			dep(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case PSH:
			psh(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case POP:
			pop(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
	/* Group 1 */
		case LLOAD:
			lload(code.inst.indirect, code.inst.addr);
			break;
		case LADD:
			ladd(code.inst.indirect, code.inst.addr);
			break;
		case LDEP:
			ldep(code.inst.indirect, code.inst.addr);
			break;
	/* Group 2 */
		case ADD:
			add(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case SUB:
			sub(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case MUL:
			mul(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case DVI:
			dvi(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case MOD:
			mod(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case SQRT:
			sqrt(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case LRTS:
			lrtr(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case ARTS:
			artr(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
		case NMI:
			nmi(code.ac_inst.accumulator, code.inst.indirect, code.inst.addr);
			break;
	/* Group 3 */
		case JMP:
			jmp(code.inst.flags, code.inst.indirect, code.inst.addr);
	/* Group 4 */
	
		default:
			return;
	}
}

int main (int argc, char **argv)
{
	memory=calloc(POWTWO(20), 4);
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

