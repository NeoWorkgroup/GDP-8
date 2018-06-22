/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* io.c: IOT Device Handler */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libgdp8.h>

extern word_t ac[4], mq, st, field, sst, sfield, *memory, pc;
extern uint8_t sc;

void console_handler(uint8_t device, uint8_t code)
{
	switch(code)
	{
		case 0x1:
			putchar((char)(AC(0)&0x00FF));
			break;
		case 0x2:
			putc((char)(AC(0)&0x00FF), stderr);
			break;
		case 0x3:
			AC(0)=(uint16_t)(getchar()&0xFF);
			break;
		default:
			return;
	}
	return;
}
