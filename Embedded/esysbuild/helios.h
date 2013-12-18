/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- helios.h								--
--                                                                      --
--	General purpose header. Defines basic types and some useful	--
--	macros.								--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: helios.h,v 1.1 1994/01/31 13:53:24 tony Exp $ */

#ifndef __helios_h
#define __helios_h

/* standard type definitions */
typedef  long 		word    ;       /* a machine word, 32 bits      */
#ifndef  WORD
#define  WORD           word            /* a synonym			*/
#endif
#ifndef  INT
#define	 INT		word	 	/* another synonym		*/
#endif

typedef  unsigned long  uword   ;       /* a machine word, 32 bits      */
#ifndef  UWORD
#define  UWORD		uword		/* synonym			*/
#endif

typedef  short 		SHORT	;	/* a 16 bit word 		*/
typedef  unsigned short USHORT	;	/* an unsigned 16 bit value	*/

typedef  char           byte    ;       /* a byte, used in place of char*/
#ifndef  BYTE
#define  BYTE		byte		/* a synonym			*/
#endif

typedef  unsigned char  ubyte   ;       /* an unsigned byte             */
#ifndef  UBYTE
#define  UBYTE		ubyte		/* a synonym			*/
#endif

typedef  word		bool	;	/* boolean value		*/

typedef  char		*string ;	/* character string pointer	*/
#ifndef STRING
#define STRING		string		/* synonym			*/
#endif

typedef  void		(*VoidFnPtr)();	/* pointer to void function	*/
typedef  word		(*WordFnPtr)();	/* pointer to word function	*/

#ifdef __TRAN
typedef  word		Channel	;	/* a transputer channel		*/
#else
typedef  word		Channel	;	/* generic id of h/w channel	*/
#endif

#ifdef in_kernel
typedef	 word		Code	;	/* Function/Result code		*/
#endif

typedef  char 		*APTR   ;       /* a machine pointer            */
typedef  word           RPTR    ;       /* a self relative pointer      */


/* On some systems the machine pointer is not the same as a C pointer.	*/

#ifndef __C40

/* On anything other than a C40, Machine and C pointers are the same.	*/
/* Define all the macros so that code will work on all systems.		*/

typedef	char		*MPtr	;	/* Machine Pointer		*/

#define MWord_(mp,offset)		(*((word *)((mp)+(offset))))
#define SetMWord_(mp,offset,val)	((*((word *)((mp)+(offset)))) = val)
#define MData_(dest,mp,offset,size)	memcpy(dest,(char *)((mp)+(offset)),size)
#define SetMData_(mp,offset,src,size)	memcpy((char *)((mp)+(offset)),src,size)
#define MToFn_(mp)			(mp)
#define MRTOA_(mp)			((MPtr)((mp)+*(word *)(mp)))
#define MInc_(mp,n) 			((MPtr)((mp)+(n)))
#define NullMPtr			NULL
#define MNull_(x)			((x)==NullMPtr)
#define MtoC_(x)			(x)
#define CtoM_(x)			((MPtr) (x))

#else

/* The C40 is a WORD addressed machine. C pointers are BYTE addresses.	*/

typedef	uword		MPtr	;	/* Machine Pointer		*/

#define MWord_(mp,offset) 		MP_GetWord(mp,(offset)/4)
#define SetMWord_(mp,offset,val)	MP_PutWord(mp,(offset)/4,val)
#define MData_(dest,mp,offset,size)	MP_GetData(dest,mp,(offset)/4,(size)/4)
#define SetMData_(mp,offset,src,size)	MP_PutData(mp,(offset)/4,src,(size)/4)
#define MToFn_(mp) 			(mp)
#define MRTOA_(mp) 			((MPtr)(mp+(MWord_(mp,0)/4)))
#define MInc_(mp,n) 			((MPtr)(mp+((n)/4)))
#define NullMPtr			0
#define MNull_(x)			((x)==NullMPtr)

#define MtoC_(x)			C40CAddress(x)
#define CtoM_(x)			C40WordAddress(x)

#endif

/* Get around ANSI spec. disallowing casts of non fn objects to fn pointers */ 

#ifdef __C40
/*
 * XXX - NC - 12/6/92
 *
 * You cannot do this on the C40, as function and data pointers
 * are completely different.  If you must cast between them then
 * use these functions ...
 */

extern VoidFnPtr	_DataToFuncConvert( void * );
extern void *		_FuncToDataConvert( VoidFnPtr );

#endif

typedef union fncast
{
    word	w;
    word *	wp;
    void *	vp;
    char *	cp;
    VoidFnPtr	vfn;
    WordFnPtr	wfn;
    MPtr	mp;
} fncast;


#define PUBLIC				/* an exported symbol		*/
#define PRIVATE		static		/* an unexported symbol		*/
#define FORWARD				/* forward proc reference	*/

/* Syntactic enrichment...						*/

#define forever		for (;;)
#define unless( x )	if (!(x))
#define until(  x )	while (!(x))
#define elif(   x )	else if (x)

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif
#define true		1
#define false		0

#define Variable	1		/* indicates variably sized vectors */
					/* in structure definitions	    */

#define RTOA(x) ((APTR)((word)&(x) + (x)))	/* convert RPTR to APTR */
#define ATOR(x) ((RPTR)((x) - (word)&(x)))	/* convert APTR to RPTR	*/

#define MinInt          0x80000000l
#define MaxInt		0x7fffffffl

#ifdef T800
#define MemStart	(MinInt + 0x70)
#endif
#ifdef T414
#define MemStart	(MinInt + 0x48)
#endif

#ifndef NULL
#define NULL		0
#endif

#define New(_type)	(_type *)Malloc(sizeof(_type))	/* allocate a struct on heap */

#define wordlen(x)	(((x) + 3) & ~3)		/* assumes 32 bit words */

#define Null(_type)	((_type *)NULL)		/* Null pointer to type _type */

#define OneSec		1000000L		/* one second in micro-seconds	   */
#define OneCentisec	10000L			/* one centisecond in micro-seconds */
#define OneMillisec	1000L			/* one millisecond in micro-seconds */

/* Helios system parameters */

#define NameMax		32		/* maximum size of name component */
#define c_dirchar	'/'		/* directory seperator		  */
#define c_matchar	':'		/* matrix row seperator		  */

/* Debugging */

#if defined(SUN4)
void IOdebug();
#else
void		IOdebug( const char *fmt,... );
void		back_trace( void );		/* Prints stack backtrace using IOdebug() */
char *		procname( VoidFnPtr );		/* Prints name of function pointed to or name of parent */ 
#endif

#define print(x) IOdebug( #x " = %x (%d)",x,x);
#define CHECK(x) if(*(word *)(x) != 0 )IOdebug("CHECK %x -> %x",x,*(word *)(x));

#endif

#ifdef __C40
#include <c40.h>	/* because of MtoC and CtoM macros */
#endif

/* -- End of helios.h */

