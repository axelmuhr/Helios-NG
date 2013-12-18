/* vc40priv.c
 *
 * V-C40 Hydra driver private functions.
 */
#include "vc40drv.h"

/**************************************************************************
 * get_property()
 *
 * reads an EEPROM value from Hydra via the monitor on DSP 1
 */
int get_property(vic, prop_id, prop_val)
struct vc40_vic *vic;
int prop_id;
u_long *prop_val;
{
	int	i;
	u_long	pval = 0;
	u_long	tlong;

	vic->CTRL_ICR = GetProperty;	/* write command word */
	vic->DSP1_SET_INTR = 0;		/* issue interrupt */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for cmd ack */
	vic->CTRL_ICR = prop_id;	/* write the property ID */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for ID ack */

	/*
	 * read the property value, a byte at time, LSB first
	 */
	for (i=0; i<4; i++) {
		pval = pval >> 8;
		tlong = vic->DATA_ICR;	/* read byte */
		vic->CTRL_ICR = 1;	/* signal for next byte */
		pval |= (tlong << 24);	/* `or' into MSB */
		CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for next byte */
	}
	*prop_val = pval;	/* return property value */
	DEBUG_MSG9(logit, "get_property: ID: %d; value: %d (0x%x)\n",
		prop_id, pval, pval);
	return (0);		/* success! */
}

/**************************************************************************
 * set_property()
 *
 * writes a property value to the EEPROM.  Note that properties set in this
 * fashion do not take effect until DSP 1 is reset.
 */
int set_property(vic, prop_id, prop_val)
struct vc40_vic *vic;
int prop_id;
u_long prop_val;
{
	int	i;
	u_long	tpval = prop_val;

	vic->CTRL_ICR = SetProperty;	/* write command word */
	vic->DSP1_SET_INTR = 0;		/* issue interrupt */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for cmd ack */
	vic->CTRL_ICR = prop_id;	/* write property ID */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for ID ack */

	/*
	 * write the property, a byte at a time, LSB first
	 */
	for (i=0; i<4; i++) {
		vic->DATA_ICR = prop_val & 0xff;	/* write byte */
		vic->CTRL_ICR = 1;			/* signal byte ready */
		tpval = tpval << 8;			/* next byte */
		CMD_HANDSHAKE(vic->CTRL_ICR);		/* wait for byte ACK */
	}
#ifdef DEBUG
	/*
	 * read property back to confirm write
	 */
	{
		int	rcode;
		u_long	pval;

		rcode = get_property(vic, prop_id, &pval);
		if (rcode) return (rcode);
		if (pval != prop_val) {
			ERROR_MSG(logit, 
"ERROR: set_property: ID: %d; wrote 0x%x; read 0x%x\n", 
prop_id, prop_val, pval);
			return (EIO);
		}
	}
#endif /* DEBUG */

	return (0);		/* success! */
}
		
/**************************************************************************
 * vc40_mapin()
 * vc40_mapout()
 *
 * these routines map in kernel resources for allocating virtual address
 * space when loading the driver, and release the resources when unloading.
 */
u_long *vc40_mapin(paddr, npages)
u_long paddr;
int npages;
{
	u_long	pnum, poff, kmx;
	u_long	psize = SYSTEM_PAGE_SIZE;
	u_long	*vaddr;

	pnum = (paddr / psize);		/* physical page number */
	poff = paddr % psize;		/* offset from beginning of page */
	kmx = rmalloc(kernelmap, npages);	/* allocate virtual pages */
	mapin(&Sysmap[kmx], btoc(Sysbase) + kmx, pnum | PGT_VME_D32, npages,
		PG_V | PG_KW);
	vaddr = (u_long *)( (u_char *)kmxtob(kmx) + poff);
	return (vaddr);
}

void vc40_mapout(vaddr, npages)
u_long *vaddr;
int npages;
{
	u_long	kmx;

	kmx = btokmx((struct pte *) vaddr);
	mapout(&Sysmap[kmx], npages);
	rmfree(kernelmap, npages, kmx);
}

/**************************************************************************
 * vc40_minphys()
 *
 * sets the maximum I/O block size.  The kernel will divide up read()'s and
 * write()'s into blocks of (at most) this size.
 */
void vc40_minphys(bp)
struct buf *bp;
{
	if (bp->b_bcount > IOBUF_SIZE) {
		bp->b_bcount = IOBUF_SIZE;
	}
}

/**************************************************************************
 * vc40_strategy()
 *
 * this routine implements the read() and write() strategy.
 * This strategy makes use of HydraMon's CopyStuff command to copy data into
 * and out of the various memory banks on the Hydra.
 *
 * The best way to do this would be to inform HydraMon of the physical address
 * of the user's memory block, and let Hydra do the work.  For now, the
 * quick and dirty way is to copy the data to Hydra's memory and deal with
 * it from there.
 */
void vc40_strategy(bp)
register struct buf *bp;
{
	register int	unit = VC40_UNIT(bp->b_dev);
	register int	wcnt = bp->b_bcount/4;
	register int	rcode;
	register u_long	*ubuf = (u_long *)bp->b_un.b_addr;
	register u_long	*dram_io = vc40_units[unit]->xfr_data;
	register struct hydra_cmd *hcmd = vc40_units[unit]->hydra_cmd;

	/*
	 * set up the copy command
	 */
	hcmd->cmd = CopyStuff;
	hcmd->param.copy_cmd.len = wcnt;
	if (bp->b_flags & B_READ) {	/* read */
		hcmd->param.copy_cmd.src = vc40_units[unit]->io_addr;
		hcmd->param.copy_cmd.dst = vc40_units[unit]->dsp_dram_io;
DEBUG_MSG9(logit, "strategy (read): src: 0x%x; dst: 0x%x; wcnt: %d\n",
hcmd->param.copy_cmd.src, hcmd->param.copy_cmd.dst, hcmd->param.copy_cmd.len);
		rcode = vc40_read_block(unit, dram_io, ubuf, wcnt); 
	}
	else {	/* write */
		hcmd->param.copy_cmd.src = vc40_units[unit]->dsp_dram_io;
		hcmd->param.copy_cmd.dst = vc40_units[unit]->io_addr;
DEBUG_MSG9(logit, "strategy (write): src: 0x%x; dst: 0x%x; wcnt: %d\n",
hcmd->param.copy_cmd.src, hcmd->param.copy_cmd.dst, hcmd->param.copy_cmd.len);
		rcode = vc40_write_block(unit, dram_io, ubuf, wcnt); 
	}
	vc40_units[unit]->io_addr += wcnt; /* update address for next block */
	if (rcode) {
		bp->b_error = rcode;
		bp->b_flags |= B_ERROR;
	}
	iodone(bp);
}

/**************************************************************************
 * vc40_read_block()
 *
 * this function reads a block from a DSP using HydraMon
 */
int vc40_read_block(unit, dram_io, ubuf, wcnt)
register int unit;
register u_long *dram_io;
register u_long *ubuf;
register int wcnt;
{
	int	rcode;

	/*
	 * strategy() has already written the command block to the DRAM
	 * I/O control buffer, so let the command fly and then copy
	 * the data from the DRAM I/O buffer to the user's buffer
	 */
	rcode = vc40_cmd_interrupt(unit);	/* fire command */
	if (rcode) return (rcode);

	/*
	 * copy data from DRAM I/O buffer to user's buffer
	 */
	while (wcnt--) {
		*ubuf++ = *dram_io++;
	}

	return (0);
}

/**************************************************************************
 * vc40_write_block()
 *
 * this function writes a block to a DSP using HydraMon
 */
int vc40_write_block(unit, dram_io, ubuf, wcnt)
register int unit;
register u_long *dram_io;
register u_long *ubuf;
register int wcnt;
{
	int	rcode;

	/*
	 * copy data from user's buffer to DRAM I/O buffer
	 */
	while (wcnt--) {
		*dram_io++ = *ubuf++;
	}

	/*
	 * strategy() has already written the command block to the DRAM
	 * I/O control buffer, so let the command fly.
	 */
	rcode = vc40_cmd_interrupt(unit);	/* fire command */

	return (rcode);
}


/**************************************************************************
 * vc40_cmd_interrupt()
 *
 * this function fires a command on the DSP via HydraMon and waits for the
 * command to finish.  The command must already be setup in the DRAM
 * command control buffer.
 */
int vc40_cmd_interrupt(unit)
register int unit;
{
	register int	dspno = DSP(unit);
	register struct hydra_cmd *hcmd = vc40_units[unit]->hydra_cmd;
	static u_long	dspnmi[] = {0,  MCR_D2N, MCR_D3N, MCR_D4N};

	/*
	 * issue the interrupt to the DSP
	 */
	if (dspno == 0) {		/* DSP 1 is a special case */
		vc40_units[unit]->vic->DSP1_SET_INTR = 0;
	}
	else {
		*(vc40_units[unit]->mcr) |= dspnmi[dspno];
		*(vc40_units[unit]->mcr) &= ~dspnmi[dspno];
		*(vc40_units[unit]->mcr) |= dspnmi[dspno];
	}

	/*
	 * when (if) command completes, HydraMon will write `Success'
	 * over the command word
	 */
	CDELAY( ((hcmd->cmd) == Success), 10000);
	if (hcmd->cmd != Success) {
		ERROR_MSG(logit, 
			"vc40dsp: strategy failed for DSP %d (cmd is %d)!\n", 
			DSP(unit)+1, hcmd->cmd);
printf("vc40dsp: strategy failed for DSP %d (cmd is %d)!\n", 
			DSP(unit)+1, hcmd->cmd);

		return (EIO);
	}
	return (0);
}

/**************************************************************************
 * vc40_reset()
 *
 * this function will reset and reboot a DSP via DSP 1.  Currently, DSP 1
 * cannot be reset or rebooted.
 */
int vc40_reset(unit)
int unit;
{
	static u_long	dspsel[] = {3, 0, 1, 2};
	int	unit1 = UNIT1(unit);
	int	rcode;
	u_long	dsp = DSP(unit);
	struct hydra_cmd *hcmd1 = vc40_units[unit1]->hydra_cmd;

	if (dsp == 0) {
		DEBUG_MSG9(logit, 
			"vc40_reset: ingnoring reset request for DSP 0\n");
		return (0);
	}
	hcmd1->cmd = BootADsp;
	hcmd1->param.boot_cmd.dspnum = dspsel[dsp];
	rcode = vc40_cmd_interrupt(unit1);
	return (rcode);
}
