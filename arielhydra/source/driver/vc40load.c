/* V-C40 Hydra Device Driver
 * vc40load.c
 * Wrapper to make driver loadable
 */

#include <sys/types.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sundev/mbvar.h>
#include <sun/autoconf.h>
#include <sun/vddrv.h>
#include "vc40drv.h"

extern struct mb_driver	vc40driver;	/* defined in driver */

/*
 * driver character device entry points
 */
struct cdevsw vc40cdev = {
	vc40_open, vc40_close, vc40_read, vc40_write, vc40_ioctl, nodev,
	vc40_select, vc40_mmap, 0
};

/*
 * device structure
 */
#ifdef NOCC
struct mb_device vc40cdevice[MAX_NVC40] = {
	&vc40driver, 	/* mb_driver *md_driver */
	0,		/* unit number on the system */
	-1,		/* mass ctlr number; -1 if none */
	0,		/* slave on controller */
	(caddr_t ) 0x0,	/* address of device in i/o space */
	0,		/* interrupt priority */
	0,		/* if init 1 set to number for iostat */
	0,		/* parameter from system specification */
	0,		/* if vectored interrupts used */
	0,		/* encode bits for addr device space */
};
#else
struct mb_device vc40cdevice[MAX_NVC40];
#endif

struct vdldrv vd = {
	VDMAGIC_DRV,	/* type of module, this one is a driver */
	VC40_DRV_NAME,	/* name of the module */
	NULL,		/* no controller */
	&vc40driver,	/* address of the mb_driver structure */
	vc40cdevice,	/* address of the mb_device structure array */
	0,		/* number of controllers */
	1,		/* number of devices */
	NULL,		/* no bdevsw entry */
	&vc40cdev,	/* address of cdevsw entry */
	0,		/* block device number */
	0		/* character device number */
};

/***************************************************************
 * driver entry point routine.  Must use "-entry _vc40_init" to `modload'
 */
vc40_init(function_code, vdp, vdi, vds)
unsigned int	function_code;
struct vddrv	*vdp;
addr_t	vdi;
struct vdstat	*vds;
{
	switch(function_code) {
		case VDLOAD:
			vdp->vdd_vdtab = (struct vdlinkage *) &vd;
			return 0;

		case VDUNLOAD:
			return (unload(vdp, vdi));

		case VDSTAT:
			return 0;

		default:
			return EIO;
	}
}

static unload(vdp, vdi)
struct vddrv	*vdp;
struct vdioctl_unload	*vdi;
{
	int	i;
	int	status = 0;
	u_long	kmx, mcr_val;
	u_long	*mcr_addr;

	/*
	 * prevent any V-C40's from being opened
	 */
	for (i=0; i < num_dsp; i++) {
		if (vc40_units[i]) {
			vc40_units[i]->open_inhibit = 1;
		}
	}

	/*
	 * check for any open V-C40's
	 */
	for (i=0; i < num_dsp; i++) {
		if (vc40_units[i] && vc40_units[i]->unit_open) {
			status = EBUSY;
		}
	}

	/*
	 * if devices are still open, can't unload
	 * then go back to business as usual
	 */
	if (status) {
		for (i=0; i < num_dsp; i++) {
			if (vc40_units[i]) {
				vc40_units[i]->open_inhibit = 0;
			}
		}
		return (status);
	}

	/*
	 * disable all interrupts
	 */

	/*
	 * remove from interrupt chain
	 */

	/*
	 * deallocate structures
	 */
	for (i=1; i<num_dsp; i+=4) {	/* do only for DSP1 of each V-C40 */
		if (vc40_units[i]) {
			/*
			 * turn off LED3
			 */
			mcr_addr = vc40_units[i]->mcr;
			if (peekl(mcr_addr, &mcr_val) != -1) {
				pokel(mcr_addr, mcr_val & ~MCR_L3);
			}
			/*
			 * mapout the JTAG interface and MCR
			 */
			vc40_mapout(vc40_units[i]->jtag, JTAG_NPAGES);

			/*
			 * mapout the DRAM I/O area
			 */
			vc40_mapout(vc40_units[i]->dram_vaddr,
				vc40_units[i]->dram_map_npgs);
		}
	}

	for (i=0; i<num_dsp; i++) {
		if (vc40_units[i]) kmem_free(vc40_units[i]);
	}
		

	return 0;
}
