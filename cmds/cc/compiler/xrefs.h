
/* xrefs.h:  Copyright (C) A. Mycroft and A.C. Norman */
/* version 0.01 */
/* $Id: xrefs.h,v 1.1 1990/09/13 17:11:20 nick Exp $ */

#ifndef _XREFS_LOADED

#define _XREFS_LOADED 1

typedef struct CodeXref {
   struct CodeXref *codexrcdr;
   int codexroff;    /* plus flags in msb. */
   Symstr *codexrsym;
   int codexrlitoff;  /* armgen.c only: for searching for suitable literal - */
                      /* holds value or address offset of codeseg literal    */
} CodeXref;

typedef struct DataXref {
   struct DataXref *dataxrcdr;
   int dataxroff;
   Symstr *dataxrsym;
} DataXref;

/* things which may appear in msb of codexrefs->codexroff */
#define X_PCreloc     0x04000000
#define X_backaddrlit 0x03000000

typedef struct ExtRef
{ struct ExtRef *extcdr;
  Symstr *extsym;
  int extindex;
  int extflags;
  int extoffset;
} ExtRef;

/* merge these next two lots of values?? */
/* 'flags' arg to obj_symref() */
#define xr_reference 0
#define xr_code   0x01
#define xr_data   0x02
#define xr_defloc 0x04
#define xr_defext 0x08

/* values for codeflags[] */
#define LIT_OPCODE  0x00000000
#define LIT_ADCON   0x01000000
#define LIT_STRING  0x02000000
#define LIT_FPNUM   0x03000000
#define LIT_FPNUM1  0x04000000
#define LIT_FPNUM2  0x05000000
#define LIT_RELADDR 0x06000000
#define LIT_NUMBER  0x07000000
/* some extra ones for vargen.c, xxxasm.c, xxxobj.c */
#define LIT_HH      0x08000000
#define LIT_BBBB    LIT_STRING
#define LIT_BBH     0x0a000000
#define LIT_HBB     0x0b000000
#define LIT_LABEL   0x0c000000

extern CodeXref *codexrefs;
extern ExtRef *obj_symlist;
extern DataXref *dataxrefs;
extern DataInit *datainitp;

#define LITF_INCODE 1
#define LITF_FIRST 2
#define LITF_LAST 4
#define LITF_PEEK 8
#define LITF_NEW  16

#endif

/* end of xrefs.h */
