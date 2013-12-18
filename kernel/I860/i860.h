#ifndef _i860_h
#define _i860_h

/* $Header: /00/raw/i860/test/RCS/i860.h,v 1.1 90/03/12 11:39:01 chris Exp $ */

/*
 * I860 internal register bit definitions
 *
 * $Log:	i860.h,v $
 * Revision 1.1  90/03/12  11:39:01  chris
 * Initial revision
 * 
 */

#ifndef _i860_i
#define _i860_i 1
/* Page table entries */

#define pte_p          0x001          /* Present */
#define pte_w          0x002          /* Writable */
#define pte_u          0x004          /* User page */
#define pte_wt         0x008          /* Write through */
#define pte_cd         0x010          /* Cache disable */
#define pte_a          0x020          /* Accessed */
#define pte_d          0x040          /* Dirty */
#define pte_avlshft    9              /* Shift to programmer bits */
#define pte_avlmsk     0x3            /* and width */
#define pte_pfamask    0xfffff000     /* Mask for Page Frame address */

/* Processor status register */

#define psr_pmshft     24             /* Pixel Mask shift */
#define psr_pmmsk      0xff           /* and width */
#define psr_psshft     22             /* Pixel size shift */
#define psr_psmsk      0x3            /* and width */
#define psr_scshft     17             /* Sift count shift */
#define psr_scmsk      0x1f           /* and width */
#define psr_knf        0x8000         /* Kill next floating ins. */
#define psr_dim        0x4000         /* Dual Instruction mode */
#define psr_ds         0x2000         /* Delayed switch */
#define psr_ft         0x1000         /* Floating trap */
#define psr_dat        0x0800         /* Data access trap */
#define psr_iat        0x0400         /* Instruction access trap */
#define psr_in         0x0200         /* Interrupt */
#define psr_it         0x0100         /* Instruction trap */
#define psr_pu         0x0080         /* Previous user mode */
#define psr_u          0x0040         /* User Mode */
#define psr_pim        0x0020         /* Previous interrupt mode */
#define psr_im         0x0010         /* Interrupt mode */
#define psr_lcc        0x0008         /* Loop condition code */
#define psr_cc         0x0004         /* Condition code */
#define psr_bw         0x0002         /* Break write */
#define psr_br         0x0001         /* Break read */

#define psr_trapmask  (psr_it|psr_in|psr_iat|psr_dat|psr_ft)

/* in the following psr_it is artificially set so that new processes
   will be started off with proper user and interrupt mode */
#define psr_init_lo   (psr_pu|psr_pim|psr_it)
#define psr_init_hi   (psr_pim|psr_it)
#define psr_init_hi_ni   (psr_it)

/* Extended processor status register */

#define epsr_ptypeshft 0
#define epsr_ptypemsk  0xff
#define epsr_stepshft  8
#define epsr_stepmsk   0x1f
#define epsr_il        0x00002000        /* Interlock */
#define epsr_wp        0x00004000        /* Write-protect mode */
/*             0x00008000        / * reserved */
/*             0x00001000        / * reserved */
#define epsr_int       0x00020000        /* Interrupt */
#define epsr_dcsshft   18                /* Data cache size */
#define epsr_dcsmsk    0xf               /* and width */
#define epsr_pbm       0x00400000        /* Page table bit mode */
#define epsr_be        0x00800000        /* Big-endian mode */
#define epsr_of        0x01000000        /* Overflow flag */
/* all other bits reserved */

/* Directory base register */

#define db_ate         0x0001            /* Address translate enable */
#define db_dpsshft     1
#define db_dpsmsk      0x7
#define db_bl          0x0010
#define db_iti         0x0020            /* I-cache, TLB invalidate */
/*             0x0040            / * reserved */
#define db_cs8         0x0080            /* Code size 8 */
#define db_rbshft      8                 /* Replacement block */
#define db_rbmsk       3
#define db_rcshft      10                /* Replacement control */
#define db_rcmsk       3
#define db_dtbmsk      0xfffff000        /* DTB mask */

/* Floating status register */

#define fsr_fz         0x00000001        /* Flush zero */
#define fsr_ti         0x00000002        /* Trap inexeact */
#define fsr_rmshft     2                 /* Rounding mode shift */
#define fsr_rmmsk      3                 /* and width */
#define fsr_u          0x00000010        /* Update */
#define fsr_fte        0x00000020        /* Floating-point trap enable */
/*             0x00000040        reserved */
#define fsr_si         0x00000080        /* Sticky inexact flag */
#define fsr_se         0x00000100        /* Source exception */
#define fsr_mu         0x00000200        /* Multiplier underflow */
#define fsr_mo         0x00000400        /* Multiplier overflow */
#define fsr_mi         0x00000800        /* Multiplier inexact */
#define fsr_ma         0x00001000        /* Multiplier add one */
#define fsr_au         0x00002000        /* Adder underflow */
#define fsr_ao         0x00004000        /* Adder overflow */
#define fsr_ai         0x00008000        /* Adder inexact */
#define fsr_aa         0x00010000        /* Adder add one */
#define fsr_rrshft     17                /* Result register mask */
#define fsr_rrmsk      0x1f              /* and width */
#define fsr_aeshft     22                /* Adder exponent */
#define fsr_aemsk      0x7               /* and width */
/*             0x02000000         reserved */
#define fsr_lrp        0x04000000        /* Load pipe result precision */
#define fsr_irp        0x08000000        /* Integer pipe result precision */
#define fsr_mrp        0x10000000        /* Multiplier pipe result precision */
#define fsr_arp        0x20000000        /* Adder pipe result precision */
/* bits 30-31 reserved */

/* Bits and pieces for pulling apart instructions */

#define f_src1r(x) ( ((x)>>11) & 0x1f )
#define f_src2(x) ( ((x)>>21) & 0x1f )
#define f_dest(x) ( ((x)>>16) & 0x1f )
#define f_src1i(x) ( (short)( (x) & 0xffff ) )
#define f_src1scr(x) ( (short)( (((x)>>5) & 0xf800) | ((x)&0x7ff) ) )
#define bit(n) ( 1 << (n) )
#define NOP (0xa0000000)


#endif

#endif
