/************************************************************************/
/* Helios Linker							*/
/*                                                                      */
/* File: link.h                                                         */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/* Copyright (c) 1991, Perihelion Software Ltd. All Rights Reserved.    */
/* Copyright (c) 1992, Perihelion Software Ltd. All Rights Reserved.    */
/* Copyright (c) 1993, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* RcsId: $Id: link.h,v 1.31 1994/01/07 14:28:30 nickc Exp $ */

#include <stdio.h>
#include <setjmp.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <strings.h>
#endif

#ifndef __STDC__
typedef char * VoidStar;
#define void int
#endif

#include <helios.h>

#ifdef __STDC__
# include <queue.h>
#else
# include "queue.nonansi"
#endif

#include "vm.h"

#ifdef __STDC__
extern VMRef VMNew( int size );
#else
extern VMRef VMNew();
#endif

#define locase( c ) (isupper( c ) ? tolower( c ) : c)
#define eqs( a, b ) (strcmp( a, b ) == 0)
#define use( x )    (x) = (x)

/* HASHSIZE must be a prime number */
/* some primes: 19 31 41 53 61 71 83 97 101 151 199  251 307 401 503 */

#define LOCAL_HASHSIZE		31
#define GLOBAL_HASHSIZE		541

#ifdef __C40
# define NEW_STUBS		/* enables new, demand generated calling stubs */
#endif

typedef union
  {
    VMRef	v;
    word   	w;		/* complicated by the fact that orion cannot pass unions as arguments ! */
    long   	fill;
  }
Value;

/* Modules */

typedef struct STEntry
  {
    VMRef	head;
    int		entries;
  }
STEntry;

typedef struct asm_Module
  {
    VMRef  	next;                  	/* link in modlist         */
    VMRef  	start;                 	/* code start		   */
    VMRef  	end;			/* code end		   */
    VMRef  	refs;		      	/* list of references	   */
    WORD   	id;                    	/* module table slot       */
    WORD   	linked;               	/* true if added to link   */
    WORD   	length;		      	/* length of module 	   */
    STEntry   	symtab[ LOCAL_HASHSIZE ];/* module symbol table     */
    char *	file_name;		/* name of the file starting the module */
  }
asm_Module;

#define NEW_REF

typedef struct Symbol
  {
    VMRef	next;		/* next symbol in hash chain	*/
    VMRef	prev;		/* prev symbol in hash chain	*/
    INT		type;           /* symbol type                  */
    unsigned int global:1,	/* true if global, else false	*/
                 referenced:1,	/* true if referenced		*/
                 AOFassumedData:1; /* true if check_symbol() assumes the symbol is data */
#ifdef NEW_REF
    VMRef	fwd_ref;	/* link into forward ref chain	*/
#endif
    VMRef	module;		/* defining module		*/

/* ************** NB different from ASM version, where value is called def */
    Value	value;            /* its definition               */
/* *********************************************************************** */

    char *	file_name;	/* name of file containing definition of symbol */
    
    char	name[ 1 ];      /* symbol (actually variable)   */
  }
Symbol;


typedef struct
  {
    UBYTE	type;      /* code object type     */
    UBYTE	size;      /* size in bytes        */
    UBYTE	vtype;     /* value type           */
    WORD	loc;       /* offset in module     */
    Value	value;     /* value                */
  }
Code;

typedef struct
  {
    byte	type;      /* patch sub op         */
    word	word;      /* word value to patch  */
    Value	value;     /* value                */
  }
Patch;

#define OBJCODE      	0x01
#define OBJBSS        	0x02
#define OBJINIT       	0x03

#define OBJBYTE       	0x09	   	/* ls 3 bits = size      */
#define OBJSHORT      	0x0a
#define OBJWORD       	0x0c

#define OBJCODESYMB   	0x0d		/* was funcref */
#define OBJMODSIZE  	0x0e		/* was OBJIMAGESIZE */
#define OBJLABELREF   	0x0f
#define OBJDATASYMB   	0x10		/* was dataref */
#define OBJDATAMODULE 	0x11
#define OBJMODNUM     	0x12

#define OBJPATCH      	0x13   		/* PATCHES are 0x13 - 0x1f */
#define OBJPATCHMAX   	0x1f

#define OBJMODULE     	0x20
#define OBJBYTESEX	0x21

#define OBJGLOBAL     	0x22
#define OBJLABEL      	0x23
#define OBJDATA       	0x24
#define OBJCOMMON     	0x25
#define OBJCODETABLE  	0x26		/* was OBJCODE or t_func */
#define OBJREF        	0x27		/* force reference to another library */

#ifdef __C40
#define OBJCODESTUB	0x28		/* addr of fn or stub, cf OBJLABELREF */
#define OBJADDRSTUB	0x29		/* address of stub returning address of function */
#endif

#define OBJNEWSEG	0x30
#define OBJEND		0x31
#define OBJLITERAL	0x32	   	/* OBJCODE of <= 4 bytes */

#define S_UNBOUND    	0x40
#define S_CODESYMB   	0x41 		/* label */
#define S_FUNCSYMB   	0x42 		/* smt codesymb */
#define S_DATASYMB   	0x43
#define S_COMMSYMB   	0x44
#define S_DATADONE   	0x45
#define S_FUNCDONE   	0x46
#define S_COMMDONE   	0x47

#define PATCHADD  	0x13 		/* patch = val1 +  val2 */
#define PATCHSHIFT	0x14 		/* patch = val2 << val1 */ /* right shift for -ve val1 */
#define PATCHSWAP	0x1E		/* pathc = byte_swap( val1 ) */
#define PATCHOR		0x1f		/* patch = val1 | val2 */

#ifdef __ARM
#define PATCHARMDT	0x15
#define PATCHARMDP	0x16
#define PATCHARMJP	0x17
#define PATCHARMDPLSB	0x18
#define PATCHARMDPREST	0x19
#define PATCHARMDPMID	0x1a
#define PATCHARMAOFLSB	0x1b		/* patch required by AOF converter.  See mcpatch() */
#define PATCHARMAOFMID	0x1c		/* patch required by AOF converter.  See mcpatch() */
#define PATCHARMAOFREST	0x1d		/* patch required by AOF converter.  See mcpatch() */
#endif

#ifdef __C40
#define PATCHC40DATAMODULE1	0x15
#define PATCHC40DATAMODULE2	0x16
#define PATCHC40DATAMODULE3	0x17
#define PATCHC40DATAMODULE4	0x18
#define PATCHC40DATAMODULE5	0x19
#define PATCHC40MASK24ADD	0x1a
#define PATCHC40MASK16ADD	0x1b
#define PATCHC40MASK8ADD	0x1c
#endif

#define i_helios   	'h'

/* traceing */

extern word traceflags;

#define db_gencode   	0x0008
#define db_genimage  	0x0010
#define db_files     	0x0400
#define db_mem       	0x0800
#define db_sym       	0x1000
#define db_modules   	0x2000
#define db_scancode  	0x4000
#ifdef NEW_STUBS
#define db_stubs	0x8000
#endif
#if defined __ARM && (defined RS6000 || defined __SUN4)
#define db_aof	       0x10000
#endif

/* externals */

extern word 	vmpagesize;
extern word 	codesize;
extern word 	heapsize;
extern word 	symsize;
extern VMRef 	curmod;
extern VMRef 	module0;
extern FILE *	infd;
extern int  	outf;
extern word 	smtopt;
extern word	gen_image_header;
extern VMRef 	firstmodule;
extern VMRef 	tailmodule;
extern word 	inlib;
extern word 	curloc;
extern word 	totalcodesize;
extern WORD 	codepos;
extern BYTE *	infile_duplicate;
#if defined __ARM && (defined RS6000 || defined __SUN4)
extern word	bSharedLib;
extern word	bDeviceDriver;
extern word	bTinyModel;
#endif

#ifdef __STDC__
extern void *	alloc( INT n );
#else
extern VoidStar	alloc();
#endif

#ifdef __STDC__
extern void 	error(  BYTE *, ... );
extern void 	report( BYTE *, ... );
extern void 	warn(   BYTE *, ... );
extern void 	inform( BYTE *, ... );
extern void 	initmem( void );
extern void 	initcode( void );
extern void 	initmodules( void );
extern void 	setmodules( void );
extern void 	initsym( void );
extern void 	readfile( void );
extern void 	genend( void );
extern void 	genimage( void );
extern void 	objed( int, char *, long, long );
extern VMRef 	codeptr( void );
extern void 	_trace( char *, ... );
extern void 	cps( char *, char * );
extern VMRef 	lookup( char * );
extern VMRef 	insert( byte *, word );
extern void 	movesym( VMRef );
extern void 	refsymbol( VMRef, word );
extern VMRef 	newcode( WORD, WORD, WORD, WORD, WORD );
extern void 	scancode( void );
extern void 	genimage( void );
extern void	endmodule(void);
extern void	refsymbol_nondef(VMRef v);
extern void	refsymbol_def(VMRef v);
extern void	outword( WORD );
#ifdef NEW_STUBS
extern WORD	new_stub( VMRef, bool );
extern void	build_stubs( void );
extern void	flush_output_buffer( void );
extern void	RequestBranchStub( VMRef );
extern WORD	FindSymbolOffset( VMRef );
#endif
#else
extern void 	error();
extern void 	report();
extern void 	warn();
extern void 	initmem();
extern void 	initcode();
extern void 	initmodules();
extern void 	setmodules();
extern void 	initsym();
extern void 	readfile();
extern void 	genend();
extern void 	genimage();
extern void 	objed();
extern VMRef 	codeptr();
extern void 	_trace();
extern void 	cps();
extern VMRef 	lookup();
extern VMRef 	insert();
extern void	movesym();
extern void 	refsymbol();
extern VMRef 	newcode();
extern void 	scancode();
extern void 	genimage();
extern void	endmodule();
extern void	refsymbol_nondef();
extern void	refsymbol_def();
extern void	outword();
extern WORD	new_stub();
extern void	build_stubs();
extern void	flush_output_buffer();
#endif
