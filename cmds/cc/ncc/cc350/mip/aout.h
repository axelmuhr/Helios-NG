#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * Modified a.out.h for use with ARM.   In fact the mod is to
 * a comment only and concerns the value of r_length in a
 * relocation record when the relocated object is an ARM branch.
 * LJA.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:00:39 $
 * Revising $Author: nickc $
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      @(#)a.out.h     5.1 (Berkeley) 5/30/85
 */

/*
 * For cross-compilation
 */
#define PAGESIZE 32768

/*
 * Header prepended to each a.out file.
 */
struct exec {
#ifdef TARGET_IS_SPARC
	unsigned char   a_toolversion;	/* dynamic + what is this? */
	unsigned char	a_machtype;	/* machine type */
	unsigned short	a_magic;	/* magic number */
#else
        long    a_magic;        /* magic number */
#endif
unsigned long   a_text;         /* size of text segment */
unsigned long   a_data;         /* size of initialized data */
unsigned long   a_bss;          /* size of uninitialized data */
unsigned long   a_syms;         /* size of symbol table */
unsigned long   a_entry;        /* entry point */
unsigned long   a_trsize;       /* size of text relocation */
unsigned long   a_drsize;       /* size of data relocation */
#ifdef TARGET_IS_CLIPPER
 unsigned int                   /* Per-process flags */
  a_format:1,                   /* If true, rest are defined */
  a_rz:1,                       /* If true, zero is readable */
  a_ip:1,                       /* If true, enable instruction prefetch */
  a_tcs:3,                      /* Text cache strategy */
  a_dcs:3,                      /* Data cache strategy */
  a_scs:3,                      /* Stack cache strategy */
  a_dummy:20;
#endif
};

#ifdef TARGET_IS_CLIPPER
#define OMAGIC  01407           /* old impure format */
#define NMAGIC  01410           /* read-only text */
#define ZMAGIC  01413           /* demand load format */
#else
#define OMAGIC  0407            /* old impure format */
#define NMAGIC  0410            /* read-only text */
#define ZMAGIC  0413            /* demand load format */
#endif

#define M_OLDSUN2	0	/* old sun-2 executable files */
#define M_68010		1	/* runs on either 68010 or 68020 */
#define M_68020		2	/* runs only on 68020 */
#define M_SPARC		3	/* runs on SPARC only */

/*
 * Macros which take exec structures as arguments and tell whether
 * the file has a reasonable magic number or offsets to text|symbols|strings.
 */
#define N_BADMAG(x) \
    (((x).a_magic)!=OMAGIC && ((x).a_magic)!=NMAGIC && ((x).a_magic)!=ZMAGIC)

#define N_TXTOFF(x) \
        ((x).a_magic==ZMAGIC ? PAGESIZE : sizeof (struct exec))
#define N_SYMOFF(x) \
        (N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize+(x).a_drsize)
#define N_STROFF(x) \
        (N_SYMOFF(x) + (x).a_syms)

/*
 * Format of a relocation datum.
 */
#ifdef TARGET_IS_SPARC
struct relocation_info {
  unsigned int	r_address;
  unsigned int	r_symbolnum	:24,
		r_extern	:1,
  				:2,
  		r_type		:5;
  int		r_addend;
};
#define RELOC_32	2
#define RELOC_WDISP30	6
#define RELOC_WDISP22	7
#define RELOC_HI22	8
#define RELOC_22	9
#define RELOC_13	10
#define RELOC_LO10	11
#else
struct relocation_info {
        int     r_address;      /* address which is relocated */
unsigned int    r_symbolnum:24, /* local symbol ordinal */
                r_pcrel:1,      /* was relocated pc relative already */
                r_length:2,     /* 0=byte, 1=word, 2=long 3=ARM branch */
                r_extern:1,     /* does not include value of sym referenced */
                r_neg:1,        /* -ve relocation */
                :3;             /* nothing, yet */
};
#endif
#define RELINFOSZ 8

/*
 * Format of a symbol table entry; this file is included by <a.out.h>
 * and should be used if you aren't interested the a.out header
 * or relocation information.
 */
struct  nlist {
        union {
                char    *n_name;        /* for use when in-core */
                long    n_strx;         /* index into file string table */
        } n_un;
unsigned char   n_type;         /* type flag, i.e. N_TEXT etc; see below */
        char    n_other;        /* unused */
        short   n_desc;         /* see <stab.h> */
unsigned long   n_value;        /* value of this symbol (or sdb offset) */
};
#define NLISTSZ 12
#define n_hash  n_desc          /* used internally by ld */

/*
 * Simple values for n_type.
 */
#define N_UNDF  0x0             /* undefined */
#define N_ABS   0x2             /* absolute */
#define N_TEXT  0x4             /* text */
#define N_DATA  0x6             /* data */
#define N_BSS   0x8             /* bss */
#define N_COMM  0x12            /* common (internal to ld) */
#define N_FN    0x1f            /* file name symbol */

#define N_EXT   01              /* external bit, or'ed in */
#define N_TYPE  0x1e            /* mask for all the type bits */

/*
 * Sdb entries have some of the N_STAB bits set.
 * These are given in <stab.h>
 */
#define N_STAB  0xe0            /* if any of these bits set, a SDB entry */

/*
 * Format for namelist values.
 */
#define N_FORMAT        "%08x"

