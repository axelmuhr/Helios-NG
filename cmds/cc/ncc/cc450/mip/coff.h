#pragma force_top_level
#pragma include_only_once

/* mip/coff.h: data structures and constants for COFF,                  */
/*   loosely taken from Unix sysV operating system programmer's guide.  */
/* version 1d.                                                          */
/* Copyright (C) Codemist Ltd, 1988, 1991.                              */
/* MIPS parts: (C) Copyright 1984 by Third Eye Software, Inc.           */
/*   Third Eye Software, Inc. grants reproduction and use rights to     */
/*   all parties, PROVIDED that this comment is maintained in the copy. */

/* The following is NOT a full definition of COFF -- it includes        */
/* only those parts required by the Norcroft compiler suite.            */

struct filehdr
{  unsigned short f_magic;
   unsigned short f_nscns;
   long f_timdat;
   long f_symptr;               /* beware: MIPS is symbolic header ptr  */
   long f_nsyms;                /* on MIPS is sizeof symbolic header.   */
   unsigned short f_opthdr;
   unsigned short f_flags;
};

#ifdef TARGET_IS_MIPS
/* On MIPS targets, there must be an 'optional' header...               */
#define OMAGIC 0407
#ifdef TARGET_IS_STARDENT
#define MIPSvstamp 0x1a00
#else
#define MIPSvstamp 0x011f
#endif

typedef struct aouthdr {
        short   magic;          /* see above                            */
        short   vstamp;         /* version stamp                        */
        long    tsize;          /* text size in bytes, padded to DW bdry*/
        long    dsize;          /* initialized data "  "                */
        long    bsize;          /* uninitialized data "   "             */
        long    entry;          /* entry pt.                            */
        long    text_start;     /* base of text used for this file      */
        long    data_start;     /* base of data used for this file      */
        long    bss_start;      /* base of bss used for this file       */
#ifdef TARGET_IS_STARDENT
        long    FIX_flags;      /* bits describing h'ware fixes applied */
        long    tlsize;         /* thread-local data area size */
        long    tl_start;       /* starting address of thread-local */
        long    stack_start;    /* starting address of stack            */
        long    stamp;          /* version/testing stamp                */
#else
        long    gprmask;        /* general purpose register mask        */
        long    cprmask[4];     /* co-processor register masks          */
#endif
        long    gp_value;       /* the gp value used for this object    */
} AOUTHDR;
#define GP_DISP 0x8000
#endif

struct scnhdr
{  char s_name[8];
   long s_paddr;
   long s_vaddr;
   long s_size;
   long s_scnptr;
   long s_relptr;
#ifndef TARGET_IS_STARDENT
   long s_lnnoptr;
#ifdef TARGET_IS_88000
   unsigned long s_nreloc;
   unsigned long s_nlnno;
#else
   unsigned short s_nreloc;
   unsigned short s_nlnno;
#endif
#else
   unsigned     s_nreloc;       /* the (new) number of reloc. entries */
   unsigned short       o_nreloc;       /* the old nreloc */
   unsigned short       filler2;        /* number of line numbers (not used) */
#endif
   long s_flags;
};
#define STYP_TEXT 0x20
#define STYP_DATA 0x40
#define STYP_BSS  0x80

/* relocation records ... */

struct reloc
{  long         r_vaddr;
#ifdef TARGET_IS_MIPS
   unsigned     r_symndx:24,    /* index into symbol table */
                r_reserved:3,
                r_type:4,       /* relocation type */
                r_extern:1;     /* if 1 symndx is an index into the external
                                   symbol table, else symndx is a section # */
#  define RELSZ sizeof(struct reloc)
#else
   long r_symndx;
   unsigned short r_type;
#ifdef TARGET_IS_88000
   unsigned short r_offset;     /* ??? high 16 bits of expressions ??? */
#  define RELSZ sizeof(struct reloc)
#else
#  define RELSZ 10        /* probably sizeof(struct reloc) = 12 */
   /* many hosts pad here */
#endif
#endif
};

/* Relocation types */
/* These are notionally system independent, but clipper R_DIR32 differs */
/* from I386.  (Clipper follows ATT 3b2).   Move to target.h?           */

#ifdef TARGET_IS_MIPS
/*
 * Mips machines
 *
 *      16-bit reference
 *      32-bit reference
 *      26-bit jump reference
 *      reference to high 16-bits       BUT what about carryout from LO???
 *      reference to low 16-bits
 *      reference to global pointer relative data item
 *      reference to global pointer relative literal pool item
 */
#define R_SN_TEXT       1       /* r_symndx values if r_extern == 0     */
#define R_SN_DATA       3
#define R_SN_BSS        6
#define R_REFHALF       1       /* r_type values.                       */
#define R_REFWORD       2
#define R_JMPADDR       3
#define R_REFHI         4
#define R_REFLO         5
#define R_GPREL         6
#define R_LITERAL       7
#else
#ifndef TARGET_IS_88000
#ifdef TARGET_IS_I386
#  define R_DIR32 6
#else
#  define R_DIR32 17            /* ATT 3b2, m68000, clipper */
#endif
#define R_PCRLONG 24
#endif
#endif

/* symbol tables... */

#ifdef TARGET_IS_MIPS           /* MIPS has a special symbol table(!) */
#define magicSym        0x7009
typedef struct {
    short   magic;          /* to verify validity of the table */
    short   vstamp;         /* version stamp */
#ifdef TARGET_IS_STARDENT
        long    filler1[15];    /* for Mips compatibility */
        long    issExtMax;      /* strings_ct number of bytes of strings */
        long    cbSsExtOffset;  /* strings_off offset of strings */
        long    comini_ct;      /* initialized common data */
        long    comini_off;     /* offset of initialized common data */
        long    address_ct;     /* number of text-relocated addresses */
        long    address_off;    /* offset of text-relocated addresses */
#else
    long    ilineMax;       /* number of line number entries */
    long    cbLine;         /* number of bytes for line number entries */
    long    cbLineOffset;   /* offset to start of line number entries*/
    long    idnMax;         /* max index into dense number table */
    long    cbDnOffset;     /* offset to start dense number table */
    long    ipdMax;         /* number of procedures */
    long    cbPdOffset;     /* offset to procedure descriptor table */
    long    isymMax;        /* number of local symbols */
    long    cbSymOffset;    /* offset to start of local symbols */
    long    ioptMax;        /* max index into optimization symbol entries */
    long    cbOptOffset;    /* offset to optimization symbol entries */
    long    iauxMax;        /* number of auxillary (sic) symbol entries */
    long    cbAuxOffset;    /* offset to start of auxillary symbol entries*/
    long    issMax;         /* max index into local strings */
    long    cbSsOffset;     /* offset to start of local strings */
    long    issExtMax;      /* max index into external strings */
    long    cbSsExtOffset;  /* offset to start of external strings */
    long    ifdMax;         /* number of file descriptor entries */
    long    cbFdOffset;     /* offset to file descriptor table */
    long    crfd;           /* number of relative file descriptor entries */
    long    cbRfdOffset;    /* offset to relative file descriptor table */
#endif
    long    iextMax;        /* max index into external symbols */
    long    cbExtOffset;    /* offset to start of external symbol entries*/
    /* If you add machine dependent fields, add them here */
} HDRR;
typedef struct fdr {        /* file descriptor entry (at least 1 needed) */
        unsigned long   adr;    /* memory address of beginning of file */
        long    rss;            /* file name (of source, if known) */
        long    issBase;        /* file's string space */
        long    cbSs;           /* number of bytes in the ss */
        long    isymBase;       /* beginning of symbols */
        long    csym;           /* count file's of symbols */
        long    ilineBase;      /* file's line symbols */
        long    cline;          /* count of file's line symbols */
        long    ioptBase;       /* file's optimization entries */
        long    copt;           /* count of file's optimization entries */
        short   ipdFirst;       /* start of procedures for this file */
        short   cpd;            /* count of procedures for this file */
        long    iauxBase;       /* file's auxiliary entries */
        long    caux;           /* count of file's auxiliary entries */
        long    rfdBase;        /* index into the file indirect table */
        long    crfd;           /* count file indirect entries */
        unsigned lang: 5;       /* language for this file */
        unsigned fMerge : 1;    /* whether this file can be merged */
        unsigned fReadin : 1;   /* true if it was read in (not just created) */
        unsigned fBigendian : 1;/* if set, was compiled on big endian machine */
                                /*      aux's will be in compile host's sex */
        unsigned glevel : 2;    /* level this file was compiled with */
        unsigned reserved : 22;  /* reserved for future use */
        long    cbLineOffset;   /* byte offset from header for this file ln's */
        long    cbLine;         /* size of lines for this file */
} FDR;
#ifdef TARGET_IS_STARDENT
typedef struct
{
        unsigned        auxval;         /* offset for initialization */
        unsigned        iss;            /* index into string table */
        int             value;          /* size for common, value for others */
        unsigned        flags:8,        /* what sort of value */
                        index:24;       /* hash of common block fill */
}  DL_XSYM;
/* flags */
/* for Mips compatibility, they go up by 4's */
# define DL_NIL         0x00
# define DL_TEXT        0x04
# define DL_DATA        0x08
# define DL_BSS         0x0c
# define DL_COMM        0x010
# define DL_ABS         0x014
        /* DL_UNDEF is needed only to distinguish from DL_NIL (== ignored)  */
# define DL_UNDEF       0x018
# define DL_COMINI      0x01c
# define DL_THREAD      0x001
# define DL_ENTRY       0x002           /* for FORTRAN entry point */
# define DL_DEF         0x020           /* symbol is defined/initialized */
# define DL_LOCAL       0x40            /* local scope for symbol */
# define DL_FLAG        0x80            /* flag to mark old common regions */
#else
typedef struct {            /* (internal) symbol record */
    long    iss;            /* index into String Space of name */
    long    value;          /* value of symbol */
    unsigned st : 6;        /* symbol type */
    unsigned sc  : 5;       /* storage class - text, data, etc */
    unsigned reserved : 1;  /* reserved */
    unsigned index : 20;    /* index into sym/aux table */
} SYMR;
#define scNil           0

#define scText          1       /* text symbol */
#define scData          2       /* initialized data symbol */
#define scBss           3       /* un-initialized data symbol */
#define scUndefined     6       /* who knows? */
#define stGlobal        1       /* external symbol */

typedef struct {            /* (external) symbol record */
#ifdef never
    unsigned jmptbl:1;      /* symbol is a jump table entry for shlibs */
    unsigned cobol_main:1;  /* symbol is a cobol main procedure */
    unsigned reserved:14;   /* reserved for future use */
#else
    short reserved;
#endif
    short   ifd;            /* where the iss and index fields point into */
    SYMR    asym;           /* symbol for the external */
} EXTR;
#endif
#define indexNil 0xfffff

#else /* TARGET_IS_MIPS */
#define SYMNMLEN 8
struct syment
{  union
   {  char _n_name[SYMNMLEN];
      struct
      {  long _n_zeroes;
         long _n_offset;
      } _n_n;
   } _n;
   unsigned long n_value;
   short n_scnum;
   unsigned short n_type;
   char n_sclass;
   char n_numaux;
#ifdef TARGET_IS_88000
   char n_pad1, n_pad2;
#  define SYMESZ sizeof(struct syment)
#else
#  define SYMESZ 18       /* probably sizeof(struct reloc) = 20 */
   /* many hosts pad here */
#endif
};
#define n_name _n._n_name
#define n_zeroes _n._n_n._n_zeroes
#define n_offset _n._n_n._n_offset

#define N_UNDEF 0
#define T_NULL 0
#define C_EXT 2
#define C_STAT 3
#endif /* TARGET_IS_MIPS */

/* end of coff.h */
