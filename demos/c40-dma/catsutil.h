/*
    CATSUTIL.H Useful debug and other routines for Helios v1.3x
    Copyright (C) 1993  Ken Blackler, JET Joint Undertaking

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    
    The author can be contacted by EMail as: kb@jet.uk
    or at:
    
    		Ken Blackler
    		JET Joint Undertaking
    		Abingdon
    		Oxfordshire
    		England
    		OX14 3EA
    
*/
                                                                              /*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄJET Joint UndertakingÄÄÄÄÄÄÄÄÄÄÄÄ¿
³                                                                            ³
³            INCLUDE FILE: CATSUTIL                                          ³
³                                                                            ³
³                 PURPOSE: Header defining utility functions                 ³
³                                                                            ³
³    MODIFICATION HISTORY:                                                   ³
³                                                                            ³
³    Version        Date       Author    Comments                            ³
³    -------     -----------   ------    --------                            ³
³      1.0      13-Mar-1993 K.Blackler  Original Issue                       ³
³      1.01     24-Aug-1993 K.Blackler  Sorted out includes                  ³
³     **** Second public release version 22/11/1993 ****                     ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄkb@jet.ukÄÄÄÄÄÄÙ
                                                                              */
                                                                              
#if !defined(__CATSUTIL_H__)
#define __CATSUTIL_H__            



/* Some notes: 

   Expressions in an ASSERT(exp) statement are only evaluated when _DEBUG is defined.
   They are NOT evaluated in release code. The ASSERT statement becomes void. Use to
   test valid values for expressions or variables while debugging.
   
   Expressions in a VERIFY(exp) statement are always evaluated, whether _DEBUG is defined
   or not. In a debug compilation it works like ASSERT, in a release version VERIFY just
   evaluates the expression without testing. Use VERIFY to test the result of a function
   return value if you want it to still be called in the release code!
*/   

/* First some useful defines and types */
typedef unsigned short WORD16;       /* 16 bits */
typedef unsigned long  WORD32;       /* 32 bits */
typedef int BOOL;

#define BIN_00  0
#define BIN_01  1
#define BIN_10  2
#define BIN_11  3
#define BIN_111 7


                                                                              
#if !defined(__HELIOS)
/* I can use this stuff for the PC as well*/

#define __FAR__ __far

typedef unsigned long WORD;

#define FALSE 0
#define TRUE  1

#define NULL ((void *)0)

#else /* is __HELIOS*/

#define __FAR__
#pragma -s0  /* Enable stack checking */

#endif  /*__HELIOS*/

#if !defined(__min) /* These have side effects if 'a' or 'b' are function calls etc. */
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#if defined(_DEBUG)
#define DEBUG_DECLARE(x) x;
#else
#define DEBUG_DECLARE(x)
#endif  

#define UPTOEVEN(x) (x+(x&&1))

void ModuleMessage(const char*ModuleName, const char *Message);
void MyMemoryDump(void *SourceAddress,int nChannels);
void MyMemoryFill(char cBase,char *pData,int nSize);

void ModuleMessage(const char*ModuleName, const char *Message);
void ModuleLineMessage(const char*ModuleName, int nLine, const char *Message);


void FatalError(const char *Message,const char *FileName, int LineNo);

#define FATALERROR(x) FatalError(x,__FILE__,__LINE__)


#ifdef _DEBUG

void AssertFailedLine(const char *FileName, int LineNo);

#define ASSERT(f)          ((f) ? (void)0 : \
                                                                AssertFailedLine(__FILE__, __LINE__))
#define VERIFY(f)          ASSERT(f)

#include <stdlib.h>
#ifndef __CATSUTIL__
#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(x)  DebugMalloc(x,__FILE__,__LINE__)
#define calloc(x,y)  DebugCalloc(x,y,__FILE__,__LINE__)
#define realloc(x,y) DebugRealloc(x,y,__FILE__,__LINE__)
#define free(x)    DebugFree(x,__FILE__,__LINE__)

void *DebugMalloc(size_t size,const char *pFile, int nLine);
void *DebugRealloc(void *pMemory,size_t size,const char *pFile, int nLine);
void *DebugCalloc( size_t num, size_t size,const char *pFile, int nLine);
void DebugFree(void *pMemBlock,const char *pFile, int nLine);

#endif

#else

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#include <stdlib.h>

#endif

#endif /* __CATSUTIL_H__ */
