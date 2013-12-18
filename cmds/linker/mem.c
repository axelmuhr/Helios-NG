/****************************************************************/
/* Helios Linker                                                */
/*								*/
/* File: mem.c                                                  */
/*                                                              */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
/* RcsId: $Id: mem.c,v 1.4 1992/10/01 10:03:26 nickc Exp $ */

#include "link.h"

#define trace if(traceflags&db_mem)_trace

PUBLIC  WORD  heapsize;      /* amount of heap memory used   */

PRIVATE VMRef vmheap;      /* virtual heap         */

PRIVATE VMRef codeseg;      /* start of current segment   */

PUBLIC WORD codesize;      /* amount of code memory used   */

#define HEAPDELTA   20000

#ifndef __STDC__
VoidStar malloc();
#endif

#define MALLOC(n) (UBYTE *)malloc(n)


/********************************************************/
/* initmem                                              */
/*                                                      */
/* initialise memory system                             */
/********************************************************/

PUBLIC void initmem()
{
   (void)VMInit(NULL,(int)vmpagesize,1000);
   
   codeseg = VMPage();
   vmheap = VMPage();
   codesize = 0;
}

/********************************************************/
/* alloc                                                */
/*                                                      */
/* allocate n bytes from the heap                       */
/********************************************************/

PUBLIC void *
alloc( INT n )
{
  UBYTE *v;

  v = MALLOC((int)n);

  if( v == NULL ) error("Cannot get local heap space");
  
  heapsize += n;
   
  return v;
}

/********************************************************/
/* newcode                                              */
/*                                                      */
/* allocates and initialise a new code structure        */
/*                                                      */
/********************************************************/

extern WORD lineno;

PUBLIC VMRef
newcode(
	WORD type,
	WORD size,
	WORD vtype,
	WORD loc,
	WORD value )
{
   Code *p;
   VMRef v;
   
   (void)codeptr();   /* ensure there is space */
   
   v = VMalloc(sizeof(Code),codeseg);
   
   p = VMAddr(Code,v);
      
        trace("newcode: %8x : %3ld %3ld %3ld %8lx",p,type,size,vtype,value);
        p->type = (byte)type;
        p->size = (byte)size;
        p->vtype = (byte)vtype;
        p->loc = loc;
        p->value.fill = value;
   codesize += sizeof(Code);
   
        VMDirty(v);

   return v;
}

PUBLIC VMRef codeptr()
{   
   /* if this would be the last code entry in the segment, get a   */
   /* new block and add a OBJNEWSEG entry.            */

   if( VMleft(codeseg) < sizeof(Code)*2 )
   {
      VMRef newseg = VMPage();
      VMRef v = VMalloc(sizeof(Code),codeseg);
      Code *codetop = VMAddr(Code,v);
                
                if( NullRef(newseg) ) error("Cannot get code segement");

      codetop->type = OBJNEWSEG;
      codetop->size = 0;
      codetop->vtype = 0;
      codetop->value.v = newseg;
      codetop->loc = -1;

      codesize += sizeof(Code);
      
      trace("code extension: %lx",codeseg);
   
           VMDirty(v);
      
      codeseg = newseg;
   }

   return VMnext(codeseg);
}

extern VMRef
VMNew( int size )
{
   VMRef v;

   size = wordlen(size);

   v = VMalloc(size,vmheap);
   
   if( NullRef(v) ) 
   {
      vmheap = VMPage();
      v = VMalloc(size,vmheap);
   }

        VMDirty(v);

   return v;
}
