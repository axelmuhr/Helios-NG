/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- module.h                        --
--                                                                      --
--   Module and Program structures               --
--                                                                      --
--   Author:  NHG 16/8/87                  --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%   %G% Copyright (C) 1987, Perihelion Software Ltd.   */

#ifndef __module_h
#define __module_h

#ifndef __helios_h
#include "helios.h"
#endif

/* Code module header */

typedef struct Module {
   word      Type;      /* module type = T_Module   */
   word      Size;      /* size of module in bytes   */
   char      Name[32];   /* module name         */
   word      Id;      /* module table index      */
   word      Version;   /* version number of this module*/
   word      MaxData;   /* highest data offset       */
   RPTR      Init;      /* root of init chain      */
} Module;

/* Reference to resident module */

typedef struct ResRef {
   WORD      Type;      /* T_ResRef         */
   WORD      Size;      /* = sizeof(ResRef)      */
   UBYTE      Name[32];   /* name of module required   */
   WORD      Id;      /* module table index      */
   WORD      Version;   /* version number of module   */
   Module      *Module;   /* pointer to module      */
} ResRef;

/* type codes */

#define T_Program   0x60f060f0l
#define   T_Module   0x60f160f1l
#define T_ResRef        0x60f260f2l
#define T_Proc          0x60f360f3l
#define T_Code          0x60f460f4l
#define T_Stack         0x60f560f5l
#define T_Static        0x60f660f6l
#define T_ProcInfo   0x60f760f7l

/* Program structure */

typedef struct Program {
   Module      Module;      /* start of module      */
   WORD      Stacksize;   /* size of initial stack   */
   WORD      Heapsize;   /* size of program heap      */
   RPTR      Main;      /* offset of main entry point   */
} Program;

/* Image file header */

typedef struct ImageHdr {
   word      Magic;      /* = Image_Magic      */
   word      Flags;      /* zero for now         */
   word      Size;      /* image size         */
} ImageHdr;

#define Image_Magic   0x12345678l   /* magic number         */

typedef struct Proc {
   word      Type;      /* T_Proc         */
   RPTR      Proc;      /* start of procedure      */
   char      Name[Variable];   /* procedure name      */
} Proc;

typedef struct ProcInfo {
   word      Type;      /* T_ProcInfo         */
   word      Size;      /* procedure size in bytes   */
   word      StackUse;   /* stack usage in words      */
   word      VstackUse;   /* vector stack use in words   */
} ProcInfo;

#endif

/* -- End of module.h */
