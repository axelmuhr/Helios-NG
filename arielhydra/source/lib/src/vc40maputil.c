/* c40util.c
 *
 * utility functions
 */
#include <stdio.h>
#include "vc40map.h"

int	c40_errno;	/* error number used like errno */

char *c40_error_str[] = {
	"No V-C40 Errors yet!"
};


/**************************************************************************
 * vc40_cmd_interrupt()
 *
 * this function fires a command on the DSP via HydraMon and waits for the
 * command to finish.  The command must already be setup in the DRAM
 * command control buffer.
 */
int vc40_cmd_interrupt(unit)
int unit;
{
	register int	dspno = DSP(unit);
	register struct hydra_cmd *hcmd;
	int	i;
	static u_long	dspnmi[] = {0,  MCR_D2N, MCR_D3N, MCR_D4N};

	unit--;
	hcmd = hydras[unit].hydra_cmd;
	/*
	 * issue the interrupt to the DSP
	 */
	if (dspno == 0) {		/* DSP 1 is a special case */
		hydras[unit].vic->DSP1_SET_INTR = 0;
	}
	else {
		*(hydras[unit].mcr) |= dspnmi[dspno];
		*(hydras[unit].mcr) &= ~dspnmi[dspno];
		*(hydras[unit].mcr) |= dspnmi[dspno];
	}

	/*
	 * when (if) command completes, HydraMon will write `Success'
	 * over the command word
	 */
	for (i=0; i<1000000 && (hcmd->cmd) != Success; i++ );
	if (hcmd->cmd != Success) {
		fprintf(stderr, "Command interrupt %d (0x%x) failed for DSP %d\n", 
			hcmd->cmd, hcmd->cmd, unit+1);
		return (EIO);
	}
	return (0);
}

/****************************************************************************
 * c40_write_long()
 *
 * function used by COFF loader to download programs to Hydra
 */
int c40_write_long(vc40, dsp_addr, buf, wcnt)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to write buffer to */
u_long *buf;		/* buffer of longs to write to DSP */
u_int	wcnt;		/* number of longs to write to DSP */
{
	int	i, rcode;
	int	blkcnt;
	u_long	*dram_xfr;
	struct hydra_cmd *hcmd = hydras[vc40-1].hydra_cmd;

	while (wcnt > 0) {
		blkcnt = (wcnt > IOBUF_SIZE/4) ? IOBUF_SIZE/4 : wcnt;
		dram_xfr = hydras[vc40-1].xfr_data;
		hcmd->cmd = CopyStuff;
		hcmd->param.copy_cmd.src = hydras[vc40-1].dsp_dram_io;
		hcmd->param.copy_cmd.dst = dsp_addr;
		hcmd->param.copy_cmd.len = blkcnt;
		for (i=0; i<blkcnt; i++) {
			*dram_xfr++ = *buf++;
		}
		rcode = vc40_cmd_interrupt(vc40);
		if (rcode) return (rcode);
		dsp_addr += blkcnt;
		wcnt -= blkcnt;
	}
		
	return (0);
}

/****************************************************************************
 * c40_read_long()
 *
 */
int c40_read_long(vc40, dsp_addr, buf, wcnt)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to write buffer to */
u_long *buf;		/* buffer of longs to write to DSP */
u_int	wcnt;		/* number of longs to write to DSP */
{
	int	i, rcode;
	int	blkcnt;
	u_long	*dram_xfr;
	struct hydra_cmd *hcmd = hydras[vc40-1].hydra_cmd;

	while (wcnt > 0) {
		blkcnt = (wcnt > IOBUF_SIZE/4) ? IOBUF_SIZE/4 : wcnt;
		dram_xfr = hydras[vc40-1].xfr_data;
		hcmd->cmd = CopyStuff;
		hcmd->param.copy_cmd.src = dsp_addr;
		hcmd->param.copy_cmd.dst = hydras[vc40-1].dsp_dram_io;
		hcmd->param.copy_cmd.len = blkcnt;
		rcode = vc40_cmd_interrupt(vc40);
		for (i=0; i<blkcnt; i++) {
			*buf++ = *dram_xfr++;
		}
		if (rcode) return (rcode);
		dsp_addr += blkcnt;
		wcnt -= blkcnt;
	}
		
	return (0);
}

/****************************************************************************
 * c40_get_long()
 *
 */
int c40_get_long(vc40, dsp_addr, lval)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
u_long *lval;
{
	int	rcode;

	rcode = c40_read_long(vc40, dsp_addr, lval, 1);
	return (rcode);
}

/****************************************************************************
 * c40_put_long()
 *
 */
int c40_put_long(vc40, dsp_addr, lval)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
u_long lval;
{
	int	rcode;

	rcode = c40_write_long(vc40, dsp_addr, &lval, 1);
	return (rcode);
}

/****************************************************************************
 * c40_get_float()
 *
 * warning: this routine requires that the DSP first converts its internal
 * floating point format to IEEE format!  See also c40_get_dsp_float().
 */
int c40_get_float(vc40, dsp_addr, fval)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float *fval;
{
	int	rcode;

	rcode = c40_read_long(vc40, dsp_addr, fval, 1);
	return (rcode);
}

/****************************************************************************
 * c40_put_float()
 *
 * warning: this function requires the the DSP convert the IEEE float to
 * its internal format before it can be used!  See also c40_put_dsp_float().
 */
int c40_put_float(vc40, dsp_addr, fval)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float fval;
{
	int	rcode;

	rcode = c40_write_long(vc40, dsp_addr, &fval, 1);
	return (rcode);
}

/****************************************************************************
 * c40_get_dsp_float()
 *
 * this function gets a floating point value from the DSP and converts it to
 * IEEE format.
 */
int c40_get_dsp_float(vc40, dsp_addr, fval)
int vc40;		/* file descriptor */
u_long dsp_addr;	/* DSP address */
float *fval;
{
	int	rcode;
	u_long	lval;

	rcode = c40_read_long(vc40, dsp_addr, &lval, 1);
	*fval = dsp2ieee(lval);
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

	lval = ieee2dsp(fval);
	rcode = c40_write_long(c40_fd, dsp_addr, &lval, 1);
	return (rcode);
}

/****************************************************************************
 * VC40RUN
 */
int c40_run(vc40, addr)
int vc40;
u_long addr;
{
	int	rcode;
	struct hydra_cmd *hcmd = hydras[vc40-1].hydra_cmd;

	hcmd->cmd = Run;
	hcmd->param.run_cmd.addr = addr;
	rcode = vc40_cmd_interrupt(vc40);
	return (rcode);
}

/****************************************************************************
 * VC40HALT
 */
int c40_halt(vc40)
int vc40;
{
	int	rcode;
	struct hydra_cmd *hcmd = hydras[vc40-1].hydra_cmd;

	hcmd->cmd = Halt;
	rcode = vc40_cmd_interrupt(vc40);
	return (rcode);
}

/****************************************************************************
 * VC40RESET
 */
int c40_reset(vc40)
int vc40;
{
        static u_long   dspsel[] = {3, 0, 1, 2};
	int	rcode;
	int	dsp1 = UNIT1(vc40);
	int	dspno = DSP(vc40);
	struct hydra_cmd *hcmd = hydras[vc40-1].hydra_cmd;

	if (dspno == 0) return (0);	

	hcmd->cmd = BootADsp;
	hcmd->param.cmd.param1 = dspsel[dspno];
	rcode = vc40_cmd_interrupt(dsp1);
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

