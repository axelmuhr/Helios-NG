/* $Id: vc40all.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

/* vc40all.h
 *
 * This header file contains definitions used by user programs, Unix device
 * drivers, and all of the mapped libraries and utility libraries.
 */

#ifndef VC40ALL_H
#define VC40ALL_H 1

#if defined(MSDOS) && !defined(__MSDOS__) /* MicroSoft always has to be different */
#   define __MSDOS__
#endif

#include "c40types.h" /* for typedefs of u_{char,short,long} */

/***************************************************************************
 * common to all flavors of Hydra
 */
/*
 * note: can't use HYDRA as ID for Hydra, it may be defined by the device
 * driver when configured into the kernel.
 */
enum hydra_board_type {
    HYDRAI = 0x01, HYDRAII = 0x02, HYDRAIVSB = 0x03, SBUS_AXDS = 0x10
};

enum hydra_misc {
    MAXNUMC40 = 16      /* maximum number of DSPs per board */
};

/***************************************************************************
 * Hydra-II  Information
 */

/*
 * various constants
 */
/* @!#$%^?*& DOS can't handle numbers > 0xffff as enums! */
#define H2MAXSHMEM	0x10000	/* maximum amount of shared memory (BYTES!) */
#define H2MAXDSP	4	/* maximum number of dsp's */	
#define H2_MMAP_O_SHMEM 0	/* hydra-ii offset to shared memory */
#define H2_MMAP_O_SEM	0x10000	/* hydra-ii offset to semaphores */
#define H2_MMAP_O_INTC	0x10400	/* hydra-ii offset to interrupt controller */
#define H2_MMAP_O_MCR	0x10800	/* hydra-ii offset to main control register */
#define H2_MMAP_O_JTAG	0x10c00	/* hydra-ii offset to jtag */

enum h2k {
    HYHOMONIIOF1 = 4,       /* tell hyhomon to use IIOF1 */
    HYHOMONIIOF2 = 5,       /* tell hyhomon to use IIOF2 */
    HYHOMONIIOF3 = 6,       /* tell hyhomon to use IIOF3 */
    HYHOMON_DSPNO = 0x48,   /* index of word in boot record for DSP no. */
    HYHOMON_IIOF = 0x49,    /* index of word in boot record for interrupt no. */
    MAXFLASHDATA = 0x10     /* max # of flash words to read/write at once */
};

struct h2mcr {
    struct {        /* to force 16-bit accesses from DOS */
        u_short bcr;
        u_short null;
    } bcr[4];
    struct {        /* to force 16-bit accesses from DOS */
        u_short icr;
        u_short null;
    } icr[4];
};

/*
 * structure defines control registers on Hydra-II
 */
struct h2ctrl {
    u_long sem[8];                  /* semaphores */
    u_long pad1[0xf8];              /* semaphores alias here */
    short intcreg[4];               /* interrupt control registers */
    short intvreg[4];               /* interrupt vector registers */
    char null[0x3f0];               /* dead space */
    struct h2mcr h2mcr;             /* Hydra-II control registers */
    char pad2[0x400 - sizeof(struct h2mcr)]; /* control registers alias here */
    u_long jtag[64];                /* JTAG debugger interface */
};
/*
 * bits within the bcr and icr
 */
enum h2bcr {
    H2RLOC0 = 0x01,     /* reset location bits */
    H2RLOC1 = 0x02,
    H2IIOF1 = 0x04,     /* IIOF pins */
    H2IIOF2 = 0x08,
    H2IIOF3 = 0x10,
    H2RESET = 0x20,     /* DSP reset (active low) */
    H2IACK  = 0x40      /* DSP IACK (read only) */
};

enum h2icr {
    H2NMIMUXA   = 0x01,
    H2NMIMUXB   = 0x02,
    H2NMIEN     = 0x04,     /* active low */
    H2IIOF2MUXA = 0x08,
    H2IIOF2MUXB = 0x10,
    H2IIOF2EN   = 0x20,     /* active low */
    H2AUTO      = 0x40
};

/*
 * structure used to read/write the flash memory
 */
struct h2flash {
    u_long foffs;   /* flash offset */
    u_long wcnt;    /* number of words to transfer */
    u_long data[MAXFLASHDATA];
};
#endif /* #ifndef VC40ALL_H */
