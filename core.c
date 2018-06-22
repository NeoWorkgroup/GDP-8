/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* core.c: For the interpretion and creation of coredump */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgdp8.h>

/* Get Address (24 Bit) from Byte Stream */
int32_t getaddrint(FILE *fp)
{
	int32_t temp=0;
	temp |= fgetc(fp);
	if(temp == EOF)
		return EOF;
	temp <<= 16;
	temp |= fgetc(fp) << 8;
	temp |= fgetc(fp);
	return temp;
}

int32_t getdataint(FILE *fp)
{
	int32_t temp=0;
	temp |= fgetc(fp);
	if(temp == EOF)
		return EOF;
	temp <<= 8;
	temp |= fgetc(fp);
	return temp;
}

int read_core(word_t *memory, FILE *fp)
{
	int32_t addr=0; /* We don't know whether it's Normal or EOF */
	int32_t data=0;
	uint8_t flag=fgetc(fp);
	int count=0;
	if(flag & 0x0001) /* Is RIM */
	{
		while((unsure_addr = getaddrint(fp)) != EOF )
		{
			MEM(unsure_addr) = getdataint(fp);
			count++;
		}
		return;
	}
	else /* Is BIN */
	{
		addr = getaddrint(fp); /* Initial Address */
		while((unsure_data = getdataint(fp)) != EOF)
			MEM(addr++) = (word_t)data;
	}
}
