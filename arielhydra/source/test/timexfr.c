#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include "hyhomon.h"
#include "vc40map.h"

#define	BSIZE	0x4000
#define	LOOPS	0x200

main()
{
	int	dsp, i, j;
	long	dbuf[BSIZE];
	long	*dptr;
	u_long addr = 0x40006000;
	float	esec, etime, eusec;
	struct timeval stp, etp;

	printf("timing DRAM read/write...\n");
	vc40map(0, 4*BSIZE);
	gettimeofday(&stp, NULL);
	for (i=0; i<LOOPS; i++) {
		dptr = Dram;
		for (j=0; j<BSIZE; j++) {
			*dptr++ = j;
		}
	}
	gettimeofday(&etp, NULL);

	esec = etp.tv_sec - stp.tv_sec;
	eusec = (etp.tv_usec - stp.tv_usec) / 1e6;
	etime = esec + eusec;

	printf("Elapsed time: %f sec\n", etime);
	printf("Rate: %8.2f Kb/s\n", sizeof(long)*BSIZE*LOOPS/etime/1000.0);

	printf("Timing driver read/write\n");
	dsp = open("/dev/vc40a1", O_RDWR);	

	gettimeofday(&stp, NULL);

	for (i=0; i<LOOPS; i++) {
		ioctl(dsp, VC40SETADDR, &addr);
		write(dsp, dbuf, sizeof(long)*BSIZE);
/*		read(dsp, dbuf, sizeof(long)*BSIZE); */

	}

	gettimeofday(&etp, NULL);

	esec = etp.tv_sec - stp.tv_sec;
	eusec = (etp.tv_usec - stp.tv_usec) / 1e6;
	etime = esec + eusec;

	printf("Elapsed time: %f sec\n", etime);
	printf("Rate: %8.2f Kb/s\n", sizeof(long)*BSIZE*LOOPS/etime/1000.0);
}
