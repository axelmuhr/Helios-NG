/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1993 Ariel Corp.               */
/****************************************************************/


/* $Id: c40util.h,v 1.2 1994/06/29 13:46:19 tony Exp $ */
/* c40util.h
 *
 * stuff needed by ALL versions of the utility library.
 */

#ifndef C40UTIL_H
#define C40UTIL_H 1

#include "portable.h"   /* for PROTOTYPE macro */
#include "c40types.h"   /* for u_long */
#include "coff.h"       /* for struct symtab */

/*
 * structures needed by all users
 */
struct vc40info {
    u_long intpri;      /* unit's interrupt priority */
    u_long intvec;      /* unit's interrupt vector */
    u_long dram_base;   /* board's DRAM base physical address (VME) */
    u_long dram_size;   /* board's DRAM size (in bytes!) */
    u_long numdsp;      /* number of DSPs present (2 or 4) */
    u_long hardrev;     /* hardware revision level 'A', 'B', etc. */
    u_long mserno;      /* motherboard serial number */
    char firmrevs[8];   /* firmware revision string */
    char drvrevs[24];   /* driver revision string */
    u_long board_type;  /* board type: HYDRAI, HYDRAII, etc. */
};

/*
 * VIC IPCR registers from VME side
 */
struct vic_ipcr {
    u_char null0;	/* illegal access */
    u_char icr0;
    u_char null1;	/* illegal access */
    u_char icr1;
    u_char null2;	/* illegal access */
    u_char icr2;
    u_char null3;	/* illegal access */
    u_char icr3;
    u_char null4;	/* illegal access */
    u_char icr4;
    u_char null5;	/* illegal access */
    u_char icr5;
    u_char null6;	/* illegal access */
    u_char icr6;
    u_char null7;	/* illegal access */
    u_char icr7;
    u_char clr_icgs0;	/* 0x10 */
    u_char set_icgs0;
    u_char clr_icgs1;
    u_char set_icgs1;
    u_char clr_icgs2;
    u_char set_icgs2;
    u_char clr_icgs3;
    u_char set_icgs3;	/* 0x17 */
    u_char null8[8];
    u_char clr_icms0;	/* 0x20 */
    u_char set_icms0;
    u_char clr_icms1;
    u_char set_icms1;
    u_char clr_icms2;
    u_char set_icms2;
    u_char clr_icms3;
    u_char set_icms3;
};

/*
 * structure for reading/writing IPCRs
 */
struct ipcr {
    int ipcr_num;
    u_char ipcr_data;
};

/*
 * JTAG/MCR structure
 */
struct vc40jmcr {
    u_long null1[8];  /* MCR is 8 words offset from the base address */
    u_long mcr;
    u_long null2[23];   /* JTAG is 32 words from the _base_ address */
    u_long jtag[32];    /* only lower 16 bits are significant of each reg. */
};


/*
 * if PROTOTYPE macro is defined, can have a single prototype expand 
 * correctly for different compilers 
 */

PROTOTYPE (int far getsopt, 
	   (int optidx, int argc, char far * far * argv, char far *options,
	    int far *opton, char far * far * optarg))

PROTOTYPE (int far c40_open, 
	   (char far *devd, int mode))

PROTOTYPE (int far c40_close, 
	   (int c40id))

PROTOTYPE (int far c40_read_long, 
	   (int c40id, u_long dsp_addr, void far *buf, u_long wcnt))

PROTOTYPE (int far c40_write_long, 
	   (int c40id, u_long dsp_addr, void far *buf, u_long wcnt))

PROTOTYPE (int far c40_get_long, 
	   (int c40id, u_long dsp_addr, u_long far *lval))

PROTOTYPE (int far c40_put_long, 
	   (int c40id, u_long dsp_addr, u_long lval))

PROTOTYPE (int far c40_get_float, 
	   (int c40id, u_long dsp_addr, float far *fval))

PROTOTYPE (int far c40_put_float, 
	   (int c40id, u_long dsp_addr, float fval))

PROTOTYPE (int far c40_get_dsp_float, 
	   (int c40id, u_long dsp_addr, float far *fval))

PROTOTYPE (int far c40_put_dsp_float, 
	   (int c40id, u_long dsp_addr, float fval))

PROTOTYPE (u_long far ieee2dsp,
	   (float ieee))

PROTOTYPE (float far dsp2ieee,
	   (long dspnum))

PROTOTYPE (struct vc40jmcr far *c40_map_jmcr,
	   (int c40id))

PROTOTYPE (u_long far *c40_map_jtag,
	   (int c40id))

PROTOTYPE (struct vic_ipcr far *c40_map_ipcr,
	   (int c40id))

PROTOTYPE (int far c40_run, 
	   (int c40id, u_long dspaddr))

PROTOTYPE (int far c40_reset, 
	   (int c40id))

PROTOTYPE (int far c40_wipe, 
	   (int c40id))

PROTOTYPE (int far c40_halt, 
	   (int c40id))

PROTOTYPE (int far c40_getinfo, 
	   (int c40id, struct vc40info far *vc40info))

PROTOTYPE (int far c40_trap, 
	   (int c40id, u_long trapnum))

PROTOTYPE (int far c40_dskey, 
	   (int c40id))

PROTOTYPE (int far c40_enkey, 
	   (int c40id))

PROTOTYPE (int far c40_attach, 
	   (int c40id, u_long acode, u_long far *ocode))

PROTOTYPE (int far c40_detach, 
	   (int c40id, u_long acode))

PROTOTYPE (int far c40_getprop, 
	   (int c40id, u_long propid, u_long far *propval))

PROTOTYPE (int far c40_write_mcr, 
	   (int c40id, u_long mcrv))

PROTOTYPE (int far c40_read_mcr, 
	   (int c40id, u_long far *mcrv))

PROTOTYPE (int far c40_load, 
	   (int c40id, char far *pname, u_long far *eaddr, int numsyms,
                 char far * far * symnames, struct symtab far *symtab))

PROTOTYPE (char far *vc40_driver,
	   (void))

PROTOTYPE (int far c40_read_shmem, 
	   (int c40id, u_long woffs, u_long far *bufp, u_long nwords))

PROTOTYPE (int far c40_write_shmem, 
	   (int c40id, u_long woffs, u_long far *bufp, u_long nwords))

PROTOTYPE (int far c40_read_ipcr, 
	   (int c40id, int ipcrn, u_char far *ipcrvp))

PROTOTYPE (int far c40_write_ipcr, 
	   (int c40id, int ipcrn, u_char ipcrvp))

PROTOTYPE (int far c40_dsint, 
	   (int c40id))

/* a number of routines have different arglists under unix & dos */

#if defined(__MSDOS__) || defined(__OS2__)

PROTOTYPE (int far c40_enint, 
	   (int c40id, u_long ipri, u_long ivec))

PROTOTYPE (char far *c40_map_shmem,
	   (int c40id, u_long offs))

PROTOTYPE (short far c40_wait,
	   (u_short mask, u_long far *intvec, u_long ticks))

#else

PROTOTYPE (int far c40_enint, 
	   (int c40id, int signum))

PROTOTYPE (char far *c40_map_shmem,
	   (int c40id, u_long offs, u_long len))

PROTOTYPE (int c40_wait,
	   (int c40id, int timeout))

#endif /* __MSDOS__ */

PROTOTYPE (int far c40_read_jtag, 
	   (int c40id, int jtagn, u_long far *jtagvp))

PROTOTYPE (int far c40_write_jtag, 
	   (int c40id, int jtagn, u_long jtagvp))

PROTOTYPE (int far c40_bootload, 
	   (int c40id, u_long far *code, u_long len))

PROTOTYPE (int far c40_flash_erase, 
	   (int c40id, int sectornum))

PROTOTYPE (int far c40_read_flash, 
	   (int c40id, u_long flash_addr, Void_p far bbuf, u_long wcnt,
	    int flash_width))

PROTOTYPE (int far c40_write_flash, 
	   (int c40id,u_long flash_addr, Void_p far bbuf, u_long wcnt,
	    int flash_width))

PROTOTYPE (struct h2ctrl far *c40_map_h2ctrl,
	   (int c40id))

PROTOTYPE (int far findfile, 
	   (char far *filename, char far *envvar, char far *path))

PROTOTYPE (int far c40_getsem,
           (int c40id, int semno))

PROTOTYPE (int far c40_relsem,
           (int c40id, int semno))

PROTOTYPE (int far c40_read_bcr,
           (int c40id, u_short *bcrval))

PROTOTYPE (int far c40_write_bcr,
           (int c40id, u_short bcrval))

PROTOTYPE (int far c40_read_icr,
           (int c40id, u_short *icrval))

PROTOTYPE (int far c40_write_icr,
           (int c40id, u_short icrval))

/*
 * function c40_map_dram() was changed to c40_map_shmem()
 * (modified by alan -- unix version 3 args; dos version 2 args. ugh!)
 */

#define c40_map_dram c40_map_shmem

#endif /* ifdef C40UTIL_H */
