#include <stdio.h>
#include <signal.h>
#include "hyhomon.h"

void handler();

int done = 0;

char *symnames[1] = {"junk"};
struct symtab symtab[1];

main()
{
	int	dsp;
	int	signum = SIGUSR1;
	u_long	entry_address;

	dsp = open("/dev/vc40a2", O_RDWR);
	if (dsp == -1) {
		perror("opening");
		exit(1);
	}

	if (c40_load(dsp, "hinttest.x40", &entry_address, 1, 
		symnames, symtab) == 0) {
		printf("coff error %s\n", cofferr);
		exit(1);
	}
printf("ready to enable interrupts...\n");
getchar();
	ioctl(dsp, VC40ENINT, &signum);
	signal(signum, handler);

	printf("ready to run...\n");
	getchar();
	
	ioctl(dsp, VC40RUN, &entry_address);

	while(!done) pause();
}

void handler()
{
	printf("GOT SIGNAL!!!!\n");
	done = 1;
}
