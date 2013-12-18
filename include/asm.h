/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   K E R N E L                        --
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- asm.h								--
--                                                                      --
--	This header defines various macros which allow users to		--
--	add in-line assembler to their C programs.			--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: asm.h,v 1.3 1992/09/14 11:05:57 paul Exp $ */

#ifndef __asm_h
#define __asm_h

#ifndef __TRAN
# error including Transputer specific header file
#else

#ifndef __helios_h
#include <helios.h>
#endif

extern int _operate(int op, ... );
extern int _direct( int fn, ... );
extern void _setpri( int pri );

#define ldlp_(local)		_direct(0x01,local)
#define ldc_(value)		_direct(0x04,value)
#define ldl_(local)		_direct(0x07,local)
#define ajw_(words)		_direct(0x0b,words)
#define stl_(local,value)	_direct(0x0d,local,value)

#define rev_(expr)		_operate(0x00,expr)
#define diff_(a,b)		_operate(0x04,a,b)
#define gcall_(fn)		_operate(0x06,fn)
#define in_(size,chan,buf)	_operate(0x07,size,chan,buf)
#define gt_(a,b)		_operate(0x09,a,b)	
#define out_(size,chan,buf)	_operate(0x0b,size,chan,buf)
#define resetch_(chan)		_operate(0x12,chan)
#define stopp_()                _operate(0x15)
#define stlb_(a)		_operate(0x17,a)
#define sthf_(a)		_operate(0x18,a)
#define stlf_(a)		_operate(0x1c,a)
#define ldpri_()		_operate(0x1e)
#define ret_()			_operate(0x20)
#define ldtimer_()		_operate(0x22)
#define testerr_()		_operate(0x29)
#define tin_(time)		_operate(0x2b,time)
#define runp_(process)		_operate(0x39,process)
#define sb_(p,v)		_operate(0x3b,p,v)
#define gajw_(wsp)		_operate(0x3c,wsp)
#define savel_(dest)		_operate(0x3d,dest)
#define saveh_(dest)		_operate(0x3e,dest)
#define move_(size,dest,source)	_operate(0x4a,size,dest,source)
#define sthb_(a)		_operate(0x50,a)
#define sum_(a,b)		_operate(0x52,a,b)
#define clrhalterr_()		_operate(0x57)
#define sethalterr_()		_operate(0x58)
#define testhalterr_()		_operate(0x59)
#define ldinf_()		_operate(0x71)
#define bitcnt_(a)		_operate(0x76,a)

#define start_()		_operate(0x1ff)
#define testhardchan_(chan,val)	_operate(0x2d,chan,val)
#define lddevid_(a,b,c)		_operate(0x17c,a,b,c)

/* for fpu... ops the code is 0x100+entry point */

#define fpstnlsn_(a)		_operate(0x88,a)
#define fpldnlsn_(a)		_operate(0x8e,a)
#define fpstnldb_(a)		_operate(0x84,a)
#define fpldnldb_(a)		_operate(0x8a,a)

#define fpusqrtfirstsn_(a)	(fpldnlsn_(&a),_operate(0x101))
#define fpusqrtfirstdb_(a)	_operate(0x101,a)
#define fpusqrtstep_()		_operate(0x102)
#define fpusqrtlastsn_(a)	(_operate(0x103),fpstnlsn_(&a))
#define fpusqrtlastdb_(a)	(_operate(0x103),fpstnldb_(&a))

#endif
#endif

/* -- End of asm.h */
