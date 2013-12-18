#include <stdio.h>
#include "vc40map.h"

main(argc, argv)
int argc;
char *argv[];
{
	unsigned long	wval = 0xa55aa55a;
	
	vc40map();
	
	if (argc > 1) {
		wval = strtol(argv[1], NULL, 0);
	}
	printf("write 0x%x to DRAM\n", wval);
	*Dram = wval;

	printf("Read back: 0x%x\n", *Dram);
}
