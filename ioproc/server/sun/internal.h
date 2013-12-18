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


/* $Id: internal.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

/* internal.h
 *
 * stuff needed by the internals of the driver and the mapping libraries,
 * but not by the user.
 *
 * User programs should not include this file for future compatibility.
 */

#ifndef INTERNAL_H
#define INTERNAL_H 1


#include "c40types.h" /* for u_long, etc. */

/***************************************************************************
 * Hardware revision dependencies
 */
#define HYDRA_NAME      "VC40"      /* Hydra(AD Bus) name in EEPROM */
#define VSB_NAME        "VSB1"      /* Hydra(VSB) name in EEPROM */
#define RESET_REVISION  'C'         /* minimum h/w rev for reset of DSP 1 */
#define SERIALNO_REVISION   2000    /* mon. rev 2.0 and up for serial number */
#define INTERRUPT_REVISION  1400    /* mon. rev 1.4 changed intrpt mapping */
#define SHMEMSIZE_REVISION   1410   /* mon. rev 1.41 for DRAM size in KB */
#define WIPEC40_REVISION     1500   /* mon. rev 1.5 for reset revision */

/***************************************************************************
 * HydraMon dependent stuff
 */
#define	CTRL_ICR	icr0	/* VIC ICR to use for control words */
#define	DATA_ICR	icr1	/* VIC ICR to use for data transfer */
#define	DSP1_SET_INTR	set_icms0	/* Where to assert DSP1's interrupt */
#define	DSP1_CLR_INTR	clr_icms0	/* Where to clear DSP1's interrupt */

enum hyhomonio {
    H1IOBUF_SIZE = 0x400,	/* Hydra DRAM I/O buffer size (in words) */
    H1IOCTRL_SIZE = 0x10,	/* Size of I/O control buffer (in words) */

    H2IOBUF_SIZE = 0x100,	/* DRAM I/O buffer size (in bytes) */
    H2IOCTRL_SIZE = 0x10,	/* Size of I/O control buffer (in bytes) */

    WIPEC40 = 0xdb,  /* V-C40 Hydra magic number to reset DSP 1 and VIC & VAC */
    RESETC40 = 0xdc  /* V-C40 Hydra magic number to just reset DSP 1 */
};

/*
 * HydraMon command structures
 */
struct hydra_copy {	/* CopyStuff command */
    u_long	src;	/* DSP source address (in DSP space) */
    u_long	dst;	/* DSP destination address (in DSP space */
    u_long	len;	/* number of words to copy */
    u_long	nulls[12];	/* 12 unused words */
};

struct hydra_boot {	/* reset and boot DSP command */
    u_long	dspnum;	/* DSP to be reset */
    u_long	nulls[14];	/* 14 unused words */
};

struct hydra_run {	/* branch to address command */
    u_long	addr;	/* address to branch to (in DSP space) */
    u_long	nulls[14];	/* 14 unused words */
};

struct generic_cmd {
    u_long	param1, param2, param3, param4, param5, param6, param7, param8;
    u_long	param9, param10, param11, param12, param13, param14, param15;
};

struct hydra_cmd {	/* generic Hydra command structure */
    u_long	cmd;	/* command token */
    union {		/* command parameters */
        struct hydra_copy	copy_cmd;
        struct hydra_boot	boot_cmd;
        struct hydra_run	run_cmd;
        struct generic_cmd	cmd;
    } param;
};

/***************************************************************************
 * Hydra architecture stuff
 */

/*
 * bits in the MCR
 */
#define	MCR_L3		0x80000000      /* up to and incl. REV C */
#define MCR_D1R         0x80000000      /* REV D and beyond */
#define	MCR_D4R		0x40000000
#define	MCR_D3R		0x20000000
#define	MCR_D2R		0x10000000

#define	MCR_WS3		0x08000000
#define	MCR_WS2		0x04000000
#define	MCR_WS1		0x02000000
#define	MCR_WS0		0x01000000

#define	MCR_EEC		0x00800000
#define	MCR_EED		0x00400000
#define	MCR_L2		0x00200000
#define	MCR_D4N		0x00100000

#define	MCR_D3N		0x00080000
#define	MCR_D2N		0x00040000
#define	MCR_4I3		0x00020000
#define	MCR_4I2		0x00010000

#define	MCR_4I1		0x00008000
#define	MCR_3I3		0x00004000
#define	MCR_3I2		0x00002000
#define	MCR_3I1		0x00001000

#define	MCR_2I3		0x00000800
#define	MCR_2I2		0x00000400
#define	MCR_2I1		0x00000200
#define	MCR_4R0		0x00000100

#define	MCR_4R1		0x00000080
#define	MCR_4EN		0x00000040
#define	MCR_3R0		0x00000020
#define	MCR_3R1		0x00000010

#define	MCR_3EN		0x00000008
#define	MCR_2R0		0x00000004
#define	MCR_2R1		0x00000002
#define	MCR_2EN		0x00000001

/***************************************************************************
 * Hydra-II stuff
 */
/* @!#$%^?*& DOS can't handle numbers > 0xffff as enums! */
#define HYDRAII_JTAG 0x10c00    /* JTAG offset from beginning of HydraII
                                   address space */
#define HYDRAII_CTRL 0x10000    /* offset from beginning of HydraII address
                                   space for control registers */
#define H2SEMADDR 0x80008000    /* DSP address of semaphores */
enum {
    H2NUMSEMS = 8,              /* 8 semaphores in dual port RAM */
};

#endif /* #ifndef INTERNAL_H */
