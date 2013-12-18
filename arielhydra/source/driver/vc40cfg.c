/* vc40drv.c
 *
 * Auto config functions for V-C40 driver
 */

#include "vc40drv.h"

static u_long *phys2virt();

struct mb_device *vc40dinfo[MAX_NVC40];	/* array of pointers to elements of
					 * the vc40cdevice[] structure array
					 * defined in vc40load.c */
struct mb_driver hydradriver = {
	vc40_probe, 	/* probe() function */
	0,		/* slave() function - for devices on controllers */
	vc40_attach, 	/* attach() function */
	0,		/* go() function - only in block drivers */
	0,		/* done() function - only in block drivers */
	0,		/* no polling interrupt routine, vectored only */
	sizeof(struct vc40_vic),	/* size required to map the CSR */ 
	VC40_DEV_NAME, 	/* device name */
	vc40dinfo, 	/* device info structure */
	0, 		/* controller name */
	0, 		/* controller infor */
	0, 		/* want exclusive use of Main Bus */
	0		/* interrupt routine linked list */
};

struct vc40_units_struct *vc40_units[MAX_NVC40];
int	logit;
int	num_dsp = 0;	/* number of V-C40's in the system, counted by the
			   probe() routine */

/**********************************************************************
 * vc40_probe() determines if there is a V-C40 present at the specified 
 * address
 */
int vc40_probe(reg, unit)
caddr_t	reg;
int	unit;
{
	char	prop_str[5];
	int	unit1 = UNIT1(unit);	/* unit no. for DSP 1 of this Hydra */
	int	dsp = DSP(unit);	/* DSP no. for this DSP */
	u_long	prop_val, dcard, jtag_paddr;
	struct vc40_units_struct *uptr;

	struct vc40_vic	*vic;

	vic = (struct vc40_vic *) reg;


	/*
 	 * this function may be called for each DSP of a Hydra.  Only
	 * check for Hydra existance on DSP 1.  DSP 2 will exist if DSP 1
	 * exists.  DSP 3 and 4 will exist if DSP 1 exists and a daughter
	 * card is present.  In any event, if this DSP is not DSP 1, just
	 * check to see if the unit structure has been allocted.  When called
	 * for DSP 1, this function will allocate the unit structures for
	 * all DSPs that exist
	 */
	if (vc40_units[unit]) {
		return (sizeof(struct vc40_vic));
	}
	if (dsp != DSP1) {
		ERROR_MSG(logit, "DSP %d does not exist on this Hydra!\n", dsp);
		return (0);
	}

	/*-------------------------------------------------------------
	 * see if the V-C40 is present:
	 * 	1) try reading CTRL_ICR
	 *	2) try reading DATA_ICR
	 *	3) try reading DSP1_INTR
	 *	4) if all OK, try to get the V-C40 ID property
	 */
	if (peekc(&(vic->CTRL_ICR)) == -1) {
		ERROR_MSG(logit, "failed to read ICR0\n");
		return (0);
	}
	if (peekc(&(vic->DATA_ICR)) == -1) {
		ERROR_MSG(logit, "failed to read ICR1\n");
		return (0);
	}
	if (peekc(&(vic->DSP1_SET_INTR)) == -1) {
		ERROR_MSG(logit, "failed to read SET_INTR\n");
		return (0);
	}
	if (get_property(vic, DRAMBase, &prop_val) != 0) {
		ERROR_MSG(logit, "failed to read DRAMBase address\n");
		return (0);
	}
	prop_str[4] = '\0';
	if (get_property(vic, BoardName, prop_str) != 0) {
		ERROR_MSG(logit, "failed to read BoardName property\n");
		return (0);
	}
DEBUG_MSG5(logit, "Board Name: %s\n", prop_str);
	if (get_property(vic, Firmware, prop_str) != 0) {
		ERROR_MSG(logit, "failed to read Firmware property\n");
		return (0);
	}
DEBUG_MSG5(logit, "firmware revision: %s\n", prop_str);
	if (get_property(vic, Hardware, prop_str) != 0) {
		ERROR_MSG(logit, "failed to read Hardware property\n");
		return (0);
	}
DEBUG_MSG5(logit, "Hardware revision: %s\n", prop_str);

	/*-------------------------------------------------------------
	 * this unit is OK, so allocate space for the unit structures
	 * based on the number of DSPs on the V-C40 (currently, this will
	 * 2 if no daughter card is attached, or 4 if there is a daughter
	 * card).
	 */
	if (get_property(vic, Daughter, &dcard) != 0) {
		ERROR_MSG(logit, "failed to read Daughter property\n");
		return (0);
	}
	DEBUG_MSG9(logit, "vc40_probe: Daughter Property is %d\n", dcard);

	/*
	 * DSP 1
	 */
	uptr = (struct vc40_units_struct *) kmem_alloc(UNIT_SIZE);
	if (uptr == NULL) {
		ERROR_MSG(logit, "failed to allocate memory for unit %d\n",
			unit);
		return (0);
	}
	bzero(uptr, UNIT_SIZE);
	vc40_units[unit] = uptr;	/* DSP 1 */
	vc40_units[unit]->vic = vic;	/* only DSP 1 accesses IPCR */
	vc40_units[unit]->dcard = dcard;

	/*
	 * DSP 2
	 */
	uptr = (struct vc40_units_struct *) kmem_alloc(UNIT_SIZE);
	if (uptr == NULL) {
		ERROR_MSG(logit, "failed to allocate memory for unit %d\n",
			unit);
		return (0);
	}
	bzero(uptr, UNIT_SIZE);
	vc40_units[unit+1] = uptr;	/* DSP 2 */
	vc40_units[unit+1]->vic = 0;	/* all IPCR access thru DSP 1 */

	/*
	 * DSPs 3 & 4
	 */
	if (dcard) {
		/*
		 * DSP 3
		 */
		uptr = (struct vc40_units_struct *) kmem_alloc(UNIT_SIZE);
		if (uptr == NULL) {
			ERROR_MSG(logit,
				"failed to allocate memory for unit %d\n",
				unit);
			return (0);
		}
		bzero(uptr, UNIT_SIZE);
		vc40_units[unit+2] = uptr;	/* DSP 3 */
		vc40_units[unit+2]->vic = 0;	/* all IPCR access thru DSP 1 */

		/*
		 * DSP 4
		 */
		uptr = (struct vc40_units_struct *) kmem_alloc(UNIT_SIZE);
		if (uptr == NULL) {
			ERROR_MSG(logit,
				"failed to allocate memory for unit %d\n",
				unit);
			return (0);
		}
		bzero(uptr, UNIT_SIZE);
		vc40_units[unit+3] = uptr;	/* DSP 3 */
		vc40_units[unit+3]->vic = 0;	/* all IPCR access thru DSP 1 */
	}

	/*
	 * so far so good
	 */
	num_dsp += 4;	/* regardless of presence of daughter card */
	return (sizeof(struct vc40_vic));
}

/**********************************************************************
 * vc40_attach() performs DSP initialization and structure allocation
 */
int vc40_attach(vc40dev)
struct mb_device *vc40dev;
{
	char	*all_dram;
	int	unit = vc40dev->md_unit;
	int	dspno = DSP(unit);
	int	i, rcode, intvec, intpri;
	u_long	jtag_paddr, dram_paddr, dram_size, mcr_val;
	u_long	dram_dsp_top, dram_dsp_addr, paddr;
	u_long	numdsp, map_size, dbase, npgs, moffs;
	u_long	*dvaddr, *dram_vme_top, *vaddr, *mcrp, *jtagp;
	struct vc40_vic *vic = vc40_units[unit]->vic;
	struct vc40_units_struct *vdsp;
	struct hydra_cmd *hcmd;

	/*
	 * save interrupt priority and vector for this DSP
	 */
	intpri = vc40dev->md_intpri;
	intvec = vc40dev->md_intr->v_vec;
	DEBUG_MSG5(logit, "V-C40 DSP %d at priority %d on vector 0x%x\n",
		dspno+1, vc40dev->md_intpri, vc40dev->md_intr->v_vec);

	/*
	 * this function may get called for each DSP of a hydra.  Only
	 * respond for DSP 1
	 */
	if (dspno != DSP1) {
		vc40_units[unit]->intpri = intpri;
		vc40_units[unit]->intvec = intvec;
		DEBUG_MSG9(logit, "DSPno %d already attached via DSP 1\n", 
			dspno+1);
		return;
	}

	/*
	 * if no interrupt priority or vector specified, use those of the
	 * previous Hydra (if this is not the first Hydra, that is)
	 */
	if (unit > 4) {
		intpri = vc40_units[unit-4]->intpri;
		intvec = vc40_units[unit-4]->intvec;
	}
	vc40_units[unit]->intpri = intpri;
	vc40_units[unit]->intvec = intvec;

	/*
	 * get register addresses from DSP1 of the V-C40
	 * First, get address of the JTAG Interface
	 */
	if (get_property(vic, HostJTAGBase, &jtag_paddr) != 0) {
		ERROR_MSG(logit, 
"vc40dsp: failed to get JTAGBase Property from unit %d\n", unit);
		vc40_units[unit]->open_inhibit = 1;
		return;
	}
	DEBUG_MSG9(logit, "JTAGBase: 0x%x\n", jtag_paddr);
	jtagp = vc40_mapin(jtag_paddr, JTAG_NPAGES);

	/*
	 * Main Control Register (MCR) is 8 words away from Jtag
	 */
	mcrp = jtagp + 8;

	/*
	 * Get the DRAM size and base address
	 */
	if (get_property(vic, DRAMSize, &dram_size) != 0) {
		ERROR_MSG(logit, 
"vc40dsp: failed to get DRAMSize Property from unit %d\n", unit);
		vc40_units[unit]->open_inhibit = 1;
		return;
	}
	if (get_property(vic, DRAMBase, &dram_dsp_addr) != 0) {
		ERROR_MSG(logit, 
"vc40dsp: failed to get DRAMBase Property from unit %d\n", unit);
		vc40_units[unit]->open_inhibit = 1;
		return;
	}
	dram_size *= (4*1024*1024);	/* convert size to bytes */
	dram_paddr = dram_dsp_addr << 2; /* convert DSP space to VME */

	DEBUG_MSG9(logit, "vc40dsp: DRAM base: 0x%x; DRAM size: 0x%x MByte\n",
		dram_paddr, dram_size/1024/1024);

	/*
	 * command control and data transfer areas are arranged at the top
	 * of Hydra's DRAM as follows.  The top of DRAM contains one command
	 * control buffer for each DSP.  If only 2 DSPs are present, then
	 * only two control buffers are allocated.  Each control buffer is
	 * IOCTRL_SIZE bytes (/4 words).  DSP 1's control area is at the top 
	 * of DRAM, DSP 2's is below that, then DSP 3 and DSP 4 (if present).
	 * Immediately below the control buffers are the data buffers, one per
	 * DSP, each of size IOBUF_SIZE bytes.  From high to low memory, the
	 * buffers are for DSP 1, 2, 3 and 4 (if 3 and 4 are present).
	 *
 	 * For a 2-headed Hydra, this looks as follows:
	 *
	 *	 DRAM TOP->+-----------------+
	 *	           |  DSP 1 Command  |  IOCTRL_SIZE bytes
         *                 +-----------------+
	 *		   |  DSP 2 Command  |  IOCTRL_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 1 Data     |  IOBUF_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 2 Data     |  IOBUF_SIZE bytes
	 *	CTRL BASE->+-----------------+
	 *		   |                 |
	 *		   |    User DRAM    |
	 *		   |                 |
	 *	DRAM BASE->+-----------------+
	 *
 	 * For a 4-headed Hydra, this looks as follows:
	 *
	 *	 DRAM TOP->+-----------------+
	 *	           |  DSP 1 Command  |  IOCTRL_SIZE bytes
         *                 +-----------------+
	 *		   |  DSP 2 Command  |  IOCTRL_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 3 Command  |  IOCTRL_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 4 Command  |  IOCTRL_SIZE bytes
	 *	           +-----------------+
	 *		   |  DSP 1 Data     |  IOBUF_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 2 Data     |  IOBUF_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 3 Data     |  IOBUF_SIZE bytes
	 *		   +-----------------+
	 *		   |  DSP 4 Data     |  IOBUF_SIZE bytes
	 *	CTRL BASE->+-----------------+
	 *		   |                 |
	 *		   |    User DRAM    |
	 *		   |                 |
	 *	DRAM BASE->+-----------------+
	 *
	 */

	/*
	 * map in the DRAM needed for the I/O buffers for read() and write()
	 */
	if (vc40_units[unit]->dcard) {
		numdsp = 4;
	}
	else {
		numdsp = 2;
	}

#ifdef MAP_ALL_DRAM
	/*
	 * map in all of DRAM
	 */
	map_size = dram_size;
	dbase = dram_paddr;
#else
	/*
	 * map in just the data transfer and control area of the DRAM
	 */
	map_size = numdsp * (IOCTRL_SIZE + IOBUF_SIZE);	/* total size */
	dbase = dram_paddr + dram_size - map_size; /* bottom of data area */
#endif
	npgs = map_size / SYSTEM_PAGE_SIZE;	/* number of physical pages */
	if (npgs * SYSTEM_PAGE_SIZE != map_size) npgs++;	/* round up */
	dvaddr = vc40_mapin(dbase, npgs);	/* map in the DRAM */

	/*
	 * top of DRAM in virtual VME space (long *)
	 */
	dram_vme_top = dvaddr + map_size/4; 

	/*
	 * top of DRAM in DSP space
	 */
	dram_dsp_top = dram_dsp_addr + dram_size/4;

	/*
	 * need to save base virtual address and number of pages mapped
 	 * for unloading the driver later - only DSP 1 structure needs this.
	 */
	vc40_units[unit]->dram_vaddr = (char *) dvaddr;
	vc40_units[unit]->dram_map_npgs = npgs;

	/*
	 * let all DSPs know this stuff
	 */
	for (i=0; i<numdsp; i++) {
	    vdsp = vc40_units[unit+i];
	    if (vdsp) {
	        /*
	         * command control buffer in DRAM (virtual address)
	         */
	        vdsp->hydra_cmd = (struct hydra_cmd *) 
			(dram_vme_top - (i+1)*IOCTRL_SIZE/4);

		/*
		 * data xfr buffer in DRAM (VME virtual address)
		 */
		moffs = numdsp*IOCTRL_SIZE/4 + (i+1)*IOBUF_SIZE/4;
		vdsp->xfr_data = dram_vme_top - moffs;

		/*
		 * data xfr buffer in DRAM (DSP space)
		 */
		vdsp->dsp_dram_io = dram_dsp_top - moffs;

		/*
		 * common info to all DSPs
		 */
		vdsp->dram_size = dram_size;	/* DRAM size */
		vdsp->dram_paddr = dram_paddr;	/* DRAM physical VME addr */
		vdsp->mcr = mcrp;		/* pointer to MCR */
		vdsp->jtag = jtagp;		/* pointer to JTAG */
		vdsp->attach_code = VC40_DETACHED;	/* DSP not attached */

		/*
		 * by default all DSPs use same interrupt priority and vector.
		 * This can be changed by listing the correct priority and
		 * vector for each DSP in the config file.
		 */
		vdsp->intpri = vc40_units[unit]->intpri;
		vdsp->intvec = vc40_units[unit]->intvec;
	    }
	}

	/*
	 * make sure no NMI's asserted
	 */
	*mcrp |= (MCR_D2N | MCR_D3N | MCR_D4N);

	/*
	 * turn LED3 on, LED2 off
	 */
#ifdef DEBUG
	if (peekl(mcrp, &mcr_val) == -1) {
		ERROR_MSG(logit, 
"vc40dsp: vc40_attach(): error reading MCR on unit %d\n", unit);
		vc40_units[unit]->open_inhibit = 1;
	}
	mcr_val |= MCR_L3;
	mcr_val &= ~MCR_L2;
	if (pokel(mcrp, mcr_val) == -1) {
		ERROR_MSG(logit, 
"vc40dsp: vc40_attach(): error writing MCR on unit %d\n", unit);
		vc40_units[unit]->open_inhibit = 1;
	}
#endif

	/*
	 * print attached message
	 */
	if (vc40_units[unit]->dcard) {
		DEBUG_MSG(logit, "vc40dsp: Attached 4-headed Hydra\n");
	}
	else {
		DEBUG_MSG(logit, "vc40dsp: Attached 2-headed Hydra\n");
	}
}

