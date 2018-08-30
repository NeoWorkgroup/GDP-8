/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* libgdp8.h: Common types and macros */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define AC(x) ac[x]
#define MQ mq
#define STATUS status
#define MEM(x) memory[x]
#define PC pc
#define L ((st & 0x8000) >> 15)
#define SC sc

/* Define some macros to simplify things */
#define INST_MASK(word) \
	((word & 0xf000) >> 12)

#define AC_MASK(word) \
	((word & 0x0C00) >> 10)

#define F_MASK(word) \
	((word & 0x0C00) >> 10)

#define I_MASK(word) \
	((word & 0x0200) >> 9)

#define C_MASK(word) \
	((word & 0x0100) >> 8)

#define ADDR_MASK(word) \
	(word & 0x00FF)

#define DEV_MASK(word) \
	((word & 0x0FF0) >> 4)

#define CODE_MASK(word) \
	(word & 0x000F)

#define DATA_FIELD_MASK(word) \
	(word & 0x00FF)

#define CODE_FIELD_MASK(word) \
	((word & 0xFF00) >> 8)

#define OPR_GROUP_MASK(word) \
	((word & 0x0600) >> 8)

#define OPR_I_MASK(word) \
	((word & 0x0100) >> 7)

#define ST_L_MASK(word) \
	((word & 0x8000) >> 15)

#define ST_GT_MASK(word) \
	((word & 0x4000) >> 14)

#define ST_INT_MASK(word) \
	((word & 0x2000) >> 13)

#define ST_DIT_MASK(word) \
	((word & 0x1000) >> 12)

#define ST_UM_MASK(word) \
	((word & 0x0800) >> 11)

#define ST_STAT_MASK(word) \
	((word & 0x0700) >> 8)

#define SETL(x) \
	(l = (x & 0x1))

/* Combine Field and Address */
#define FIELDADDR(f, a) \
	((f << 16) | a)

/* Get Page Address's Real Address */
#define PAGEADDR(p, a) \
	((p & 0xFF00) | a)

/* Power of 2 */
#define POWTWO(exp) \
	(1 << exp)

typedef uint16_t word_t;
typedef void (*DEV_HANDLER)(uint8_t, uint8_t);

/* Instruction Definitions */
#define AND	0x0
#define ADD	0x1
#define ISZ	0x2
#define DEP	0x3
#define JMS	0x4
#define JMP	0x5
#define IOT	0x6
#define OPR	0x7
#define CALL	0x8
#define RET	0x9
#define PUSH	0xA
#define POP	0xB
#define EUM	0xC
#define INT	0xD
#define SYS	0xE
#define STP	0xF

void inst_and(uint16_t word);
void inst_add(uint16_t word);
void inst_isz(uint16_t word);
void inst_dep(uint16_t word);
void inst_jms(uint16_t word);
void inst_jmp(uint16_t word);
void inst_iot(uint16_t word);
void inst_opr(uint16_t word);
void inst_load(uint16_t word);
void inst_xor(uint16_t word);
void inst_push(uint16_t word);
void inst_pop(uint16_t word);
void inst_eum(uint16_t word);
void inst_int(uint16_t word);
void inst_sys(uint16_t word);
void inst_stp(uint16_t word);
uint16_t rotl(uint16_t word, uint8_t count);
uint16_t rotr(uint16_t word, uint8_t count);
void lwrotr(uint16_t *word, uint16_t *status);
void lwrotl(uint16_t *word, uint16_t *status);

/* Device Handlers */
void console_handler(uint8_t device, uint8_t code);
