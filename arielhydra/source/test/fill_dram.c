#include <stdio.h>
#include <string.h>
#include "vc40map.h"

main(argc, argv)
int argc;
char *argv[];
{
	unsigned long	addr, len, i, val;
	
	if (argc != 4) {
		printf("Usage: fill_dram <offset> <len> <value>\n");
		exit(1);
	}

	addr = strtol(argv[1], NULL, 0);
	len = strtol(argv[2], NULL, 0);
	val = strtol(argv[3], NULL, 0);

	vc40map(addr, sizeof(long)*len);
	
	for (i=0; i<len; i++) {
		Dram[i] = val;
	}
}
