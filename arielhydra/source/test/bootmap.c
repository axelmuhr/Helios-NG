#include <stdio.h>
#include "vc40map.h"

#define	DELAY_SYM	0

#define	HYDRA1_VIC	0xf000
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
	int	i;
	int	status;

	vc40map(HYDRA1_VIC);
	dsp = 1;
	if (argc > 1) dsp = atoi(argv[1]);
printf("DSP: %d\n", dsp);
#ifdef NOCC
	printf("Press ENTER to boot DSP %d\n", dsp);
	getchar();
	status = ioctl(c40_fd2, VC40RESET);
	if (status) {
		perror("VC40BOOT failed");
		exit(1);
	}
#endif
	/*
	 * test read/write with DSP 
	 */
	printf("Press ENTER to perform write/read test on DSP %d\n", dsp);
	getchar();
	for (i=0; i<32; i++) {
		obuf[i] = i;
	}
printf("write\n");
	c40_write_long(dsp, 0x40001000, obuf, 32);
printf("read\n");
	c40_read_long(dsp, 0x40001000, ibuf, 32);
	for (i=0; i<32; i++) {
		if (obuf[i] != ibuf[i]) {
			printf("%d) wrote: 0x%x; read: 0x%x\n", i, 
				obuf[i], ibuf[i]);
		}
	}
/* exit(1); */
	printf("Press ENTER to run program on DSP %d\n", dsp);
	getchar();
	if (c40_load(dsp, "blink.x40", &entry_address, NUMSYMS, 
	    symnames, symtab) == 0) {
		printf("cofferr is: `%s'\n", cofferr);
		exit(1);
	}
	printf("entry address: $%lx\n", entry_address);

	if (symtab[DELAY_SYM].type != T_UNDEF) {
		printf("delay is at address 0x%x\n", symtab[DELAY_SYM].val.l);
	}
	else {
		printf("delay is undefined!\n");
	}

	c40_run(dsp, entry_address);

	do {
		if (c40_get_long(dsp, symtab[DELAY_SYM].val.l, &delay_val, 1)) {
			printf("read failed\n");
			exit(1);
		}
		printf("Current delay value: 0x%x\n", delay_val);
		printf("Enter new value: ");
		fflush(stdout);
		scanf("%s", uinp);
		delay_val = strtol(uinp, 0, 0);
		if (c40_put_long(dsp, symtab[DELAY_SYM].val.l, delay_val, 1)) {
			printf("write failed\n");
			exit(1);
		}
	} while (delay_val != 0);

}

