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
}

/* 0x1:ADD */
void inst_add(word_t word)
{
}

/* 0x2:ISZ */
void inst_isz(word_t word)
{
}

/* 0x3:DEP */
void inst_dep(word_t word)
{
}

/* 0x4:JMS */
void inst_jms(word_t word)
{
}

/* 0x5:JMP */
void inst_jmp(word_t word)
{
}

/* 0x6:IOT */
void inst_iot(word_t word)
{
}

/* 0x7:OPR */
void inst_opr(word_t word)
{
}
/* 0x8:LOAD */
void inst_load(word_t word)
{
}

/*0x9:XOR */
void inst_xor(word_t word)
{
}

/* 0xA:PUSH */
void inst_psh(word_t word)
{
}

/* 0xB:POP */
void inst_pop(word_t word)
{
}

/* 0xC:EUM */
void inst_eum(word_t word)
{
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
