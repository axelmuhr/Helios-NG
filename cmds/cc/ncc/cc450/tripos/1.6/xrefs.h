
/* xrefs.h:  Copyright (C) A. Mycroft and A.C. Norman */
/* version 2d */

#ifndef _XREFS_LOADED

#define _XREFS_LOADED 1

typedef struct CodeXref {
   struct CodeXref *codexrcdr;
   int32 codexroff;    /* plus flags in msb. */
   Symstr *codexrsym;
   int32 codexrlitoff;/* armgen.c only: for searching for suitable literal - */
                      /* holds value or address offset of codeseg literal    */
   int32 codexrpos;   /* Helios only: holds position in obj file to be  */
                      /* patched */
} CodeXref;
/* for the 68000 codexrcode holds the offset from the symbol.         */
/* The symbol offset from the segment base needs to be added to this  */
/* when a reloc32 relocation is output.                               */
#define codexrcode codexrlitoff

typedef struct DataXref {
   struct DataXref *dataxrcdr;
   int32 dataxroff;
   Symstr *dataxrsym;
} DataXref;

/* things which may appear in msb of codexrefs->codexroff */
#define X_absreloc    0x02000000L    /* only used by clipper so far     */
#define X_backaddrlit 0x03000000L    /* same as absreloc, but used ONLY */
                                     /* by vargen.c to share literals   */
#define X_PCreloc     0x04000000L

#ifdef HELIOS
#define X_DataSymb    0x05000000L
#define X_DataModule  0x06000000L
#define X_Modnum      0x07000000L
#define X_Init        0x08000000L
extern void outINIT(void);
#endif

typedef struct ExtRef
{ struct ExtRef *extcdr;
  Symstr *extsym;
  int32 extindex;
  int extflags;    /* xr_xxx things below */
  int32 extoffset;
} ExtRef;

/* merge these next two lots of values?? */
/* 'flags' arg to obj_symref() */
#define xr_code   0x01
#define xr_data   0x02
#define xr_defloc 0x04
#define xr_defext 0x08
/* the following two values are only meaningful (or indeed, correctly set) */
/* if TARGET_CALL_USES_DESCRIPTOR is defined (e.g. 32016)                  */
/* Between them, one is set if the symbol has been referenced.             */
#define xr_refcode 0x10
#define xr_refdata 0x20
#ifdef HELIOS
#define xr_sent 0x40
#endif
/* values for codeflags[] */
#define LIT_OPCODE  0x00000000L
#define LIT_ADCON   0x01000000L
#define LIT_STRING  0x02000000L
#define LIT_FPNUM   0x03000000L
#define LIT_FPNUM1  0x04000000L
#define LIT_FPNUM2  0x05000000L
#define LIT_RELADDR 0x06000000L
#define LIT_NUMBER  0x07000000L
/* some extra ones for vargen.c, xxxasm.c, xxxobj.c */
#define LIT_HH      0x08000000L
#define LIT_BBBB    LIT_STRING
#define LIT_BBH     0x0a000000L
#define LIT_HBB     0x0b000000L
#define LIT_LABEL   0x0c000000L
#define LIT_FNCON   0x0d000000L     /* only if TARGET_CALL_USES_DESCRIPTOR */

extern CodeXref *codexrefs;
extern ExtRef *obj_symlist;
extern DataXref *dataxrefs;
extern DataXref *dbgxrefs;
extern DataInit *datainitp;
extern int32 obj_symref(Symstr *s, int flags, int32 loc);

#define LITF_INCODE 1L
#define LITF_FIRST 2L
#define LITF_LAST 4L
#define LITF_PEEK 8L
#define LITF_NEW  16L

#endif

/* end of xrefs.h */
