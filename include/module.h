/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- module.h								--
--                                                                      --
--	Module and Program structures					--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RcsId: $Id: module.h,v 1.14 1993/12/13 11:48:38 tony Exp $ */

#ifndef __module_h
#define __module_h

#ifndef __helios_h
#include <helios.h>
#endif

#ifndef offsetof

#define offsetof(type, member) ((char *)&(((type *)0)->member) - (char *)0)

#endif

/* Code module header */

struct Module
{
	word		Type;		/* module type = T_Module	*/
	word		Size;		/* size of module in bytes	*/
	char		Name[32];	/* module name			*/
	word		Id;		/* module table index		*/
	word		Version;	/* version number of this module*/
	word		MaxData;	/* highest data offset 		*/
	RPTR		Init;		/* root of init chain		*/
#ifdef __SMT
	word		MaxCodeP;	/* highest code offset		*/
#endif
};

#ifndef __cplusplus
typedef struct Module Module;
#endif

#define ModuleWord_(m,field) MWord_(m,offsetof(Module,field))
#define ModuleName_(dest,m) MData_(dest,m,offsetof(Module,Name),32)
#define ModuleInit_(m) MInc_(m,offsetof(Module,Init))
#define ModuleNext_(m) MInc_(m,ModuleWord_(m,Size))

/* Reference to resident module */

struct ResRef
{
	WORD		Type;		/* T_ResRef			*/
	WORD		Size;		/* = sizeof(ResRef)		*/
	UBYTE		Name[32];	/* name of module required	*/
	WORD		Id;		/* module table index		*/
	WORD		Version;	/* version number of module	*/
	MPtr		Module;		/* pointer to module		*/
};

#ifndef __cplusplus
typedef struct ResRef  ResRef;
#endif


#define ResRefWord_(m,field) MWord_(m,offsetof(ResRef,field))
#define ResRefName_(dest,m) MData_(dest,m,offsetof(ResRef,Name),32)
#define SetResRefModule_(m,v) SetMWord_(m,offsetof(ResRef,Module),v)

/* type codes */

#define T_Mask		0xfff0fff0l	/* mask for constant bits 	*/
#define T_Valid		0x60f060f0l	/* contant bit in type code	*/

#define T_Program	0x60f060f0l
#define	T_Module	0x60f160f1l
#define T_ResRef        0x60f260f2l
#define T_Proc          0x60f360f3l
#define T_Code          0x60f460f4l
#define T_Stack         0x60f560f5l
#define T_Static        0x60f660f6l
#define T_ProcInfo	0x60f760f7l
#define T_Device	0x60f860f8l
#define T_FileName	0x60f960f9l
#define T_DevInfo	0x60fa60fal
#define T_RomDisk	0x60fb60fbl

/* Program structure */

struct Program
{
        Module		Module;		/* start of module		*/
	WORD		Stacksize;	/* size of initial stack	*/
	WORD		Heapsize;	/* size of program heap		*/
	RPTR		Main;		/* offset of main entry point	*/
};

#ifndef __cplusplus
typedef struct Program Program;
#endif
  
#define ProgramWord_(p,field) MWord_(p,offsetof(Program,field))
#define ProgramMain_(p) MInc_(p,offsetof(Program,Main))

/* Image file header */

struct ImageHdr
{
	word		Magic;		/* = ??????_Magic		*/
	word		Flags;		/* zero for now			*/
	word		Size;		/* image size			*/
};

#ifndef __cplusplus
typedef struct ImageHdr ImageHdr;
#endif

/*
 * Magic Numbers
 *
 * Cannot use #elif as SUN compiler cannot cope!
 */

#ifdef __TRAN
# define Image_Magic		0x12345678L	/* simple program	*/
#else
# if defined(__C40)
#  define Image_Magic		0xc4045601L	/* TMS320C40 program	*/
# else
#  if defined(__ARM)
#   define Image_Magic		0x0a245601L	/* ARM program		*/
#  else
#   if defined(__I860)
#    define Image_Magic		0x86045601L	/* i860 program		*/
#   else
#    if defined(__M68K)
#     define Image_Magic	0x68045601L	/* 68000 program	*/
#    endif
#   endif
#  endif
# endif
#endif

#define TaskForce_Magic 	0x12345677L	/* CDL object file	*/
#define RmLib_Magic		0x12345676L	/* Network object	*/

#ifdef __ARM
/* This obese function header combines the Helios and ARM C function headers */
struct Proc
{
	word		Type;		/* T_Proc			*/
	RPTR		Proc;		/* start of procedure		*/
	char		Name[Variable];	/* procedure name		*/
	RPTR		ProcName;	/* start of name		*/
					/* high byte = ff		*/
};
#else
# if defined(__C40)
struct Proc
{
	char		Name[Variable];	/* procedure name		*/
	word		ProcSize;	/* bytes above used for name	*/
					/* + high byte = ff		*/
};
# else
/* transputer version */
struct Proc
{
	word		Type;		/* T_Proc			*/
	RPTR		Proc;		/* start of procedure		*/
	char		Name[Variable];	/* procedure name		*/
};
# endif
#endif

#ifndef __cplusplus
typedef struct Proc Proc;
#endif

struct ProcInfo
{
	word		Type;		/* T_ProcInfo			*/
	word		Size;		/* procedure size in bytes	*/
	word		StackUse;	/* stack usage in words		*/
	word		VstackUse;	/* vector stack use in words	*/
	word		Modnum;		/* module number		*/
	word		Offset;		/* offset in data/code section	*/
	word		Reserve1;	/* reserved for later use	*/
	word		Reserve2;	/* ditto			*/
};

#ifndef __cplusplus
typedef struct  ProcInfo ProcInfo;
#endif

#endif /* __module_h */

/* -- End of module.h */
