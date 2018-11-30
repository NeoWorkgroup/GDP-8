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
	switch(INST(cpu).arg.io.op)
	{
		case CPU_IO_EI:
			IREG(cpu).interrupt=1;
			break;
		case CPU_IO_DI:
			IREG(cpu).interrupt=0;
			break;
		case CPU_IO_ECLK:
		case CPU_IO_DCLK:
			break;
		case CPU_IO_PSR:
			R(cpu, INST(cpu).arg.io.reg) = IREG(cpu).panelswitch;
			break;
		case CPU_IO_DPY:
			IREG(cpu).display = R(cpu, INST(cpu).arg.io.reg);
			break;
	}
}

IO_DEFINE(console)
{
	switch(INST(cpu).arg.io.op)
	{
		case CONSOLE_IO_OUT:
			fputc((int)(R(cpu, INST(cpu).arg.io.reg) & 0xFF), stdout);
			break;
		case CONSOLE_IO_IN:
			R(cpu, INST(cpu).arg.io.reg)=(word_t)fgetc(stdin);
			break;
		case CONSOLE_IO_BEL:
			fputc('\a', stdout);
			break;
		case CONSOLE_IO_RST:
			fputs("\033)0\033[H\033[J", stdout);
			break;
	}
}
