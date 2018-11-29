/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "libgdp8.h"
#include "insthandler.h"

IO_DEFINE(cpu)
{
}

IO_DEFINE(console)
{
	switch(cpu->ireg.inst.arg.io.op)
	{
		case CONSOLE_IO_OUT:
			fputc((int)(cpu->reg.r[cpu->ireg.inst.arg.io.reg] & 0xFF), stdout);
			break;
		case CONSOLE_IO_IN:
			cpu->reg.r[cpu->ireg.inst.arg.io.reg]=(word_t)fgetc(stdin);
			break;
		case CONSOLE_IO_BEL:
			fputc('\a', stdout);
			break;
		case CONSOLE_IO_RST:
			fputs("\033)0\033[H\033[J", stdout);
			break;
	}
}
