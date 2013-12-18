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
	u_long	*ibuf, *obuf, addr;
	unsigned long	entry_address, delay_val, dsp;
	int	i, c40_fd1, c40_fd2;
	int	cnt = 64;
	int	status;

	if (argc < 3) {
		printf("Usage: copy <dsp no.> <dsp address>\n");
		exit(1);
	}
	dsp = atoi(argv[1]);
	addr = strtol(argv[2], NULL, 0);
	sprintf(dname, "/dev/vc40a%d", dsp);
	printf("device name: %s; address: 0x%x\n", dname, addr);
	if (argc > 3) {
		cnt = strtol(argv[3], NULL, 0);
	} 
	printf("Testing 0x%x words\n", cnt);
	c40_fd2 = open(dname, O_RDWR);
	if (c40_fd2 == -1) {
		perror("opening");
		exit(1);
	}

	/*
	 * test read/write with DSP 
	 */
	obuf = (u_long *) calloc(cnt, sizeof(long));
	ibuf = (u_long *) calloc(cnt, sizeof(long));
	printf("Press ENTER to perform write/read test on DSP %d\n", dsp);
	getchar();
	for (i=0; i<cnt; i++) {
		obuf[i] = i;
	}
	if (ioctl(c40_fd2, VC40SETADDR, &addr) == -1) {
		perror("VC40SETADDR");
		exit(1);
	}
	if (write(c40_fd2, obuf, 4*cnt) == -1) {
		perror("write");
		exit(1);
	}
printf("ready to read...\n");
getchar();
	c40_read_long(c40_fd2, addr, ibuf, cnt);
	for (i=0; i<cnt; i++) {
		if (obuf[i] != ibuf[i]) {
			printf("%d) wrote: 0x%x; read: 0x%x\n", i, 
				obuf[i], ibuf[i]);
		}
	}
}

