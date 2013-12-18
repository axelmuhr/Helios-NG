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
/* SccsId: @(#)helios.h	1.6	25/11/88 Copyright (C) 1987, Perihelion Software Ltd.	*/

#ifndef __helios_h
#define __helios_h

/* standard type definitions */

typedef  long 		word    ;       /* a machine word, 32 bits      */
#define  WORD           word            /* a synonym			*/
#define	 INT		word	 	/* another synonym		*/

typedef  unsigned long  uword   ;       /* a machine word, 32 bits      */
#define  UWORD		uword		/* synonym			*/

typedef  short 		SHORT	;	/* a 16 bit word 		*/
typedef  unsigned short USHORT	;	/* an unsigned 16 bit value	*/

typedef  char           byte    ;       /* a byte, used in place of char*/
#define  BYTE		byte		/* a synonym			*/

typedef  unsigned char  ubyte   ;       /* an unsigned byte             */
#define  UBYTE		ubyte		/* a synonym			*/

typedef  word		bool	;	/* boolean value		*/

typedef  char		*string ;	/* character string pointer	*/
#define STRING		string		/* synonym			*/

typedef  void		(*VoidFnPtr)();	/* pointer to void function	*/
typedef  word		(*WordFnPtr)();	/* pointer to word function	*/

typedef  word		Channel	;	/* a transputer channel		*/

#ifdef in_kernel
typedef	 word		Code	;	/* Function/Result code		*/
#endif

typedef  char 		*APTR   ;       /* a machine pointer            */
typedef  word           RPTR    ;       /* a self relative pointer      */

#define PUBLIC				/* an exported symbol		*/
#define PRIVATE		static		/* an unexported symbol		*/
#define FORWARD				/* forward proc reference	*/

/* Syntactic enrichment...						*/

#define forever		for(;;)
#define unless(x)	if(!(x))
#define until(x)	while(!(x))
#define elif(x)		else if(x)

#ifndef TRUE
#define TRUE		1l
#define FALSE		0l
#endif
#define true		1l
#define false		0l

#define Variable	1		/* indicates variably sized vectors */
					/* in structure definitions	    */

#define RTOA(x) ((APTR)((word)&(x)+(x)))	/* convert RPTR to APTR */
#define ATOR(x) ((RPTR)((x)-(word)&(x)))	/* convert APTR to RPTR	*/

#define MinInt          0x80000000l
#define MaxInt		0x7fffffffl

#ifdef T800
#define MemStart	(MinInt+0x70)
#endif
#ifdef T414
#define MemStart	(MinInt+0x48)
#endif

#ifndef NULL
#define NULL		0
#endif

#define New(_type)	(_type *)Malloc(sizeof(_type)) /* allocate a struct on heap */

#define wordlen(x)	(((x)+3) & ~3)		/* assumes 32 bit words */

#define Null(_type)	((_type *)NULL)		/* Null pointer to type _type */

#define OneSec		1000000l		/* one second in micro-seconds	   */

/* Helios system parameters */

#define NameMax		32		/* maximum size of name component */
#define c_dirchar	'/'		/* directory seperator		  */
#define c_matchar	':'		/* matrix row seperator		  */

/* Debugging */

#ifdef ORION
void IOdebug();
#else
void IOdebug(const char *fmt,...);
#endif

#define print(x) IOdebug( #x " = %x (%d)",x,x);
#define CHECK(x) if(*(word *)(x) != 0 )IOdebug("CHECK %x -> %x",x,*(word *)(x));

#endif

/* -- End of helios.h */
