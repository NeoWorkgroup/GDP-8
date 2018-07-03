/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* inst.c: Instruction implement */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
		AC(AC_MASK(word)) &= IMEM(FIELD, PC, word);
	else
		AC(AC_MASK(word)) &= DMEM(FIELD, PC, word);
}

/* 0x1:ADD */
void inst_add(word_t word)
{
	if(I_MASK(word))
		AC(AC_MASK(word)) += IMEM(FIELD, PC, word);
	else
		AC(AC_MASK(word)) += DMEM(FIELD, PC, word);
}

/* 0x2:ISZ */
void inst_isz(word_t word)
{
	if(I_MASK(word))
	{
		if(++IMEM(FIELD, PC, word) == 0)
			PC++;
	}
	else
	{
		if(++DMEM(FIELD, PC, word) == 0)
			PC++;
	}
}

/* 0x3:DEP */
void inst_dep(word_t word)
{
	if(I_MASK(word))
		IMEM(FIELD, PC, word) = AC(AC_MASK(word));
	else
		DMEM(FIELD, PC, word) = AC(AC_MASK(word));
	AC(AC_MASK(word)) = 0x0000;
}

/* 0x4:JMS */
void inst_jms(word_t word)
{
	if(I_MASK(word))
	{
		/* Cross FIELD JMS */
		IMEM(FIELD, PC, word) = PC;
		PC = IADDR(FIELD, PC, word) + 1;
		FIELD = ((FIELD & 0x00FF) << 16) | (FIELD & 0xFF); /* DATA FIELD to CODE FIELD */
	}
	else
	{
		DMEM(FIELD, PC, word) = PC;
		PC = PAGEADDR(PC, ADDR_MASK(word)) + 1;
	}
}

/* 0x5:JMP */
void inst_jmp(word_t word)
{
	if(I_MASK(word))
	{
		/* Cross FIELD JMP */
		PC = IADDR(FIELD, PC, word);
		FIELD = ((FIELD & 0x00FF) << 16) | (FIELD & 0xFF); /* DATA FIELD to CODE FIELD */
	}
	else
	{
		PC = PAGEADDR(PC, ADDR_MASK(word));
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
}
/* 0x8:LOAD */
void inst_load(word_t word)
{
	if(I_MASK(word))
		AC(AC_MASK(word)) = IMEM(FIELD, PC, word);
	else
		AC(AC_MASK(word)) = DMEM(FIELD, PC, word);
}

/*0x9:XOR */
void inst_xor(word_t word)
{
	if(I_MASK(word))
		AC(AC_MASK(word)) ^= IMEM(FIELD, PC, word);
	else
		AC(AC_MASK(word)) ^= DMEM(FIELD, PC, word);
}

/* 0xA:PUSH */
void inst_psh(word_t word)
{
	if(I_MASK(word))
		MEM(CADDR(FIELD, AC(AC_MASK(word)))) = IMEM(FIELD, PC, word);
	else
		MEM(CADDR(FIELD, AC(AC_MASK(word)))) = DMEM(FIELD, PC, word);
	AC(AC_MASK(word))--;
}

/* 0xB:POP */
void inst_pop(word_t word)
{
	AC(AC_MASK(word))++;
	if(I_MASK(word))
		IMEM(FIELD, PC, word) = MEM(CADDR(FIELD, AC(AC_MASK(word))));
	else
		DMEM(FIELD, PC, word) = MEM(CADDR(FIELD, AC(AC_MASK(word))));
	MEM(CADDR(FIELD, AC(AC_MASK(word)))) = 0x0000;
}

/* 0xC:EUM */
void inst_eum(word_t word)
{
	/* Set the UM flag in status */
	status |= 0x0800;
	if(I_MASK(word))
	{
		PC = IADDR(FIELD, PC, word);
		FIELD = (FIELD & 0x00FF) | ((FIELD & 0x00FF) << 8);
	}
	else
		PC = DADDR(FIELD, PC, word);
	STATUS |= 0x1000; /* Disable Interrupt */
}

/* 0xD:INT */
void inst_int(word_t word)
{
	/* 0x04: User Interrupt */
	/* 0x00: CPU Device Handler */
	Interrupt(PC, 0x04, 0x00)
}

/* 0xE:SYS */
void inst_sys(word_t word)
{
	/* STUB */
}

/* 0xF:STP */
void inst_stp(word_t word)
{
	/* STUB */
}
