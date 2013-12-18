#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * xrefs.h, version 2q
 * Copyright (C) Codemist Ltd, 1988.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1993/08/03 15:00:29 $
 * Revising $Author: nickc $
 */

#ifndef _xrefs_LOADED
#define _xrefs_LOADED 1

typedef struct CodeXref {
   struct CodeXref *codexrcdr;
   int32 codexroff;    /* plus flags in msb. */
   Symstr *codexrsym;
/* The next comment is out of date-- see codebuf.c                           */
   int32 codexrlitoff;/* armgen.c only: for searching for suitable literal - */
                      /* holds value or address offset of codeseg literal    */
/*#ifdef TARGET_IS_HELIOS*/
   int32 codexrpos;   /* holds position in obj file to be patched           */
#  define codexrcode codexrlitoff
      /* for Helios codexrcode holds the offset from the symbol.            */
      /* The symbol offset from the segment base needs to be added to this  */
      /* when a reloc32 relocation is output.                               */
/*#endif*/
} CodeXref;

typedef struct DataXref {
   struct DataXref *dataxrcdr;
   int32 dataxroff;
   Symstr *dataxrsym;
} DataXref;

/* Things which may appear in msb of codexrefs->codexroff.              */
/* AM: these are a bit (lot) in flux and need naming tidying.           */
/* Note that all dataseg relocs in dataxrefs->dataxroff are currently   */
/* implicitly absreloc.                                                 */
#define X_absreloc    0x01000000L   /* 32 bit abs (direct) reloc.       */
/* Note that X_absreloc need not be word aligned even if data is (e.g.  */
/* as opcode address field.  X_absreloc only used by clipper so far.    */
#define X_backaddrlit 0x02000000L   /* same as absreloc, but used ONLY  */
                                    /* by vargen.c to share literals    */
/* The following codes are only used on some machines.                  */
#define X_PCreloc     0x03000000L   /* PC relative CALL/BR, e.g. ARM.   */
/* X_DataAddr relocates immediate field(s) to load an ADCON into a      */
/* register.  E.g. use of 3 ADD/SUB's on 27bit ARM or OR.H;OR.U on      */
/* m88k, i860, amd29000 etc.  As an alternative some machines do this   */
/* by loading an ADCON literal (X_absreloc) from pc-relative addr mode. */
/* We MAY need to distinguish X_DataAddr from X_FnAddr for 32000 etc.   */
/* Helios needs similar things.                                         */
#define X_DataAddr    0x04000000L   /* Immediate field in load address  */
#define X_DataVal     0x05000000L   /* Ditto but for load/store.        */
#define X_DataAddr1   0x06000000L   /* Immediate field (high part-word) */
#define X_DataVal1    0x07000000L

#ifdef TARGET_IS_HELIOS
#define X_DataSymb    	0x08000000L   /* same as DataAddr/DataVal???      */
#define X_DataModule  	0x09000000L
#define X_Modnum      	0x0a000000L
#define X_Init        	0x0b000000L

#ifdef TARGET_IS_C40

#define X_DataModule1  	0x0c000000L   /* function calling stub generation */
#define X_DataModule2 	0x0d000000L
#define X_DataModule3  	0x0e000000L
#define X_DataModule4  	0x0f000000L
#define X_DataModule5  	0x10000000L
#define X_DataSymbHi  	0x11000000L   /* loading high part of integer constants */
#define X_DataSymbLo  	0x12000000L   /* loading low  part of integer constants */
#define X_FuncAddr	0x13000000L   /* like X_DataAddr, but for function addresses */
#define X_PCreloc2      0x14000000L   /* like X_PCreloc but for conditional calls  */
#endif /* TARGET_IS_C40 */

#ifdef TARGET_IS_ARM
/* I think I have more of these than are really needed... prune someday */
#  define X_PCreloc1  	0x0c000000L   /* pc reloc but in ADD instr        */
#  define X_DataSymb1 	0x0d000000L   /* address of data using ADD instr  */
#  define X_PCreloc2  	0x0e000000L   /* pc reloc but in ADD instr        */
#  define X_DataSymb2 	0x0f000000L   /* address of data using ADD instr  */
#  define X_PCreloc3  	0x10000000L   /* pc reloc but in ADD instr        */
#  define X_DataSymb3 	0x11000000L   /* address of data using ADD instr  */
#  define X_PCreloc4  	0x12000000L   /* pc reloc but in ADD instr        */
#  define X_DataSymb4 	0x13000000L   /* address of data using ADD instr  */
#  define X_PCreloc5  	0x14000000L   /* pc reloc but in ADD instr        */
#  define X_DataSymb5 	0x15000000L   /* address of data using ADD instr  */
#  define X_DataSymb6 	0x16000000L

#endif /* TARGET_IS_ARM */

#ifdef TARGET_HAS_DEBUGGER

#define X_Debug_Modnum	0x17000000L	/* Modnum patch for debugger support */
#define X_Debug_Offset	0x18000000L	/* Offset patch for debugger support */
#define X_Debug_Ref	0x19000000L	/* Labref patch for debugger support */

#endif

#endif /* TARGET_IS_HELIOS */

#ifdef TARGET_IS_ACW
#  define X_codelink  0x0c000000L    /* WGD: for ACW  */
#  define X_datalink  0x0d000000L    /* WGD: for ACW  */
#  define X_sysname   0x10000000L    /* WGD: for ACW  */
#endif

#ifdef TARGET_IS_SPARC
#  define X_PCw30reloc	0x08000000L
#endif

typedef struct ExtRef
{ struct ExtRef *extcdr;
  Symstr *extsym;
  int32 extindex;
  int extflags;            /* xr_xxx things below */
  int32 extoffset;
  Symstr *codesym;         /* for code defs when one area per fn */
} ExtRef;

/*
 * 'flags' arg to obj_symref().
 * No bits set => reference to untyped symbol.
 */
#define xr_code   0x01         /* defined as code, or imported as code */
#define xr_data   0x02         /* defined as data, or imported as data */
#define xr_defloc 0x04         /* local (= static) definition.         */
#define xr_defext 0x08         /* exported definition.                 */
/* The following bits are not used (as not needed) by all back-ends:   */
#define xr_comref 0x10         /* OR-ed with xr_data in COMMON ref.    */
#define xr_cblock 0x20         /* common block definition              */
#define xr_objflg 0x40         /* private use by object code formatter */
#ifdef TARGET_HAS_BSS
#  define xr_bss  0x80
#endif

/* (Currently for Acorn AOF object format only), bits in the           */
/* 0xffffff00 area are used for common block index numbers.            */
/* The following macros sugar the conversion between the flag field    */
/* and index values.                                                   */
#define xr_flagtoidx_(n)       ((n) >> 8)
#define xr_idxtoflag_(n)       ((n) << 8)

/* Due to an idleness (actually silly optimisation when ~TARGET_CALL...*/
/*  ...USES_DESCRIPTOR) xr_data may get spuriously set, so always test */
/* xr_code.  E.g. { extern int f(); g(f); }  @@@ AM to fix soon.       */

/* Values for code_flag_(byteaddr).  One byte of flag per code word.    */
/* See mip/codebuf.h.  One byte gives us enough room for the following  */
/* and possible extensions on byte-oriented machines of 2 bytes opcode  */
/* plus 2 bytes literal.  Currently such things do not happen.          */
/* They are used (a) for disassembly (when they are aided by aux        */
/* entries in code_aux_() and (b) for object output (when cross-sex     */
/* compiling).  Currently exactly those tagged ($) below have a         */
/* code_aux_() entry.                                                   */
/* AM thinks that maybe we need LIT_OPCODE to be an OR-ed in bit to     */
/* deal with thinks like fp literals (88000) or 2*16bit addressing.     */
#define LIT_OPCODE  0x00        /* all 4 bytes opcode.                  */
#define LIT_RELADDR 0x01        /* ($) all 4 bytes opcode, but flags    */
                                /* that code_aux_ has an entry.  E.g.   */
                                /* BL foo.  Redundant?                  */
#define LIT_NUMBER  0x02        /* 4 byte integer constant literal      */
#define LIT_ADCON   0x03        /* 4 byte address constant literal      */
#define LIT_FPNUM   0x04        /* ($) 4 byte floating constant literal */
#define LIT_FPNUM1  0x05        /* ($) first 4 bytes of double literal  */
#define LIT_FPNUM2  0x06        /* ($) second 4 bytes of double literal */
#define LIT_STRING  0x10        /* 4 bytes of string literal            */
/* old ACW implementations need gaps 0x10-13 for extra LIT_STRING info. */
/* The above are also used to flag data values (in struct DataInit),    */
/* with the exception of LIT_OPCODE and LIT_RELADDR, but the following  */
/* additional values are also used (but beware that LIT_FPNUM is used   */
/* for all FP data values which are discriminated by FloatCon fields).  */
#define LIT_HH      0x08        /* 2 halfwords datainit in host sex     */
#define LIT_BBBB    LIT_STRING  /* 4 bytes datainit in host sex         */
#define LIT_BBH     0x0a        /* 2 bytes then halfword in host sex    */
#define LIT_HBB     0x0b        /* halfword then 2 bytes in host sex    */
#define LIT_LABEL   0x0c        /* static/extern defined here.          */
#define LIT_FNCON   0x0d        /* an alternative to LIT_ADCON, either  */
                                /* vestigial (obj_symref subsumes) or   */
                                /* up-and-coming.  Currenltly only used */
                                /* if TARGET_CALL_USES_DESCRIPTOR.      */
#if (sizeof_ptr == 2)
#define LIT_HX      0x0e        /* one halfword only (host sex)         */
#define LIT_BBX     0x0f        /* two bytes only (host sex)            */
#endif

extern CodeXref *codexrefs;
extern ExtRef *obj_symlist;
extern DataXref *dataxrefs;
extern DataXref *dbgxrefs;
extern DataInit *datainitp;
extern int32 obj_symref(Symstr *s, int flags, int32 loc);

#define LITF_INCODE  1L
#define LITF_FIRST   2L
#define LITF_LAST    4L
#define LITF_PEEK    8L
#define LITF_NEW    16L
#define LITF_DOUBLE 32L

#ifdef TARGET_HAS_BYTE_INSTRUCTIONS     /* currently ACW only */
#define LITB_OPCODE   0
#define LITB_BASE     1
#define LITB_CASE     2
#define LITB_DISP     3
#define LITB_CHAR     4
#define LITB_PROVIGN  5
#define LITB_IGN      6
#define LITB_HEX      7
#define LITB_FLOATSTR 8

#define COUNTMAGIC  0xE6E6DEE6L
#define FUNMAGIC    0xFEE6E600L

#endif /* TARGET_HAS_BYTE_INSTRUCTIONS */

#endif

/* end of xrefs.h */
