/* hydra.h
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "coff.h"

#define	NUM_VC40_ERRORS	(sizeof(c40_error_str) / (sizeof(char *)))

extern int c40_errno;

void c40_error();
int c40_write_long();
char *c40_map_dram();

/*
 * V-C40 driver ioctl()'s
 */
#define	VC40RUN		(_IOW(v, 0, u_long))
#define	VC40RESET	(_IO(v, 1))
#define	VC40GETINFO	(_IOR(v, 2, struct vc40info))
#define	VC40ENINT	(_IOW(v, 3, int))
#define	VC40DSINT	(_IO(v, 4))
#define	VC40HALT	(_IO(v, 5))
#define	VC40SETADDR	(_IOW(v, 6, u_long))
#define	VC40DISKEY	(_IO(v, 7))
#define	VC40ENKEY	(_IO(v, 8))

/*
 * macros so that the ioctl()'s look like functions
 * (for possible portability to the PC world?)
 */
#define	c40_run(fd, addr)	ioctl(fd, VC40RUN, &addr)
#define	c40_reset(fd)		ioctl(fd, VC40RESET)
#define	c40_getinfo(fd, info)	ioctl(fd, VC40GETINFO, info)
#define	c40_enint(fd, signum)	ioctl(fd, VC40ENINT, &signum)
#define	c40_dsint(fd)		ioctl(fd, VC40DSINT)
#define	c40_halt(fd)		ioctl(fd, VC40HALT)

/*
 * user structures
 */
struct vc40info {
	u_long	intpri;	/* unit's interrupt priority */
	u_long	intvec;	/* unit's interrupt vector */
	u_long	dram_base;	/* board's DRAM base physical address (VME) */
	u_long	dram_size;	/* board's DRAM size (in bytes!) */
	u_long	numdsp;		/* number of DSPs present (2 or 4) */
};

void c40_error();
int c40_write_long();
int c40_read_long();
int c40_get_long();
int c40_put_long();
u_long ieee2dsp();
float dsp2ieee();
int c40_get_float();
int c40_put_float();
int c40_get_dsp_float();
int c40_put_dsp_float();
