#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif

/* mip/coff.h: data structures and constants for COFF,                  */
/*   loosely taken from Unix sysV operating system programmer's guide.  */
/* version 1c.                                                          */
/* Copyright (C) Codemist Ltd.                                          */

/* The following is NOT a full definition of COFF -- it includes        */
/* only those parts required by the Norcroft compiler suite.            */

struct filehdr
{  unsigned short f_magic;
   unsigned short f_nscns;
   long           f_timdat;
   long           f_symptr;
   long           f_nsyms;
   unsigned short f_opthdr;
   unsigned short f_flags;
};

struct scnhdr
{  char s_name[8];
   long s_paddr;
   long s_vaddr;
   long s_size;
   long s_scnptr;
   long s_relptr;
   long s_lnnoptr;
#ifdef TARGET_IS_88000
   unsigned long s_nreloc;
   unsigned long s_nlnno;
#else
# ifdef TARGET_IS_MIPS
   unsigned long s_nreloc;
   unsigned short s_onreloc;
   unsigned short s_nlnno;
# else
   unsigned short s_nreloc;
   unsigned short s_nlnno;
# endif
#endif
   long s_flags;
};
#define STYP_TEXT 0x20
#define STYP_DATA 0x40
#define STYP_BSS  0x80

struct reloc
{  long r_vaddr;
   long r_symndx;
   unsigned short r_type;
#ifdef TARGET_IS_88000
   unsigned short r_offset;     /* ??? high 16 bits of expressions ??? */
#  define RELSZ sizeof(struct reloc)
#else
#  define RELSZ 10        /* probably sizeof(struct reloc) = 12 */
   /* many hosts pad here */
#endif
};

/* Relocation types */
/* These are notionally system independent, but clipper R_DIR32 differs */
/* from I386.  (Clipper follows ATT 3b2).   Move to target.h?           */

#ifndef TARGET_IS_88000
#ifdef TARGET_IS_I386
#  define R_DIR32 6
#else
#  define R_DIR32 17            /* ATT 3b2, m68000, clipper */
#endif
#define R_PCRLONG 24
#endif

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

/* end of coff.h */
