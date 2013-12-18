/* vc40top.c
 *
 * driver "top-half" functions
 */

#include "vc40drv.h"

/***************************************************************************
 * open() system call
 */
int vc40_open(dev, flags)
dev_t dev;
int flags;
{
	int	unitn = VC40_UNIT(dev);
	int	unit1 = UNIT1(unitn);
	int	status;
	struct vc40_units_struct *vc40unitn = vc40_units[unitn];

	/*
	 * make sure structures have been allocated (if not, then this
	 * unit was never attached)
	 */
	if (!vc40unitn) {
		ERROR_MSG(logit, "vc40dsp: unit %d does not exist\n", unitn);
		return (ENODEV);
	}

	/*
	 * make sure open() is allowed now
	 */
	if (vc40_units[unit1]->open_inhibit || vc40unitn->open_inhibit) {
		ERROR_MSG(logit, "vc40dsp: open() inhibited on unit %d\n", 
			unitn);
		return (EIO);
	}

	/*
	 * unit must be opened for read and write
	 */
	if ((!(flags & FREAD)) || (!(flags & FWRITE))) {
		ERROR_MSG(logit, 
		"vc40dsp: must open() with R/W access on unit %d\n", unitn);
		return (EIO);
	}

	/*
 	 * only perform initialization if the unit is not currently open
	 * and not currently attached
	 */
	if (!vc40unitn->unit_open && vc40unitn->attach_code == VC40_DETACHED) {
		status = vc40_reset(unitn);
		if (status) return (status);

		/*
		 * mark this unit as open and turn on the red LED
		 */
		vc40_units[unitn]->unit_open = 1;

#		ifdef DEBUG
		*(vc40_units[unitn]->mcr) |= MCR_L2;
#		endif
	}

	return (0);	/* success */
}

/***************************************************************************
 * close() system call.
 *
 * Note that while vc40_open() is called for every open() on the unit,
 * vc40_close() is called only after the last close().
 */
int vc40_close(dev, flags)
dev_t dev;
int flags;
{
	int	unit = VC40_UNIT(dev);

	if (!vc40_units[unit]) {	/* a "can't happen" */
		ERROR_MSG(logit, 
"vc40dsp: vc40_close() called on an un-attached unit, %d\n", unit);
		return (ENODEV);
	}

	/*
	 * no more interrupts
	 */
	vc40_units[unit]->usignum = 0;
	vc40_units[unit]->uproc = NULL;

	/*
	 * turn off red LED
	 */
#ifdef DEBUG
	*(vc40_units[unit]->mcr) &= ~MCR_L2;
#endif

	vc40_units[unit]->unit_open = 0;

	return (0);
}

/***************************************************************************
 * write() system call
 */
vc40_write(dev, uio)
dev_t dev;
struct uio *uio;
{
	int	unit = VC40_UNIT(dev);
	int	rcode;

	/*
	 * enforce a word count - write specifies its length in _bytes_
	 */
	if (uio->uio_iov->iov_len & 0x3) {
		ERROR_MSG(logit, 
"vc40dsp: write() byte count of %d not suitable for *word* transfer\n",
 uio->uio_iov->iov_len);
		return (EINVAL);
	}

	/*
	 * The VC40SETADDR ioctl() must be called before read() or write()
	 * in order to set the DSPs transfer address.  lseek() cannot be
	 * used since the Hydra's addresses require all 32 bits (i.e. an
	 * unsigned long) whereas lseek() uses signed longs, and the system
	 * will not allow an absolute address < 0 (write() will return
	 * the error Invalid Argument).
	 */

	/*
	 * start the write
	 */
	rcode = physio(vc40_strategy, &(vc40_units[unit]->buf), dev, B_WRITE,
		vc40_minphys, uio);
	return (rcode);
}

/***************************************************************************
 * read() system call
 */
vc40_read(dev, uio)
dev_t dev;
struct uio *uio;
{
	int	unit = VC40_UNIT(dev);
	int	rcode;

	/*
	 * enforce a word count - write specifies its length in _bytes_
	 */
	if (uio->uio_iov->iov_len & 0x3) {
		ERROR_MSG(logit, 
"vc40dsp: read() byte count of %d not suitable for *word* transfer\n",
 uio->uio_iov->iov_len);
		return (EINVAL);
	}

	/*
	 * The VC40SETADDR ioctl() must be called before read() or write()
	 * in order to set the DSPs transfer address.  lseek() cannot be
	 * used since the Hydra's addresses require all 32 bits (i.e. an
	 * unsigned long) whereas lseek() uses signed longs, and the system
	 * will not allow an absolute address < 0 (write() will return
	 * the error Invalid Argument).
	 */

	/*
	 * start the read
	 */
	rcode = physio(vc40_strategy, &(vc40_units[unit]->buf), dev, B_READ,
		vc40_minphys, uio);
	return (rcode);
}

/***************************************************************************
 * mmap() system call
 */
#ifdef MAP_ALL_DRAM
int vc40_mmap(dev, off, prot)
dev_t dev;
off_t off;
int prot;
{
	int unitn = VC40_UNIT(dev);
	int unit1 = UNIT1(unitn);	/* DSP 1 of this Hydra */
	int kpfnum;

	/*
	 * check that offset is within the DRAM's size
	 */
DEBUG_MSG3(logit, "mmap: off is 0x%x\n", off);
	if ( (u_long) off > vc40_units[unit1]->dram_size) {
		return (-1);
	}

	/*
	 * return the page frame number to the kernel
	 */
	kpfnum = hat_getkpfnum(vc40_units[unit1]->dram_vaddr + off);
DEBUG_MSG3(logit, "mmap: kpfnum is %d; addr: 0x%x\n", kpfnum,
vc40_units[unit1]->dram_paddr + off);
	return (kpfnum);
}

#endif /* MAP_ALL_DRAM */

/***************************************************************************
 * ioctl() system call
 */
int vc40_ioctl(dev, cmd, arg, flag)
dev_t dev;
int cmd;
caddr_t arg;
int flag;
{
	int unitn = VC40_UNIT(dev);	/* unit for given DSP */
	int unit1 = UNIT1(unitn);	/* unit for DSP 1 of board */
	int rcode = 0;
	struct hydra_cmd *hcmdn = vc40_units[unitn]->hydra_cmd;
	struct hydra_cmd *hcmd1 = vc40_units[unit1]->hydra_cmd;

	switch(cmd) {

	/*-----------------------------------------------------------------
	 * VC40RUN
	 *
	 * tell a DSP to begin execution at the given address
	 *	ioctl(fd, VC40RUN, addr)
	 *		int fd;
	 * 		u_long *addr;
	 */
	case VC40RUN: {
		hcmdn->cmd = Run;
		hcmdn->param.run_cmd.addr = *(u_long *) arg;
		rcode = vc40_cmd_interrupt(unitn);
		break;
	} /* VC40RUN */
		
	/*-----------------------------------------------------------------
	 * VC40RESET
	 *
	 * boot a DSP via DSP 1.  The file descriptor must indicate the DSP
	 * to be booted.  Booting DSP 1 is illegal.
	 *	ioctl(fd, VC40RESET)
	 *		int fd;
 	 */
	case VC40RESET: {
		rcode = vc40_reset(unitn);
		break;
	} /* VC40RESET */

	/*-----------------------------------------------------------------
	 * VC40GETINFO
	 *
	 * returns important information:
	 *	unit's interrupt priority (as defined in the config file)
	 *	unit's interrupt vector (as defined in the config file)
	 *	board's DRAM size (as defined in EEPROM)
	 *	board's DRAM base address (as defined in EEPROM)
	 *	number of DSP's on the board (determined by the presence of
	 *		a daughter card).
	 */
	case VC40GETINFO: {
		struct vc40info *vc40info = (struct vc40info *)arg;

		vc40info->intpri = vc40_units[unitn]->intpri;
		vc40info->intvec = vc40_units[unitn]->intvec;
		vc40info->dram_base = vc40_units[unitn]->dram_paddr;
		vc40info->dram_size = vc40_units[unitn]->dram_size;
		vc40info->numdsp = 2*(vc40_units[unit1]->dcard + 1);

		break;

	} /* VC40GETINFO */

	/*-----------------------------------------------------------------
	 * VC40ENINT
	 *
	 * Enables interrupts from the V-C40.  When an interrupt is received,
	 * the requested signal will be sent to the user's process.  HyHoMon
	 * is informed of the interrupt vector and priority thus enabling the
	 * host interrupt trap.  If the config file does not specify the
	 * interrupt priority and vector for a DSP, then those for DSP 1 are
	 * used.
	 */
	case VC40ENINT: {
		int	usignum = *(int *)arg;
		int	intvec, intpri;

		/*
		 * record the PID and signal number
		 */
		vc40_units[unitn]->usignum = usignum;
		vc40_units[unitn]->uproc = u.u_procp;	/* from user struct */

		/*
		 * check for a selected vector and priority
	 	 */
		intvec = vc40_units[unitn]->intvec;
		intpri = vc40_units[unitn]->intpri;
		if (intvec == 0 || intpri == 0) {
			rcode = EINVAL;
			ERROR_MSG(logit, 
"vc40dsp: no interrupt vector or priority specified for unit %d\n", unitn);
			break;
		}
		/*
		 * inform DSP of its interrupt priority and vector
		 */
		hcmdn->cmd = HostIntNumber;
		hcmdn->param.cmd.param1 = intpri;
		rcode = vc40_cmd_interrupt(unitn);
		if (rcode) break;

		hcmdn->cmd = HostIntVector;
		hcmdn->param.cmd.param1 = intvec;
		rcode = vc40_cmd_interrupt(unitn);

		break;
	} /* VC40ENINT

	/*-----------------------------------------------------------------
	 * VC40DSINT
	 *
	 * disables interrupts from the V-C40.  Signals will no longer be
	 * delivered to user process.
	 */
	case VC40DSINT: {

		/*
		 * deregister the PID and signal number
		 */
		vc40_units[unitn]->usignum = 0;
		vc40_units[unitn]->uproc = NULL;

		/*
		 * deregister the interrupt priority and vecotor
		 */
		hcmdn->cmd = HostIntNumber;
		hcmdn->param.cmd.param1 = 0;
		rcode = vc40_cmd_interrupt(unitn);
		if (rcode) break;

		hcmdn->cmd = HostIntVector;
		hcmdn->param.cmd.param1 = 0;
		rcode = vc40_cmd_interrupt(unitn);

		break;
	} /* VC40DSINT

	/*-----------------------------------------------------------------
	 * VC40HALT
	 *
	 * Halts a DSP by throwing it into a dead loop in HydraMon.
	 */
	case VC40HALT: {
		hcmdn->cmd = Halt;
		rcode = vc40_cmd_interrupt(unitn);
		break;		
	} /* VC40HALT */

	/*-----------------------------------------------------------------
	 * VC40SETADDR
	 *
	 * Sets the DSP transfer address for read() and write()
	 */
	case VC40SETADDR: {
		u_long dsp_addr = *(u_long *)arg;

		vc40_units[unitn]->io_addr = dsp_addr;
		break;
	} /* VC40SETADDR */

	case VC40DISKEY: {
		hcmdn->cmd = DisableKeyInt;
		rcode = vc40_cmd_interrupt(unitn);
		break;
	}

	case VC40ENKEY: {
		hcmdn->cmd = EnableKeyInt;
		rcode = vc40_cmd_interrupt(unitn);
		break;
	}

	/*-----------------------------------------------------------------
	 * unknown ioctl()
	 */
	default: {
		ERROR_MSG(logit, "vc40dsp: Unknown ioctl(): %d (0x%x)\n",
			cmd, cmd);
		rcode = EIO;
		break;
	} /* default */

	} /* switch(cmd) */

	return (rcode);
}
