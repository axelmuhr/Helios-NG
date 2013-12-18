/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   C   L I B R A R Y			--
--                     -------------------------------                  --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- nonansi.h								--
--                                                                      --
--	Helios extensions to the ANSI library				--
--                                                                      --
--	Author:  BLV 9/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: nonansi.h,v 1.11 1993/07/27 13:59:18 paul Exp $ */

#ifndef __nonansi_h
#define __nonansi_h

#ifndef __syslib_h
#include <syslib.h>
#endif

#ifndef __memory_h
#include <memory.h>
#endif

#ifndef __stdio_h
#include <stdio.h>
#endif

void *	NewProcess(  WORD stacksize, VoidFnPtr function, WORD argsize );
void	ExecProcess( void * process, word logpri );
void	RunProcess(  void * process );
void	ZapProcess(  void * process );
word	Fork(        WORD stacksize, VoidFnPtr function, WORD argsize, ... );
word	_cputime(    void );
word	_ldtimer(    word pri );	/* returns current system clock in microseconds */
void	StackCheck(  void );
#if defined(SUN4)
void    IOdebug();
#else
void	IOdebug(     const char * fmt, ... );
#endif
void	IOputc(      char c );
void	IOputs(      char * s );

#ifndef __TRAN
word	CallWithModTab( word arg1, word arg2, WordFnPtr fn, word * mpdtab );
word	*_GetModTab( void );  /* DO NOT USE THESE FUNCTIONS ! */
#endif

#if defined(__TRAN) || defined(__C40) || defined(__ABC)
word	AccelerateCode( VoidFnPtr function );
word    Accelerate( Carrier * fastram, VoidFnPtr function, WORD argsize, ... );
#endif
#ifdef __ABC
word    SpeedUpCode( VoidFnPtr function );
#endif

#ifdef __TRAN
void	bytblt(	char * source,	char * dest, 
		word   sindex,	word   dindex, 
		word   sstride,	word   dstride,
		word   length,	word   width,
		word   op );
#define BYTBLT_ALL	0
#define BYTBLT_NONZERO	1
#define BYTBLT_ZERO	2
#endif

extern Stream *	       fdstream( int );		/* POSIX fd -> Helios stream convert */

#ifndef fileno
extern int	       fileno( FILE * );	/* Clib stream -> POSIX fd convert   */
#endif

extern FILE *	       fdopen( int, string );	/* POSIX fd -> Clib FILE * 	     */

/* This macro is for compatability */
#define Heliosno( s )  fdstream( fileno( s ) )

#endif /* __nonansi_h */
