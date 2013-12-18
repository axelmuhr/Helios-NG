#include <stdio.h>
#include <math.h>
#include "vc40map.h"

#define	HYDRA1_VIC	0xf000

#define	POINTS	512

#define	NUMSYMS	(sizeof(symnames)/sizeof(char *))
#define	INPUT	0
#define	OUTPUT	1
#define	START	2
#define	ELTIME	3
#define	FFTSIZE	4

char *symnames[] = {
	"_in_addr",
	"_out_addr",
	"_start_flag",
	"_elapsed_time",
	"_FFTSize"
};
struct symtab	symtab[NUMSYMS];

typedef struct {
	float x, y;
} COMPLEX;

#define	N1	(POINTS/4)
#define	N2	(POINTS/3)

main(argc, argv)
int argc;
char *argv[];
{
	unsigned long	entry_address, inp_addr, out_addr, dsp;
	int	i, undef = 0;
	u_long	sflag;
	float	pi = 4.0*atan(1.0);
	float	f1, f2;
	float	real, imag, etime;
	float	sig[POINTS];
	COMPLEX	fft[POINTS];


	vc40map(HYDRA1_VIC);
	dsp = 1;
	if (argc > 1) dsp = atoi(argv[1]);
printf("Halting...\n");
	c40_halt(dsp);	/* always halt before loading! */
printf("loading...\n");
	if (c40_load(dsp, "test.x40", &entry_address, NUMSYMS, 
	    symnames, symtab) == 0) {
		printf("cofferr is: `%s'\n", cofferr);
		exit(1);
	}
	printf("entry address: $%lx\n", entry_address);

	for (i=0; i<NUMSYMS; i++) {
		if (symtab[i].type == T_UNDEF) {
			printf("Symbol `%s' is undefined!\n", symnames[i]);
			undef = 1;
		}
	}
	if (undef) exit(1);

	c40_get_long(dsp, symtab[INPUT].val.l, &inp_addr);
	c40_get_long(dsp, symtab[OUTPUT].val.l, &out_addr);

	printf("input address is 0x%x\n", inp_addr);
	printf("output address is 0x%x\n", out_addr);
	printf("start flag is 0x%x\n", symtab[START].val.l);

	/*
	 * calculate input signal
	 */
	f1 = N1 * 2.0*pi/POINTS;
	f2 = N2 * 2.0*pi/POINTS;
	for (i=0; i<POINTS; i++) {
		sig[i] = sin(i*f1) + 0.5*sin(i*f2);
	}
	printf("\nFFT Size: %d\n", POINTS);
	printf("Expect peaks at %d and %d\n\n", N1, N2);

printf("running...\n");
	c40_run(dsp, entry_address);

printf("setting FFT size\n");
	c40_put_long(dsp, symtab[FFTSIZE].val.l, POINTS);

printf("writing signal...\n");
	for (i=0; i<POINTS; i++) {
		c40_put_dsp_float(dsp, inp_addr+i, sig[i]);
	}

printf("setting start flag...\n");
	c40_put_long(dsp, symtab[START].val.l, 1L);

	do {
		for (i=0; i<1000000; i++) ;
		c40_get_long(dsp, symtab[START].val.l, &sflag);
	} while (sflag);
printf("getting result...\n");
	for (i=0; i<POINTS/2; i++) {
		c40_get_dsp_float(dsp, out_addr+2*i, &real);
		c40_get_dsp_float(dsp, out_addr+2*i+1, &imag);
		if (fabs(real) > 0.01 || fabs(imag) > 0.01) {
			printf("%d) %.2f + j * %.2f\n", i, real, imag);
		}
	}

	c40_get_dsp_float(dsp, symtab[ELTIME].val.l, &etime);
	printf("\n\nelapsed time: %f usec\n", 1e6*etime);

}

