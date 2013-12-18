/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1991 Ariel Corp.               */
/****************************************************************/


/* DSP specific code for the Mandelbrot set calculation
 * for the Hydra V-C40
 */

#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <errno.h>
#include "mand_dsp.h"
#include "hyhomon.h"

#define	MAXDSP	4

static void get_mand_line();
static unsigned long get_symbol();

#define	NUMSYMS	(sizeof(symnames)/(sizeof(char *)))
#define	START_FLAG	0
#define	LINE_DONE	1
#define	XSTART		2
#define	XSTEP		3
#define	YSTART		4
#define	YSTEP		5
#define	YCOORD		6
#define	WIDTH		7
#define	HEIGHT		8
#define	MAXIT		9
#define	MAXVAL		10
#define	INTPRI		11
#define	INTVEC		12
#define	BUF1		13
#define	BUF2		14
#define	YCOORD1		15
#define	YCOORD2		16
static char *symnames[] = {
	"_start_flag", "_line_done", "_x_start", "_x_step", "_y_start",
	"_y_step", "_y_coord", "_width", "_height", "_maxit", "_max_val",
	"_intpri", "_intvec", "_buf1", "_buf2", "_ycoord1", "_ycoord2"
};
static int	c40_fd[MAXDSP];
static struct symtab symtab[NUMSYMS];
static int	numdsp;
static int	numsigs;
static int	bufaddr[2], ycoordaddr[2];


#define	SYMADDR(idx)	(symtab[idx].val.l)
#define	DSP(x)		(c40_fd[x-1])

/***************************************************************************
 * function to initialize the Hydra
 */
int init_dsp(devname, fname)
char *devname, *fname;
{
	char	devdsp[20];
	int	i, err, rval, dsp;
	int	undef = 0;
	u_long	entry_address;
	struct vc40info vc40info;

	/*
	 * insure proper initialization in case the previous process
	 * attached to the S-32C
	 */
	DSP(1) = open("/dev/vc40a1", O_RDWR);
	if (DSP(1) == -1) {
		fprintf(stderr, "failed to open device `%s'\n", devname);
		perror("Open error");
		return(0);
	}

	/*
	 * get info about the board
	 */
	rval = ioctl(DSP(1), VC40GETINFO, &vc40info);
	if (rval) {
		perror("VC40GETINFO failed");
		return (0);
	}
	numdsp = vc40info.numdsp;

	/*
	 * open remaining DSPs
	 */
	for (dsp=2; dsp<=numdsp; dsp++) {
		sprintf(devdsp, "/dev/vc40a%d", dsp);
		DSP(dsp) = open(devdsp, O_RDWR);
		if (DSP(dsp) == -1) {
			fprintf(stderr, "failed opening %s: ", devdsp);
			perror("");
			return(0);
		}
	}
	/*
	 * load the Mandelbrot engine program to all DSPs
	 */
	for (dsp=1; dsp<=numdsp; dsp++) {
		if (err = c40_load(DSP(dsp), "manddsp.x40", &entry_address, 
				   NUMSYMS, symnames, symtab) == 0) {
			fprintf(stderr, "Error in coffload(): %d\n", cofferr);
			return (0);
		}
	}
	/*
	 * make sure all symbols were found
	 */
	for (i=0; i<NUMSYMS; i++) {
		if (symtab[i].type == T_UNDEF) {
			printf("Symbol `%s' undefined!\n", symnames[i]);
			undef = 1;
		}
	}
	if (undef) {
		return (0);
	}
	bufaddr[0] = SYMADDR(BUF1);
	bufaddr[1] = SYMADDR(BUF2);
	ycoordaddr[0] = SYMADDR(YCOORD1);
	ycoordaddr[1] = SYMADDR(YCOORD2);

	/*
	 * register the signal handler, use routine provided in the
	 * generic code.  Note that since all DSPs use the same vector,
	 * all interrupts will look like they come from DSP 1.
	 */
	register_signal(SIGUSR1, get_mand_line);

	/*
	 * inform all DSPs of the interrupt priority and vector and run
	 * them.
	 */
	for (dsp=1; dsp<=numdsp; dsp++) {
		c40_put_long(DSP(dsp), SYMADDR(INTVEC), vc40info.intvec);
		c40_put_long(DSP(dsp), SYMADDR(INTPRI), vc40info.intpri);
		ioctl(DSP(dsp), VC40RUN, &entry_address);
	}

	return(1);
}

/***************************************************************************
 * function to tidy-up the DSPs on program exit
 */
void close_dsp()
{
	int	dsp;

	for (dsp=1; dsp<=numdsp; dsp++) {
		if (ioctl(DSP(dsp), VC40DSINT) != 0) {
			perror("error disabling interrupts");
		}
		close(DSP(dsp));
	}
}

/***************************************************************************
 * function to initialize a new set calculation
 */
static int	y_coord = -1;
static int	done_drawing = 1;

void draw_mandelbrot_dsp()
{
	PIXEL_TYPE	line[MAXLINE];
	int	dsp;
	int	signal = SIGUSR1;
	int	yheight = ny/numdsp;
	int	ypix = ny;
	int	maxit;
        float	xstep = (xmax - xmin) / nx;
        float	ystep = (ymax - ymin) / ny;
	float	maxmag = 4.0;
	float 	fy = ymin;

	/*
	 * since drawing is done using interrupts, this routine could get
	 * called even when a set is being drawn.  If a set is currently
	 * being drawn, then don't do anything.
	 */
	if (!done_drawing) return;

	maxit = get_maxit();

	/*
	 * initialize set parameters
	 */
	y_coord = ny - 1;
	for (dsp=1; dsp<=numdsp; dsp++) {
		c40_put_dsp_float(DSP(dsp), SYMADDR(XSTART), xmin);
		c40_put_dsp_float(DSP(dsp), SYMADDR(XSTEP), xstep);

		c40_put_dsp_float(DSP(dsp), SYMADDR(YSTART), fy);
		c40_put_dsp_float(DSP(dsp), SYMADDR(YSTEP), ystep);
		fy += yheight * ystep;

		c40_put_dsp_float(DSP(dsp), SYMADDR(MAXVAL), maxmag);
		c40_put_long(DSP(dsp), SYMADDR(MAXIT), maxit);

		c40_put_long(DSP(dsp), SYMADDR(WIDTH), nx);

		if (dsp == numdsp) {
			c40_put_long(DSP(dsp), SYMADDR(HEIGHT), 
				ny - (numdsp - 1)*yheight);
		}
		else {
			c40_put_long(DSP(dsp), SYMADDR(HEIGHT), yheight); 
		}
		c40_put_long(DSP(dsp), SYMADDR(YCOORD), y_coord);
		y_coord -= yheight;

		c40_put_long(DSP(dsp), SYMADDR(START_FLAG), 0);
		if (ioctl(DSP(dsp), VC40ENINT, &signal)!=0) {
			perror("VC40ENINT: ");
			exit(1);
		}
		c40_put_long(DSP(dsp), SYMADDR(START_FLAG), 1);
	}
	y_coord = ny - 1;		/* initialize screen y coordinate */
	numsigs = 0;
}

/***************************************************************************
 * signal handler to read a line of pixels from the S-32C and display it
 */
static void get_mand_line()
{
	static PIXEL_TYPE	line[MAXLINE];
	int	dsp, i;
	u_long	bufnum, yc;

	numsigs++;

	for (dsp=1; dsp<=numdsp; dsp++) {
		c40_get_long(DSP(dsp), SYMADDR(LINE_DONE), &bufnum);
		if (bufnum == 0) continue;

		c40_read_long(DSP(dsp), bufaddr[bufnum-1], line, nx);
		c40_get_long(DSP(dsp), ycoordaddr[bufnum-1], &yc);
		c40_put_long(DSP(dsp), SYMADDR(LINE_DONE), 0);
		y_coord--;
		draw_mandelbrot_line(line, yc);
		if (y_coord < 0) {
			stop_timing();
			done_drawing = 1;
			if (ioctl(DSP(dsp), VC40DSINT) != 0) {
				perror("disabling interrupts: ");
			}
		}
	}
}

