/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Neo_Chen: This will be the most useless Virtual Machine ever!
 * ENCODING: UTF-8
 * LANGUAGE: zh_TW, en_US
 */

/* 有沒有從命名看出我在向什麼公司的什麼電腦致敬？ */

/* 大致架構：
 * 多累加器，16 Bit 機器字，16 Bit 定址，加上 8 Bit 分段（共 24 Bit）
 * 中斷，指令權限區分，無虛擬記憶體
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgdp8.h>

/* 16 Bit, AC0 ~ AC3 and MQ */
uint16_t ac[4]={0,0,0,0}, mq;
uint8_t sc; /* Step Counter */
uint16_t st=0, sf=0; /* Status and Save Field */
uint16_t *memory;
/* 16 Bit, Memory Addressing (and 8 bit field) */
uint16_t pc=0;
/* Code Field and Data Field */
uint8_t cf=0, df=0;

int main (int argc, char **argv)
{
	memory=calloc(POWTWO(20), 4);
	FILE *corefile;
	int opt;
	while((opt = getopt(argc, argv, "hf:s:")) != -1)
	{
		switch(opt)
		{
			case 's':
				sscanf(optarg, "%hx", &pc);
				break;
			case 'f':
				if((corefile = fopen(optarg, "r")) == NULL)
				{
					perror(argv[0]);
					exit(8);
				}
				load_core(corefile);
				break;
			case 'h':
				printf("Usage: %s [-h] [-f file] [-s address]\n", argv[0]);
				break;
			default:
				fprintf(stderr,"%s: %s\n", argv[0], optarg);
				exit(0);
		}
	}
	return 0;
}

