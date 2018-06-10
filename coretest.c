#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
int main(void)
{
	srandom((long int)time(NULL));
	srandomdev();
	uint32_t addr=0;
	for(addr=0; addr <= 0xfffff; addr++)
		printf("%05x:%04hx\n", addr , (uint16_t)(random()%65536));
}
