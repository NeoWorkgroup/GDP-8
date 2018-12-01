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

struct CPU *cpu;
FILE	*corefile;
addr_t	start_address=0;
word_t	panel=0;

void parsearg(int argc, char **argv)
{
	int opt=0;
	while((opt = getopt(argc, argv, "hc:s:p:")) != EOF)
	{
		switch(opt)
		{
			case 'h':
				fprintf(stderr, "%s [-h] [-c corefile]\n", argv[0]);
				exit(0);
				break;
			case 'c':
				if((corefile = fopen(optarg, "r")) == NULL)
				{
					perror(optarg);
					exit(8);
				}
				break;
			case 's':
				sscanf(optarg, "%x", &start_address);
				break;
			case 'p':
				sscanf(optarg, "%lx", &panel);
				break;
			default:
				panic("?ARG\n");
				break;
		}
	}
}

void read_core(FILE *fp, struct CPU *cpu, addr_t start_address)
{
	int buffer=0;
	addr_t address=start_address;
	if(fp == NULL)
		panic("?CORE\n");
	while((buffer = fgetc(fp)) != EOF && address < (1<<24))
		cpu->mem[address++]=(byte_t)buffer;
}

int main(int argc, char **argv)
{
	parsearg(argc, argv);
	cpu = cpu_init();
	read_core(corefile, cpu, start_address);
	IREG(cpu).panelswitch=panel;
	cpu_mainloop(cpu, start_address);
	cpu_destroy(cpu);
	return 0;
}
