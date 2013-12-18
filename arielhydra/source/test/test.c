#include <stdio.h>
#include "hyhomon.h"

#define	DELAY	0

#define	NUMSYMS	(sizeof(symnames)/sizeof(char *))

char *symnames[] = {
	"_delay"
};
struct symtab	symtab[NUMSYMS];

main()
{
	char	uinp[20];
	unsigned long	entry_address, delay_val, *dramp;
	int	i, vc40_fd, vc40_fd2;
	int	status;

	vc40_fd = open("/dev/vc40a1", O_RDWR);
	if (vc40_fd == -1) {
		perror("opening /dev/vc40a1");
		exit(1);
	}

	/*
	 * try disabling keyboard junk
	 */
	printf("ready to disable...\n");
	getchar();
	ioctl(vc40_fd, VC40DISKEY);
	printf("ready to enable...\n");
	getchar();
	ioctl(vc40_fd, VC40ENKEY);
exit(1);

	/*
	 * try mapping the DRAM
	 */
	dramp = (u_long *) c40_map_dram(vc40_fd, 0, 0x100);
	for (i=0; i<20; i++) printf("0x%x\n", *dramp++);
exit(1);

	if (c40_load(vc40_fd, "blink.x40", &entry_address, NUMSYMS, 
	    symnames, symtab) == 0) {
		printf("cofferr is: `%s'\n", cofferr);
	}
	printf("entry address: $%lx\n", entry_address);

	if (symtab[DELAY].type != T_UNDEF) {
		printf("delay is at address 0x%x\n", symtab[DELAY].val.l);
	}
	else {
		printf("delay is undefined!\n");
	}

	printf("press enter to continue\n");
	getchar();

	status = ioctl(vc40_fd, VC40RUN, &entry_address);

	printf("press enter to continue\n");
	getchar();

	while (1) {
		ioctl(vc40_fd, VC40SETADDR, &symtab[DELAY].val.l);
		if (read(vc40_fd, &delay_val, sizeof(long)) != sizeof(long)) {
			printf("read failed\n");
			exit(1);
		}
		printf("Current delay value: 0x%x\n", delay_val);
		printf("Enter new value: ");
		fflush(stdout);
		scanf("%s", uinp);
		delay_val = strtol(uinp, 0, 0);
		ioctl(vc40_fd, VC40SETADDR, &symtab[DELAY].val.l);
		if (write(vc40_fd, &delay_val, sizeof(long)) != sizeof(long)) {
			printf("write failed\n");
			exit(1);
		}
	}

}

