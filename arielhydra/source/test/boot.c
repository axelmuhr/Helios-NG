#include <stdio.h>
#include "hyhomon.h"

#define	DELAY	0

#define	NUMSYMS	(sizeof(symnames)/sizeof(char *))

char *symnames[] = {
	"_delay"
};
struct symtab	symtab[NUMSYMS];

main(argc, argv)
int argc;
char *argv[];
{
	char	uinp[20], dname[20];
	u_long	ibuf[32], obuf[32];
	unsigned long	entry_address, delay_val, dsp;
	int	i, c40_fd1, c40_fd2;
	int	status;

	dsp = 2;
	if (argc > 1) dsp = atoi(argv[1]);
	sprintf(dname, "/dev/vc40a%d", dsp);
printf("device name: %s\n", dname);
	c40_fd2 = open(dname, O_RDWR);
	if (c40_fd2 == -1) {
		perror("opening");
		exit(1);
	}
	printf("Press ENTER to boot DSP %d\n", dsp);
	getchar();
	status = ioctl(c40_fd2, VC40RESET);
	if (status) {
		perror("VC40BOOT failed");
		exit(1);
	}

	/*
	 * test read/write with DSP 
	 */
	printf("Press ENTER to perform write/read test on DSP %d\n", dsp);
	getchar();
	for (i=0; i<32; i++) {
		obuf[i] = 3*i+5;
	}
	c40_write_long(c40_fd2, 0x8d000000, obuf, 32);
	c40_read_long(c40_fd2, 0x8d000000, ibuf, 32);
	for (i=0; i<32; i++) {
		if (obuf[i] != ibuf[i]) {
			printf("%d) wrote: 0x%x; read: 0x%x\n", i, 
				obuf[i], ibuf[i]);
		}
	}
/* exit(1); */
	printf("Press ENTER to run program on DSP %d\n", dsp);
	getchar();
	if (c40_load(c40_fd2, "blink.x40", &entry_address, NUMSYMS, 
	    symnames, symtab) == 0) {
		printf("cofferr is: `%s'\n", cofferr);
		exit(1);
	}
	printf("entry address: $%lx\n", entry_address);

	if (symtab[DELAY].type != T_UNDEF) {
		printf("delay is at address 0x%x\n", symtab[DELAY].val.l);
	}
	else {
		printf("delay is undefined!\n");
	}

	status = ioctl(c40_fd2, VC40RUN, &entry_address);

	do {
		ioctl(c40_fd2, VC40SETADDR, &symtab[DELAY].val.l);
		if (read(c40_fd2, &delay_val, sizeof(long)) != sizeof(long)) {
			printf("read failed\n");
			exit(1);
		}
		printf("Current delay value: 0x%x\n", delay_val);
		printf("Enter new value: ");
		fflush(stdout);
		scanf("%s", uinp);
		delay_val = strtol(uinp, 0, 0);
		ioctl(c40_fd2, VC40SETADDR, &symtab[DELAY].val.l);
		if (write(c40_fd2, &delay_val, sizeof(long)) != sizeof(long)) {
			printf("write failed\n");
			exit(1);
		}
	} while (delay_val != 0);

	ioctl(c40_fd2, VC40HALT);

}

