/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      DCC GDP-8                      *
 * digital computer corpration, general data processor *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Random core file generator */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
int main(void)
{
	srandomdev();
	uint32_t addr=0;
	for(addr=0; addr <= 0xffffff; addr++)
		printf("%06x:%04hx\n", addr , (uint16_t)(random()&0xFFFF));
}
