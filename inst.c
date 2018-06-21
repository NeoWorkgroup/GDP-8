/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgdp8.h>

extern word_t ac[4], mq, st, field, sst, sfield, *memory, pc;
extern uint8_t sc;

/* Normal Rotate */
uint16_t rotl(uint16_t word, uint8_t count)
{
	return (word << count) | (word >> (16 - count));
}

uint16_t rotr(uint16_t word, uint8_t count)
{
	return (word >> count)|(word << (16 - count));
}

/* Rotate L with WORD */
void lwrotr(uint16_t *word, uint16_t *status)
{
	uint16_t temp=*word;
	*word=((temp >> 1) | (*status & 0x8000));
	*status=((*status & 0x7FFF) | ((temp & 0x0001) << 15));
	return;
}

void lwrotl(uint16_t *word, uint16_t *status)
{
	uint16_t temp=*word;
	*word=((temp << 1) | (*status >> 15));
	*status=((*status & 0x7FFF) | ((temp & 0x8000) << 15));
	return;
}

/* 0x0:AND */
void inst_and(word_t word)
{
	if(I_MASK(word))
		AC(AC_MASK(word)) &= MEM(DADDR(field, MEM(CADDR(field, PADDR(PC, ADDR_MASK(word))))));
	else
		AC(AC_MASK(word)) &= MEM(CADDR(field, PADDR(PC, ADDR_MASK(word))));
}

/* 0x1:ADD */
void inst_add(word_t word)
{
	if(I_MASK(word))
		AC(AC_MASK(word)) += MEM(DADDR(field, MEM(CADDR(field, PADDR(PC, ADDR_MASK(word))))));
	else
		AC(AC_MASK(word)) += MEM(CADDR(field, PADDR(PC, ADDR_MASK(word))));
}

/* 0x2:ISZ */
void inst_isz(word_t word)
{
	if(I_MASK(word))
	{
		if(++MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))) == 0)
			PC++;
	}
	else
	{
		if(++MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))) == 0)
			PC++;
	}
}

/* 0x3:DEP */
void inst_dep(word_t word)
{
	if(I_MASK(word))
		MEM(DADDR(field, MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))))) = AC(AC_MASK(word));
	else
		MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))) = AC(AC_MASK(word));
	AC(AC_MASK(word)) = 0x0000;
}

/* 0x4:JMS */
void inst_jms(word_t word)
{
	if(I_MASK(word))
	{
		/* Cross Field JMS */
		MEM(DADDR(field, MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))))) = PC;
		PC = MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))) + 1;
		field = ((field & 0x00FF) << 16) | (field & 0xFF); /* DATA FIELD to CODE FIELD */
	}
	else
	{
		MEM(CADDR(field, PADDR(PC, ADDR_MASK(word)))) = PC;
		PC = PADDR(PC, ADDR_MASK(word)) + 1;
	}
}

/* 0x5:JMP */
void inst_jmp(word_t word)
{
	if(I_MASK(word))
	{
		/* Cross Field JMP */
		PC = MEM(CADDR(field, PADDR(PC, ADDR_MASK(word))));
		field = ((field & 0x00FF) << 16) | (field & 0xFF); /* DATA FIELD to CODE FIELD */
	}
	else
	{
		PC = PADDR(PC, ADDR_MASK(word));
	}
}

/* 0x6:IOT */
void inst_iot(word_t word)
{
	/* Stub */
}

/* 0x7:OPR */
void inst_opr(word_t word)
{
	/* Stub */
}

/* 0x8:PSH */
void inst_psh(word_t word)
{
	if(I_MASK(word))
		MEM(CADDR(field, AC(AC_MASK(word)))) = MEM(DADDR(field, MEM(CADDR(field, ADDR_MASK(word)))));
	else
		MEM(CADDR(field, AC(AC_MASK(word)))) = MEM(CADDR(field, ADDR_MASK(word)));
	AC(AC_MASK(word))--;
}

/* 0x9:POP */
void inst_pop(word_t word)
{
	AC(AC_MASK(word))++;
	if(I_MASK(word))
		IMEM(field, PC, word) = MEM(CADDR(field, AC(AC_MASK(word))));
	else
		DMEM(field, PC, word) = MEM(CADDR(field, AC(AC_MASK(word))));
	MEM(CADDR(field, AC(AC_MASK(word)))) = 0x0000;
}

/* 0xA:CAL */
void inst_cal(word_t word)
{
	MEM(CADDR(field, AC(AC_MASK(word))--)) = PC;
	MEM(CADDR(field, AC(AC_MASK(word))--)) = field;
	if(I_MASK(word))
	{
		PC = IMEM(field, PC, word);
		field = ((field & 0x00FF) << 16) | (field & 0xFF);
	}
	else
		PC = DMEM(field, PC, word);
}

/* 0xB:RET */
/* TODO: Make the Address argument useful */
void inst_ret(word_t word)
{
	if(I_MASK(word))
	{
		PC = MEM(DADDR(field, MEM(CADDR(field, ++AC(AC_MASK(word))))));
		MEM(DADDR(field, MEM(CADDR(field, AC(AC_MASK(word)))))) = 0x0000;
		field = MEM(DADDR(field, MEM(CADDR(field, ++AC(AC_MASK(word))))));
		MEM(DADDR(field, MEM(CADDR(field, AC(AC_MASK(word)))))) = 0x0000;
	}
	else
	{
		PC = MEM(CADDR(field, ++AC(AC_MASK(word))));
		MEM(CADDR(field, AC(AC_MASK(word)))) = 0x0000;
	}
}

/* 0xC:EUM */
void inst_eum(word_t word)
{
	/* Set the UM flag in status */
	status |= 0x0800;

}

/* 0xD:INT */
void inst_int(word_t word)
{
}

/* 0xE:SYS */
void inst_sys(word_t word)
{
}

/* 0xF:STP */
void inst_stp(word_t word)
{
}
