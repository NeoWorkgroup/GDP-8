/* =================================================== *\
||                 D C C   G D P - 8                   ||
|| digital computer corpration, general data processor ||
\* =================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

int input=0;
FILE *corefile;

void parsearg(int argc, char **argv)
{
	int opt=0;
	while((opt = getopt(argc, argv, "hf:")) != EOF)
	{
		switch(opt)
		{
			case 'f':
				if(strcmp(optarg, "-"))
				{
					if((corefile = fopen(optarg, "r")) == NULL)
					{
						perror(optarg);
						exit(8);
					}
				}
				else
					corefile=stdin;
				break;
			default:
			case 'h':
				fprintf(stderr, "%s [-h] [-f file]\n", argv[0]);
				exit(0);
				break;
		}
	}
}

int main(int argc, char **argv)
{
	corefile=stdin;
	parsearg(argc, argv);
	while(fscanf(corefile, "%2x", &input) != EOF)
		fputc((char)input, stdout);
	return 0;
}
