#include <stdio.h>
#include <signal.h>
#include "hyhomon.h"

void handler();

int done = 0;

char *symnames[1] = {"junk"};
struct symtab symtab[1];

main(argc, argv)
int argc;
char *argv[];
{
	char	devname[20];
	int	dsp, i;
	int	signum = SIGUSR1;
	u_long	entry_address;
	u_long	oval[0x4000 - 0xe00];

	if (argc <2) {
		printf("usage: erase <dsp no>\n");
		exit(1);
	}
	strcpy(devname, "/dev/vc40a");
	strcat(devname, argv[1]);
	printf("Opening %s\n", devname);
	dsp = open(devname, O_RDWR);
	if (dsp == -1) {
		perror("opening");
		exit(1);
	}

	ioctl(dsp, VC40HALT);

printf("filling array...\n");
	for (i=0; i<0x4000-0xe00; i++) oval[i] = 0x12345678;
printf("Press return to write\n");
getchar();
printf("writing...\n");
	c40_write_long(dsp, 0x40000e00, oval, 0x4000-0xe00);

	if (c40_load(dsp, "hinttest.x40", &entry_address, 1, 
		symnames, symtab) == 0) {
		printf("coff error %s\n", cofferr);
		exit(1);
	}

	ioctl(dsp, VC40ENINT, &signum);
	signal(signum, handler);

	printf("ready to run...\n");
	getchar();
	
	ioctl(dsp, VC40RUN, &entry_address);

	while(!done) pause();

printf("ready to halt...\n");
getchar();
	ioctl(dsp, VC40HALT);
}

void handler()
{
	printf("GOT SIGNAL!!!!\n");
	done = 1;
}
