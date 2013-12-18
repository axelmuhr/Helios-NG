/*
    CATSUTIL.C Useful debug and other routines for Helios v1.3x
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
³                  MODULE: CATSUTIL                                          ³
³                                                                            ³
³                 PURPOSE: Various useful subroutines that don't belong      ³
³                          anywhere else.                                    ³
³                                                                            ³
³    MODIFICATION HISTORY:                                                   ³
³                                                                            ³
³    Version        Date       Author    Comments                            ³
³    -------     -----------   ------    --------                            ³
³      1.0      13-Mar-1993 K.Blackler  Original Issue                       ³
³      1.1      23-Aug-1993 K.Blackler  Fixed DebugCalloc to zero memory     ³
³     **** Second public release version 22/11/1993 ****                     ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄkb@jet.ukÄÄÄÄÙ */

#define __CATSUTIL__

#include <stdio.h>
#include <string.h>
#include "catsutil.h"

void FatalError(const char *Message,const char *FileName, int LineNo)
{
  ModuleLineMessage(FileName,LineNo,Message);
#if defined(_WINDOWS) && defined(_DEBUG) /* Yes I even use this stuff in Windows 3.1!!! */
  _asm { int 3 }; /* Cause a debug break */
#endif      
  exit(4);
}

#ifdef _DEBUG

void AssertFailedLine(const char *FileName, int LineNo)
{
  ModuleLineMessage(FileName,LineNo,"ASSERT Failed");
#if defined(_WINDOWS)  
  _asm { int 3 };
  _exit(3);
#endif      
  exit(3);
}

static void MemoryAllocationErrorLine(const char *FileName, int LineNo, const char *Message)
{
  ModuleLineMessage(FileName,LineNo,Message);
#if defined(_WINDOWS)  
  _asm { int 3 };
#endif      
  exit(3);
}

#ifdef _TRACE_ALLOCS
static void MemoryAllocationTrace(const char *FileName, int LineNo,int nmalloc)
{
  char Buffer[64];
  
  sprintf(Buffer,"Memory allocation Number: %d",nmalloc);
  ModuleLineMessage(FileName,LineNo,Buffer);
}
#else

#define MemoryAllocationTrace(x,y,z) ((void)0)

#endif

static int nAllocs;

#define PAD_BEFORE 0x42454721 /* These are meant to be uncommon...... */
#define PAD_LENGTH 0x53495a45
#define PAD_AFTER  0x454e4421

void DebugCheckHeap(void)
{
  if (nAllocs!=0)
    MemoryAllocationErrorLine("DURING EXIT",-1,"Memory allocation error, not all memory cleaned up.");
}

void *DebugMalloc(size_t size,const char *pFile, int nLine)
{
  size_t nWords=size / sizeof(WORD32) + 1 + 4; /* All we're interested in is 32-bit words */
  WORD32 *pMemBlock=(WORD32 *)malloc(nWords*sizeof(WORD32));
  
  if (pMemBlock!=NULL)
    {
      *(pMemBlock)         = PAD_BEFORE;
      *(pMemBlock+1)       = nWords;
      *(pMemBlock+2)       = PAD_LENGTH;
      *(pMemBlock+nWords-1)= PAD_AFTER;
      if (nAllocs==0) /* This is the first alloc */
        {
          atexit(DebugCheckHeap);
        }
      nAllocs++;
      MemoryAllocationTrace(pFile,nLine,nAllocs);
    }
  else
    {
      MemoryAllocationErrorLine(pFile,nLine,"Memory allocation error, out of memory?");
    }
  return (void *)(pMemBlock+3);
}
 
void *DebugCalloc( size_t num, size_t size,const char *pFile, int nLine)
{ 
  void *pMem=DebugMalloc(num*size,pFile,nLine);
  memset(pMem,0,num*size);
  
  return(pMem);
}

void DebugFree(void *pDataMemBlock,const char *pFile, int nLine)
{
  WORD32 *pMemBlock=((WORD32 *)pDataMemBlock)-3;
  size_t nWords=(size_t)*(pMemBlock+1);

  if (*pMemBlock!=PAD_BEFORE)
    {               
      MemoryAllocationErrorLine(pFile,nLine,"Memory deallocation error, corrupted before memory block.");
    }

  if (*(pMemBlock+2)!=PAD_LENGTH) 
    {               
      MemoryAllocationErrorLine(pFile,nLine,"Memory deallocation error, corrupted data before memory block.");
    }

  if (*(pMemBlock+nWords-1)!=PAD_AFTER) 
    {               
      MemoryAllocationErrorLine(pFile,nLine,"Memory deallocation error, corrupted after memory block.");
    }
  
  *pMemBlock=*(pMemBlock+2)=*(pMemBlock+nWords-1)=0; /* So that it doesn't look correct any more */
  
  MemoryAllocationTrace(pFile,nLine,nAllocs);
  if (nAllocs--<=0)
    {
      MemoryAllocationErrorLine(pFile,nLine,"Memory deallocation error, more deallocations than allocations.");
    }
  free(pMemBlock);
} 

void *DebugRealloc(void *pMemory,size_t size,const char *pFile, int nLine)
{
  void *pNewMemory=DebugMalloc(size,pFile, nLine);
  
  memcpy(pNewMemory,pMemory,size); /* size may well be overkill for a grow but its simple... */
  
  DebugFree(pMemory,pFile, nLine);
  
  return pNewMemory;
}

#endif

void ModuleLineMessage(const char*ModuleName, int nLine, const char *Message)
{
  printf("%s : Line %d : %s\n",ModuleName,nLine,Message);
}

void ModuleMessage(const char*ModuleName, const char *Message)
{
  printf("%s : %s\n",ModuleName,Message);
}

void MyMemoryDump(void *SourceAddress,int nSize)
{
  int i,j;
  char *pChars=(char *)SourceAddress;

  for (i=0; i<nSize; i+=20)
    {
#if defined(__C40)    
      printf("%08lX ",((WORD32)SourceAddress+i) >> 2 );
#else
      printf("%08lX ",(WORD32)SourceAddress+i);
#endif      
      for (j=i; j<i+20; j++)
        {
	        if (j>nSize)
	          {
	            printf(".. ");
	          }
	        else
	          {
	            printf("%02x ",((int)pChars[j])&0xff);
	          }
        }
      putchar('\"');  
      for (j=i; j<i+20; j++)
        {
	        if ( j>nSize || pChars[j]<' ' )
	          {
	            putchar('.');
	          }
	        else
	          {
	            putchar(pChars[j]);
	          }
        }
      putchar('\"'); putchar('\n');
    }
}

void MyMemoryFill(char cBase,char *pData,int nSize)
{
  int i;
  for (i=0; i<nSize; i++)
    {
      pData[i]=cBase+(i&7);
    }
}
