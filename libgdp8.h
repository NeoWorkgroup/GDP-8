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
#define L ((st & 0x8000) >> 15)
#define SC sc
#define CF ((field & 0xFF00) >> 8)
#define DF (field & 0x00FF)

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
#define FADDR(f, a) \
	((f << 16) | a)

/* Get Page Address's Real Address */
#define PADDR(p, a) \
	((p & 0xffff00) | a)

/* Get Real Address */
/* Code Field, Address */
#define CADDR(f, p, a) \
	(((field & 0xFF00) << 8) | ((p & 0xff00) | addr))

/* Data Field, Page Address */
#define DADDR(f, p, a) \
	(((f & 0x00FF) << 16) | ((p & 0xff00) | a))

/* Common Combined Usage */
/* Indirect Address (Indirect from CODE FIELD to DATA FIEL) */
#define IMEM(f, p, word) \
	memory[(((field & 0x00FF) << 16) | memory[(((field & 0xFF00) << 8) | ((p & 0xFF00) | (word & 0x00FF)))])]
/* Direct Access from CODE FIELD */
#define DMEM(f, p, word) \
	memory[(((field & 0xFF00) << 8) | ((p & 0xff00) | (word & 0x00FF)))]

/* Power of 2 */
#define POWTWO(exp) \
	(1 << exp)

typedef uint16_t word_t;

/* Notation:
 * 	0x(XX)YYYY: XX -> Field, YYYY -> Address
 * 	<L,WORD>:	= (L << 16) | WORD
 * 	<AC,MQ>:	= (AC(x) << 16) | MQ
 */

/* Registers:
 * 	AC0~AC3:	Accumulator
 * 	MQ:		Multiplier Quotient
 * 	PC:		Program Counter
 * 	L:		The Link
 * 	STATUS:		Status
 *
 * Special Address:
 * 	0x(FF)0000:		Interrupt Handler
 * 	0x0020~0x002F:	Auto Increment (If Indirect)
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

/* No AC Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       |   2   | 1 | 1 |               8               |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      O P      |   F   | I | C |            A D D R            |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * I:	Indirect
 * C:	Current Page
 * F:	Instruction specific Flag
 */

/* IOT Instruction Format:
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |       4       |               8               |       4       |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |      O P      |          D E V I C E          |    C O D E    |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *
 * DEVICE:	The target Device
 * CODE:	The Pulse Code to Send
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
 * 	0:	NOP		No Operation
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
 * 	C:	DSZ		Double Precision Skip if 0
 * 	D:	DPI		Double Precision Increment
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
 * 4:	Usermode Interrupt
 * 5:	Trap
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
 * 0	AND	YES		AND AC with the Target
 * 1	ADD	YES		ADD the Accumulator with the Target
 * 2	ISZ	NO		ADD the Target, and skip next instruction if result is zero
 * 3	DEP	YES		Deposit the Accumulator into the Target
 * 4	JMS	NO		Jump to the Subroutine
 * 5	JMP	NO		Jump to the Target
 * 6	IOT	YES		I/O Transfer
 * 7	OPR	YES		Operations
 * 8	PSH	YES		Push into Stack
 * 9	POP	YES		Pop off Stack
 * A	CAL	YES		Call function
 * B	RET	YES		Return
 * C	EUM	NO		Enter Usermode at the Address
 * D	INT	YES		Interrupt, same Format as IOT
 * E	SYS	YES		System Call, same Format as IOT, triggers a Interrupt
 * F	STP	NO		Halt, Halt and Catch Fire, or Report Bug
 */

typedef void (*DEV_HANDLER)(uint8_t, uint8_t);

/* Device Handler Registory */
typedef struct
{
	char *name;	/* To Store Device Name */
	DEV_HANDLER handler;	/* Device Handler Function */
	uint8_t min_dev;	/* Minimal Device Number Managed By This Handler */
	uint8_t max_dev;	/* Maxium Device Number Managed By This Handler */
} dev_desc_t;

/* Instruction Definitions */
#define AND	0x0
#define ADD	0x1
#define ISZ	0x2
#define DEP	0x3
#define JMS	0x4
#define JMP	0x5
#define IOT	0x6
#define OPR	0x7
#define PSH	0x8
#define POP	0x9
#define CAL	0xA
#define RET	0xB
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
void inst_psh(uint16_t word);
void inst_pop(uint16_t word);
void inst_cal(uint16_t word);
void inst_ret(uint16_t word);
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
