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

extern word_t *memory;

/* Get Address (24 Bit) from Byte Stream */
int32_t getaddrint(FILE *fp)
{
	int32_t temp=0;
	temp=fgetc(fp);
	if(temp == EOF)
		return EOF;
	temp |= fgetc(fp) << 8;
	temp |= fgetc(fp) << 16;
	return temp;
}

int32_t getdataint(FILE *fp)
{
	int32_t temp=0;
	temp = fgetc(fp);
	if(temp == EOF)
		return EOF;
	temp |= fgetc(fp) << 8;
	return temp;
}

int32_t putaddrint(FILE *fp, uint32_t addr)
{
	uint32_t temp=addr;
	putc(temp & 0xFF, fp);
	temp >>= 8;
	putc(temp & 0xFF, fp);
	temp >>= 8;
	putc(temp & 0xFF, fp);
}

int32_t putdataint(FILE *fp, uint16_t word)
{
	uint16_t temp=word;
	putc(temp & 0xFF, fp);
	temp >>= 8;
	putc(temp & 0xFF, fp);
}

int read_core(word_t *memory, FILE *fp)
{
	int32_t addr=0; /* We don't know whether it's Normal or EOF */
	int32_t data=0;
	uint8_t flag=fgetc(fp);
	int count=0;
	if(flag & 0x0001) /* Is RIM */
	{
		while((addr = getaddrint(fp)) != EOF )
		{
			data = getdataint(fp);
			if(data == EOF)
				return EOF;
			MEM(addr) = (uint16_t)data;
			count++;
		}
		return count;
	}
	else /* Is BIN */
	{
		addr = getaddrint(fp); /* Initial Address */
		while((data = getdataint(fp)) != EOF)
		{
			MEM(addr++) = (word_t)data;
			count++;
		}
		return count;
	}
	return EOF; /* You shouldn't be here */
}

int write_core(uint8_t flag, uint32_t start_address, uint32_t end_address, FILE *fp)
{
	int32_t addr;
	addr=start_address;
	putc(flag, fp);
	if(flag & 0x0001) /* Is RIM */
	{
		while(addr <= end_address)
		{
			putaddrint(fp, addr);
			putdataint(fp, MEM(addr));
			addr++;
		}
	}
	else /* Is BIN */
	{
		putaddrint(fp, addr);
		while(addr <= end_address)
		{
			putdataint(fp, MEM(addr));
			addr++;
		}
	}
	return addr;
}
