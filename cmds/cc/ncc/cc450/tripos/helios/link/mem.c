/****************************************************************/
/* File: mem.c                                                  */
/*                                                              */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
static char *SccsId = "%W%   %G% Copyright (C) Perihelion Software Ltd.";

#include "link.h"

#define trace if(traceflags&db_mem)_trace

PUBLIC  WORD  heapsize;      /* amount of heap memory used   */

PRIVATE VMRef vmheap;      /* virtual heap         */

PRIVATE VMRef codeseg;      /* start of current segment   */

PUBLIC WORD codesize;      /* amount of code memory used   */

#define HEAPDELTA   20000

#ifdef MWC
UBYTE *lmalloc();
#define MALLOC(n) (UBYTE *)lmalloc(n)
#else
#ifdef IBMPC
UBYTE *getml();
#define MALLOC(n) (UBYTE *)getml(n)
#else
void *malloc(int n);
#define MALLOC(n) (UBYTE *)malloc(n)
#endif
#endif

/********************************************************/
/* initmem                                              */
/*                                                      */
/* initialise memory system                             */
/********************************************************/

PUBLIC void initmem()
{
   VMInit(NULL,vmpagesize,1000);
   
   codeseg = VMPage();
   vmheap = VMPage();
   codesize = 0;

}

/********************************************************/
/* alloc                                                */
/*                                                      */
/* allocate n bytes from the heap                       */
/********************************************************/

PUBLIC void *alloc(n)
INT n;
{
        UBYTE *v;

   v = MALLOC(n);

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

PUBLIC VMRef newcode(type,size,vtype,loc,value)
WORD type;
WORD size,vtype;
Value value;
WORD loc;
{
   Code *p;
   VMRef v;
   
   codeptr();   /* ensure there is space */
   
   v = VMalloc(sizeof(Code),codeseg);
   
   p = VMAddr(Code,v);
      
        trace("newcode: %8x : %3ld %3ld %3ld %8lx",p,type,size,vtype,value);
        p->type = type;
        p->size = size;
        p->vtype = vtype;
        p->loc = loc;
        p->value = value;
   codesize += sizeof(Code);
   
        VMDirty(v);

   return v;
}

PUBLIC VMRef codeptr()
{   
   /* if this would be the last code entry in the segment, get a   */
   /* new block and add a t_newseg entry.            */

   if( VMleft(codeseg) < sizeof(Code)*2 )
   {
      VMRef newseg = VMPage();
      VMRef v = VMalloc(sizeof(Code),codeseg);
      Code *codetop = VMAddr(Code,v);
                
                if( NullRef(newseg) ) error("Cannot get code segement");

      codetop->type = t_newseg;
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

extern VMRef VMNew(int size)
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
