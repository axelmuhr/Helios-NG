#include <stdio.h>
#include <math.h>       /* need this for sin() and fabs() to work! */
#include <signal.h>
#include "hyhomon.h"    /* Hydra defintions, etc. */

#define	POINTS	512     /* size of FFT to perform */

/*
 * Notice that the number of symbols to extract from the COFF file is
 * defined in terms of the size of the symnames array.  This way new
 * symbols and be added to symnames with a minimum of fuss.
 */
#define	NUMSYMS	(sizeof(symnames)/sizeof(char *))

/*
 * These are the indices of the symbols within the symnames array, and 
 * therefore in the symtab array
 */
#define	INPUT	0
#define	OUTPUT	1
#define	START	2
#define	ELTIME	3
#define	FFTSIZE	4
#define INTPRI  5
#define INTVEC  6

/*
 * notice that symbol names have a leading "_" - the TI C compiler does this,
 * as do most compilers.
 */
char *symnames[] = {
    "_in_addr",
    "_out_addr",
    "_start_flag",
    "_elapsed_time",
    "_FFTSize",
    "_intpri",
    "_intvec"
};
struct symtab	symtab[NUMSYMS];   /* symbol table entries */

/*
 * Complex data type for FFT result - in TI float format!
 */
typedef struct {
	u_long x, y;
} COMPLEX;

/*
 * input to the FFT will consist of two mixed sine functions, thus the
 * the output will have two well defined peaks.
 */
#define	N1	(POINTS/4)
#define	N2	(POINTS/3)

/*
 * signal handler stuff
 */
void c40_handler();
int done_flag = 0;

main(argc, argv)
int argc;
char *argv[];
{
    int vc40_fd;  /* file descriptor for the DSP we will use */
    int signum = SIGUSR1;   /* Grab this signal for the DSP */
    u_long entry_address;   /* Program start address returned by c40_load() */
    u_long inp_addr;        /* address of input array */
    u_long out_addr;        /* address of the output array */
    int i, undef = 0;
    u_long sflag;
    float pi = 4.0*atan(1.0);
    float f1, f2, real, imag, etime;
    u_long sig[POINTS];    /* input signal in TI float format */
    COMPLEX fft[POINTS];   /* fft result */
    struct vc40info vc40info;   /* information about the Hydra */

    /*
     * open the DSP
     */
    vc40_fd = open("/dev/vc40a1", O_RDWR);

    /*
     * always halt the DSP before loading, lest we load overtop another
     * executing program!
     */
    ioctl(vc40_fd, VC40HALT);

    /*
     * get information about this Hydra
     */
    ioctl(vc40_fd, VC40GETINFO, &vc40info);
    printf("This Hydra has %d DSPs and its DRAM is %d MWords\n",
	   vc40info.numdsp, vc40info.dram_size/1024/1024/4);

    /*
     * load the program to the DSP
     */
    if (c40_load(vc40_fd, "fftdsp.x40", &entry_address, NUMSYMS, symnames, 
		 symtab) == 0) {
	printf("cofferr is: `%s'\n", cofferr);
	exit(1);
    }
    printf("entry address: 0x%lx\n", entry_address);

    /*
     * quick check to make sure all symbols are defined
     */
    for (i=0; i<NUMSYMS; i++) {
	if (symtab[i].type == T_UNDEF) {
	    printf("Symbol `%s' is undefined!\n", symnames[i]);
	    undef = 1;
	}
    }
    if (undef) exit(1);

    /*
     * NOTE: This is a temporary kludge as the HyHoMon host interrupt
     * function is not working.
     * upload the interrupt vector and interrupt priority (level).
     */
    c40_put_long(vc40_fd, symtab[INTPRI].val.l, vc40info.intpri);
    c40_put_long(vc40_fd, symtab[INTVEC].val.l, vc40info.intvec);

    /*
     * now grab the signal and enable interrupts from Hydra
     */
    signal(signum, c40_handler);
    ioctl(vc40_fd, VC40ENINT, &signum);

    /*
     * start the program running - once started, it waits for the start
     * flag to be set before computation.
     */
    c40_run(vc40_fd, entry_address);

    /*
     * the DSP program needs to know the size of the FFT to be calculated
     */
    c40_put_long(vc40_fd, symtab[FFTSIZE].val.l, POINTS);

    /*
     * The DSP program dynamically allocates the input and output buffers.
     * So, we get the addresses of the pointers from the COFF file, then
     * read the values of the pointers to get the buffer addresses.
     */
    c40_get_long(vc40_fd, symtab[INPUT].val.l, &inp_addr);  /* input buffer */
    c40_get_long(vc40_fd, symtab[OUTPUT].val.l, &out_addr); /* output buffer */

    printf("input address is 0x%x\n", inp_addr);
    printf("output address is 0x%x\n", out_addr);
    printf("start flag is 0x%x\n", symtab[START].val.l);

    /*
     * calculate input signal - mixed sine waves.  Note that we
     * convert to TI floating point format on the fly here.
     */
    f1 = N1 * 2.0*pi/POINTS;
    f2 = N2 * 2.0*pi/POINTS;
    for (i=0; i<POINTS; i++) {
	sig[i] = ieee2dsp(sin(i*f1) + 0.5*sin(i*f2));
    }
    printf("\nFFT Size: %d\n", POINTS);
    printf("Expect peaks at %d and %d\n\n", N1, N2);

    /*
     * download the input data to the DSP
     */
    c40_write_long(vc40_fd, inp_addr, sig, POINTS);

    /*
     * set the start flag to start the computation
     */
    c40_put_long(vc40_fd, symtab[START].val.l, 1L);

    /*
     * when the DSP is done, it will send an interrupt which the driver
     * will receive, the driver will then send me SIGUSR1, which will
     * set the done_flag.
     */
    while (!done_flag) ;

    /*
     * now get the results (their complex!).  Since we are doing a real
     * FFT, there are only half the number points to read back, but they
     * are complex, so its double the number of words.  In short, its
     * the same amount of data that we wrote!
     */
    c40_read_long(vc40_fd, out_addr, fft, POINTS);

    /*
     * now just print out the peaks
     */
    for (i=0; i<POINTS/2; i++) {
	real = dsp2ieee(fft[i].x);
	imag = dsp2ieee(fft[i].y);
	if (fabs(real) > 0.01 || fabs(imag) > 0.01) { 
	    printf("%d) %.2f + j * %.2f\n", i, real, imag);
	} 
    }

    /*
     * The DSP even calculates the amount of time it took to perform the
     * FFT!
     */
    c40_get_dsp_float(vc40_fd, symtab[ELTIME].val.l, &etime);
    printf("\n\nelapsed time: %f usec\n", 1e6*etime);

    /*
     * that's all there is to it!
     */
    return (0);
}

void c40_handler()
{
    printf("Got signal!\n");
    done_flag = 1;
}
