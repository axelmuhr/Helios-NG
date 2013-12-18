/* c40util.c
 *
 * utility functions
 */
#include <stdio.h>
#include "hyhomon.h"

int	c40_errno;	/* error number used like errno */

char *c40_error_str[] = {
	"No V-C40 Errors yet!"
};



/****************************************************************************
 * c40_error()
 *
 * report V-C40 error in manner of perror()
 */
void c40_error(str)
{
	if (c40_errno < 1000) {	/* standard system errors */
		perror(str);
	}
	else {
		fprintf(stderr, "%s: ", str);
		if (c40_errno-1000 < NUM_VC40_ERRORS) {
			fprintf(stderr, "%s\n",
				c40_error_str[c40_errno-1000]);
		}
		else {
			fprintf(stderr, "Unkown error %d\n", c40_errno);
		}
	}
}
		

/****************************************************************************
 * c40_write_long()
 *
 * function used by COFF loader to download programs to Hydra
 */
int c40_write_long(c40_fd, dsp_addr, buf, wcnt)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to write buffer to */
u_long *buf;		/* buffer of longs to write to DSP */
u_int	wcnt;		/* number of longs to write to DSP */
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = write(c40_fd, buf, sizeof(u_long)*wcnt);
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_read_long()
 *
 */
int c40_read_long(c40_fd, dsp_addr, buf, wcnt)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
u_long *buf;		/* buffer of longs to read from DSP */
u_int	wcnt;		/* number of longs to read from DSP */
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = read(c40_fd, buf, sizeof(u_long)*wcnt);
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_get_long()
 *
 */
int c40_get_long(c40_fd, dsp_addr, lval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
u_long *lval;
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = read(c40_fd, lval, sizeof(u_long));
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_put_long()
 *
 */
int c40_put_long(c40_fd, dsp_addr, lval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
u_long lval;
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = write(c40_fd, &lval, sizeof(u_long));
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_get_float()
 *
 * warning: this routine requires that the DSP first converts its internal
 * floating point format to IEEE format!  See also c40_get_dsp_float().
 */
int c40_get_float(c40_fd, dsp_addr, fval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float *fval;
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = read(c40_fd, fval, sizeof(float));
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_put_float()
 *
 * warning: this function requires the the DSP convert the IEEE float to
 * its internal format before it can be used!  See also c40_put_dsp_float().
 */
int c40_put_float(c40_fd, dsp_addr, fval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float fval;
{
	int	rcode;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = write(c40_fd, &fval, sizeof(float));
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_get_dsp_float()
 *
 * this function gets a floating point value from the DSP and converts it to
 * IEEE format.
 */
int c40_get_dsp_float(c40_fd, dsp_addr, fval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float *fval;
{
	int	rcode;
	u_long	lval;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	rcode = read(c40_fd, &lval, sizeof(long));
	*fval = dsp2ieee(lval);
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * c40_put_dsp_float()
 *
 * This function converts the given floating point value to the DSP's format
 * and writes it to the DSP.
 */
int c40_put_dsp_float(c40_fd, dsp_addr, fval)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float fval;
{
	int	rcode;
	u_long	lval;

	rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);
	if (rcode == -1) {
		c40_errno = errno;
		return (-1);
	}
	lval = ieee2dsp(fval);
	rcode = write(c40_fd, &lval, sizeof(float));
	c40_errno = errno;
	return (rcode);
}

/****************************************************************************
 * IEEE float / 2s complement float conversion functions
 */
union lorf {
	long	l;
	float	f;
};
u_long ieee2dsp(ieee)
float ieee;
{
	u_long	dspnum;
	u_long	lieee;			/* IEEE float as a long */
	u_long	eieee, sieee, fieee;	/* IEEE exponent, sign, mantissa */
	long	edsp, sdsp, fdsp;	/* DSP exponent, sign, mantissa */
	union lorf	cvt;

	cvt.f = ieee;
	lieee = cvt.l;

	eieee = (lieee & 0x7f800000) >> 23;
	sieee = ((lieee & 0x80000000) >> 31) & 1;
	fieee = (lieee & 0x007fffff);

	if (eieee == 0) {			/* case 6 */
		edsp = 0x80;
		sdsp = 0;
		fdsp = 0;
	}
	else if (eieee == 255 && sieee == 1) {	/* case 1 */
		edsp = 0x7f;
		sdsp = 1;
		fdsp = 0;
	}
	else if (eieee == 255 && sieee == 0) {	/* case 2 */
		edsp = 0x7f;
		sdsp = 0;
		fdsp = 0x7fffff;
	}
	else if (sieee == 0) {			/* case 3 */
		edsp = eieee - 0x7f;
		sdsp = 0;
		fdsp = fieee;
	}
	else if (sieee == 1 && fieee != 0) {	/* case 4 */
		edsp = eieee - 0x7f;
		sdsp = 1;
		fdsp = (~fieee) + 1;
	}
	else if (sieee == 1 && fieee == 0) {	/* case 5 */
		edsp = eieee - 0x80;
		sdsp = 1;
		fdsp = 0;
	}
	else {
		fprintf(stderr, "ieee2dsp -- unknown case!!!\n");
		exit(1);
	}

	dspnum = ( (edsp << 24) | (sdsp << 23) | (fdsp & 0x7fffff) );

	return (dspnum);
}

float dsp2ieee(dspnum)
long dspnum;
{
	u_long	lieee;			/* IEEE float as a long */
	u_long	eieee, sieee, fieee;	/* IEEE exponent, sign, mantissa */
	long	edsp, sdsp, fdsp;	/* DSP exponent, sign, mantissa */
	union	lorf cvt;

	edsp = (dspnum >> 24);		/* need sign extension here! */
	sdsp = (dspnum >> 23) & 1;
	fdsp = (dspnum & 0x7fffff);

	if (edsp == -128 || edsp == -127) {		/* case 1 and 2 */
		eieee = 0;
		sieee = 0;
		fieee = 0;
	}
	else if (edsp == 127 && sdsp == 1 && fdsp == 0) {	/* case 6 */
		eieee = 0xff;
		sieee = 1;
		fieee = 0;
	}
	else if (sdsp == 0) {		/* case 3 */
		eieee = edsp + 0x7f;
		sieee = 0;
		fieee = fdsp;
	}
	else if (sdsp == 1 && fdsp != 0) {	/* case 4 */
		eieee = edsp + 0x7f;
		sieee = 1;
		fieee = (~fdsp) + 1;
	}
	else if (sdsp == 1 && fdsp == 0) {	/* case 5 */
		eieee = edsp + 0x7e;
		sieee = 1;
		fieee = 0;
	}
	else {
		fprintf(stderr, "dsp2ieee -- unknow case!!!\n");
		exit(1);
	}
	lieee = ( (sieee << 31) | ((eieee & 0xff) << 23) | (fieee & 0x7fffff) );
	cvt.l = lieee;
	return (cvt.f);
}

/****************************************************************************
 * c40_map_dram()
 *
 * maps in Hydra's DRAM to the user's virtual address space
 */
char *c40_map_dram(c40_fd, offs, len)
int c40_fd;	/* unit's file descriptor */
u_long offs;	/* offset from the beginning of the unit's DRAM to start */
u_long len;	/* number of *bytes* to map */
{
	char *vpstart;
	u_long psize = getpagesize();	/* get system's page size */
	u_long pstart, poff;

	/*
	 * calculate beginning of nearest page, and offset from beginning
	 * of page.
	 */
	pstart = (offs / psize) * psize;
	poff = offs % psize;

	/*
	 * map the DRAM in
	 */
printf("mapping: pstart: 0x%x; len: %d; offset: 0x%x\n", pstart, len, poff);
	vpstart = (char *) mmap((caddr_t)0, len, PROT_READ|PROT_WRITE,
		MAP_SHARED, c40_fd, pstart);
	if (vpstart == (char *)(-1)) {
		return ((char *)-1);
	}

	/*
	 * return virtual address
	 */
	return (vpstart + poff);
}
