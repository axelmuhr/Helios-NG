/* land_vc40.c
 *
 * DSP-specific code for host-side landscape program
 */
#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <errno.h>
#include "land_dsp.h"
#include "hyhomon.h"

void get_land_data();
static void process_dsp();
static void draw_quadrant();

#define	SYMADDR(idx)	(symtab[idx].val.l)
#define	DSP(x)		(c40_fd[x-1])

/*
 * These defines relate to the symbols in the symbol table extracted from the
 * COFF file.  The numbers refer to the index in the symnames[] array below.
 */
#define NUMSYMS (sizeof(symnames) / sizeof(char *))
#define STEEP       0
#define SEALEVEL    1
#define YBOTTOM     2
#define WATERCOLOR  3
#define LANDCOLOR   4
#define SEED        5
#define START_FLAG  6
#define READY_FLAG  7
#define INTPRI      8
#define INTVEC      9
#define TESTLINE    10
#define LINES       11
#define DEEP	    12
#define	NL          13
#define DONE_FLAG   14
#define IX0         15
#define IY0         16
#define IX2         17
#define IY2         18
#define IZ0         19
#define IZ1         20
#define IZ2         21
#define IZ3         22

/*
 * We'll need the addresses of these symbols from the COFF file
 */
char *symnames[] = {
    "_steep",
    "_sealevel",
    "_ybottom",
    "_WATERCOLOR",
    "_LANDCOLOR",
    "_seed",
    "_start_flag",
    "_ready_flag",
    "_intpri",
    "_intvec",
    "_testline",
    "_lines",
    "_ideep",
    "_nl",
    "_done_flag",
    "_ix0",
    "_iy0",
    "_ix2",
    "_iy2",
    "_iz0",
    "_iz1",
    "_iz2",
    "_iz3",
};
static struct symtab symtab[NUMSYMS];	/* the symbol table */

#define	MAXDSP	4

static int	c40_fd[MAXDSP];	/* file descriptors for each DSP */
static int	numdsp;
static int	xmid, ymid, newz;
static int	quadrant_drawn[4];


#define  XA 0
#define  XB 500
#define  YA 0
#define  ZA 0
#define  YADD 38


struct vc40info vc40info;	/* info about Hydra */
static int dsps_running;	/* number of DSPs currently calculating */

/***************************************************************************
 * function to initialize the Hydra.  This is called just once after the
 * windows have been created and before event processing starts.
 */
int init_dsp(devname, fname)
char *devname, *fname;
{
	char	devdsp[20];
	int	i, err, rval, dsp;
	int	undef = 0;
	u_long	entry_address;
	struct vc40info vc40info;

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
	numdsp = vc40info.numdsp;	/* will be either 2 or 4 */
printf("numdsp: %d\n", numdsp);
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
		ioctl(DSP(dsp), VC40HALT);
		if (err = c40_load(DSP(dsp), "landdsp.x40", &entry_address, 
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

	/*
	 * register the signal handler, use routine provided in the
	 * generic code.  Note that since all DSPs use the same vector,
	 * all interrupts will look like they come from DSP 1.
	 */
	register_signal(SIGUSR1, get_land_data);

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
		c40_put_long(DSP(dsp), SYMADDR(INTVEC), 0);
		c40_put_long(DSP(dsp), SYMADDR(INTPRI), 0);
		close(DSP(dsp));
	}
}

/***************************************************************************
 * function to initialize a new set calculation
 */
void draw_landscape_dsp()
{
	int	i, dsp, depth;
	int	signal = SIGUSR1;
	int	quadnum = 0;
	u_long	seed = random();

	/*
	 * since drawing is done using interrupts, this routine could get
	 * called even when a set is being drawn.  If a set is currently
	 * being drawn, then don't do anything.
	 */
done_drawing = 1;
	if (!done_drawing) return;

	depth = get_depth();
printf("draw landscape!\n");

	printf("depth is: %d\n", depth);

	for (dsp=1; dsp<=numdsp; dsp++) {
		if (ioctl(DSP(dsp), VC40ENINT, &signal) != 0) {
			perror("error enabling interrupts");
			exit(1);
		}
		c40_put_long(DSP(dsp), SYMADDR(INTVEC), vc40info.intvec);
		c40_put_long(DSP(dsp), SYMADDR(INTPRI), vc40info.intpri);
	}

    if (rand() < 16384) {
        newz = ZA + (int)((rand() / 32768.0) * ((ybottom-YA)* steep));
    }
    else {
        newz = ZA - (int)((rand() / 32768.0) * ((ybottom-YA)* steep));
    }
    steep = (frandom() / 2.5) + 0.55;
    sealevel = (int)(17*frandom()) - 8;
    xmid = (XA + XB) / 2;
    ymid = (YA + ybottom) / 2;
    for (i=0; i<4; i++) {
		quadrant_drawn[i] = 0;
    }
    for (dsp=1; dsp<=numdsp; dsp++) {
	draw_quadrant(quadnum, dsp);
	quadrant_drawn[quadnum++] = 1;
    }
    dsps_running = numdsp;
        
}

/***************************************************************************
 * function to get landscape data from a DSP
 */
void get_land_data()
{
	int	dsp;
	u_long	ready_flag;

	/*
	 * check all DSPs for data
	 */
	for (dsp=1; dsp<=numdsp; dsp++) {
		c40_get_long(DSP(dsp), SYMADDR(READY_FLAG), &ready_flag);
		if (ready_flag) {
			process_dsp(dsp);
		}
	}
}

/***************************************************************************
 * process data from a DSP
 */
static void process_dsp(dsp)
int dsp;
{
	u_long	done_flag, nlval;
	int	i, maxx=0, maxy=0, minx=1000, miny=1000;

	c40_get_long(DSP(dsp), SYMADDR(DONE_FLAG), &done_flag);
	if (done_flag) {
		dsps_running--;
		if (dsps_running == 0) {
			done_drawing = 1;
		}
		c40_put_long(DSP(dsp), SYMADDR(READY_FLAG), 0L);
		return;
	}

	c40_get_long(DSP(dsp), SYMADDR(NL), &nlval);
	c40_read_long(DSP(dsp), SYMADDR(LINES), (u_long *)lines, 3*nlval);
	c40_put_long(DSP(dsp), SYMADDR(READY_FLAG), 0L);
	draw_lines(nlval);
}

/***************************************************************************
 * set up drawing for a quadrant
 */
static void draw_quadrant(quadnum, dsp)
int quadnum, dsp;
{
	int	depth = get_depth();

        c40_put_long(DSP(dsp), symtab[INTPRI].val.l, 2L);
        c40_put_long(DSP(dsp), symtab[INTVEC].val.l, 0x50L);
        c40_put_long(DSP(dsp), symtab[YBOTTOM].val.l, (long) ybottom);
        c40_put_long(DSP(dsp), symtab[WATERCOLOR].val.l, (long) watercolor);
        c40_put_long(DSP(dsp), symtab[LANDCOLOR].val.l, (long) landcolor);
        c40_put_long(DSP(dsp), symtab[DEEP].val.l, (long) depth-1);
	draw_quadrant(dsp);
        switch(quadnum) {
            case 1: {
printf("quad 1\n");
                c40_put_long(DSP(dsp), symtab[IX0].val.l, (long) XA);
                c40_put_long(DSP(dsp), symtab[IY0].val.l, (long) YA);
                c40_put_long(DSP(dsp), symtab[IX2].val.l, (long) xmid);
                c40_put_long(DSP(dsp), symtab[IY2].val.l, (long) ymid);
                c40_put_long(DSP(dsp), symtab[IZ0].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ1].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ2].val.l, (long) newz);
                c40_put_long(DSP(dsp), symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(DSP(dsp), symtab[SEED].val.l, random());
                break;
            }

            case 2: {
printf("quad 2\n");
                c40_put_long(DSP(dsp), symtab[IX0].val.l, (long) xmid);
                c40_put_long(DSP(dsp), symtab[IY0].val.l, (long) YA);
                c40_put_long(DSP(dsp), symtab[IX2].val.l, (long) XB);
                c40_put_long(DSP(dsp), symtab[IY2].val.l, (long) ymid);
                c40_put_long(DSP(dsp), symtab[IZ0].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ1].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ2].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ3].val.l, (long) newz);
	        c40_put_long(DSP(dsp), symtab[SEED].val.l, (long) random());
                break;
            }
                
            case 3: {
 printf("quad 3\n");
               c40_put_long(DSP(dsp), symtab[IX0].val.l, (long) XA);
                c40_put_long(DSP(dsp), symtab[IY0].val.l, (long) YA);
                c40_put_long(DSP(dsp), symtab[IX2].val.l, (long) (XA+XB)>>1);
                c40_put_long(DSP(dsp), symtab[IY2].val.l, (long) 
			(YA+ybottom)>>1);
                c40_put_long(DSP(dsp), symtab[IZ0].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ1].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ2].val.l, (long) newz);
                c40_put_long(DSP(dsp), symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(DSP(dsp), symtab[SEED].val.l, (long) random());
                break;
            }
                
            case 4: {
printf("quad 4\n");
                c40_put_long(DSP(dsp), symtab[IX0].val.l, (long) XA);
                c40_put_long(DSP(dsp), symtab[IY0].val.l, (long) YA);
                c40_put_long(DSP(dsp), symtab[IX2].val.l, (long) (XA+XB)>>1);
                c40_put_long(DSP(dsp), symtab[IY2].val.l, (long) 
			(YA+ybottom)>>1);
                c40_put_long(DSP(dsp), symtab[IZ0].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ1].val.l, (long) ZA);
                c40_put_long(DSP(dsp), symtab[IZ2].val.l, (long) newz);
                c40_put_long(DSP(dsp), symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(DSP(dsp), symtab[SEED].val.l, (long) random());
                break;
            }

        }   /* switch */
	c40_put_dsp_float(DSP(dsp), SYMADDR(STEEP), steep);
	c40_put_long(DSP(dsp), SYMADDR(SEALEVEL), sealevel);
	c40_put_long(DSP(dsp), SYMADDR(DONE_FLAG), 0L);
	c40_put_long(DSP(dsp), SYMADDR(START_FLAG), 1L);
}
