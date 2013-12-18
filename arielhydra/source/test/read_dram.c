#include <stdio.h>
#include <string.h>
#include "vc40map.h"

main(argc, argv)
int argc;
char *argv[];
{
	unsigned long	addr, len, i;
	
	if (argc != 3) {
		printf("Usage: read_dram <offset> <len>  (in 32-bit words)\n");
		exit(1);
	}

	addr = strtol(argv[1], NULL, 0);
	len = strtol(argv[2], NULL, 0);

	vc40map(addr, sizeof(long)*len);
	
	for (i=0; i<len; i++) {
		printf("0x%8.8x\n", *(Dram+i));
	}
}
