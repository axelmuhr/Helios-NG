/*> stubs/c <*/
/*---------------------------------------------------------------------------*/
/* Provide code to handle external function call stubs			     */
/*---------------------------------------------------------------------------*/
/* (c) 1990, Active Book Company, Cambridge, United Kingdom.		     */
/*---------------------------------------------------------------------------*/

#include "code.h"
#include "globvars.h"
#include "literals.h"
#include "asmvars.h"
#include "store.h"
#include "stubs.h"
#include "errors.h"
#include "store.h"
#include "occur.h"
#include "extypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

StubBlockPointer stubBlockPointer = NULL ;
StubBlockPointer currentStubBlockPointer = NULL ;

/*---------------------------------------------------------------------------*/

static BOOLEAN DefineStubLabel(Name name)
{
 SymbolPointer  symbolPointer ;
 char          *label = mymalloc(name.length + 2) ;
 Name           nname ;

 /* We must precede all "stub" symbols with "." */
 *label = '.' ;
 memcpy((label + 1),name.key,name.length) ;
 *(label + 1 + name.length) = '\0' ;
 nname.key = label ;
 nname.length = strlen(label) ;

 symbolPointer = LookupFixed(nname,FALSE) ;

 if ((symbolPointer == NULL) || ((symbolPointer)->u.s.sds != UndefinedSDS))
  {
   free(label) ;
   Warning(MulDefSym) ;
   return(TRUE) ;
  } ; /* if */

 symbolPointer->value.card = programCounter ;	/* remember where we are */
 
 AddDef(symbolPointer) ;
 free(label) ;
 return(FALSE) ;
}

/*---------------------------------------------------------------------------*/

void StubFileEnd(void)
{
 StubBlockPointer stub = stubBlockPointer ;	/* head of the pending STUBs */
 Name		  name ;
 char	          ntext[MaxStringSize] ;
 
 name.key = ntext ;

 for (; (stub != NULL);)
  {
   switch (pass)
    {
     case 1 : /* define the symbols, and account for the space */
 	      while ((programCounter % 4) != 0)
   	       programCounter++ ;		/* word-align stub code */

	      /* generate the symbol */
	      (void)DefineStubLabel(stub->symbol) ;
	      if (stub->except)
	       programCounter += 20 ;	/* 1-push, 2-loads, 1-store, 1-pop */
	      else
               programCounter += 12 ;	/* 2-loads and 1-mov */

	      /* remove this STUB from the chain */
	      currentStubBlockPointer = stub ;  /* remember current */
	      stub = stub->next ;		  /* get ready for next */
	      free(currentStubBlockPointer->symbol.key) ;
	      free(currentStubBlockPointer) ;
   	      break ;

     case 2 : /* define the symbols, and generate the code */
	      while ((programCounter % 4) != 0)
	       CodeByte(0) ;

	      /* generate the symbol */
	      write_dotlabel(HOF_t_label,stub->symbol.key,stub->symbol.length) ;

	      ntext[0] = 128 ;
	      memcpy(&ntext[1],stub->symbol.key,stub->symbol.length) ;
	      ntext[1 + stub->symbol.length] = '\0' ;
	      name.length = strlen(ntext) ;
	      
	      /* output the code (and patches) */

	      if (stub->except)
	       CodeWord(stubPush) ;	/* "STMFD sp!,{ip,lk}" */

              /* "LDR <r>,[dp,#modnum_offset_of_label]" */
              write_patch(HOF_t_patch_armdt,LDRfromdp(12)) ; /* "ip" reg */
              write_label(HOF_t_datamod,name.key,name.length) ;
	      /* "LDR <r>,[<r>,#offset]" */
       	      write_patch(HOF_t_patch_armdt,LDRfromoffset(12)) ;

	      /* we should only ever be referencing functions from STUBs */
	      write_label(HOF_t_codesymbol,name.key,name.length) ;

	      if (stub->except)
	       {
		CodeWord(stubStore) ;	/* "STR   ip,[sp,#4]"  */
		CodeWord(stubPop) ;	/* "LDMFD sp!,{ip,pc}" */
	       }
	      else
 	       CodeWord(stubCall) ;	/* "MOV pc,ip" */

	      /* remove this STUB from the chain */
	      currentStubBlockPointer = stub ;  /* remember current */
	      stub = stub->next ;		  /* get ready for next */
	      free(currentStubBlockPointer->symbol.key) ;
	      free(currentStubBlockPointer) ;
   	      break ;
    }
  }

 CodeEnd() ;
 /* we should have an empty list again */
 currentStubBlockPointer = stubBlockPointer = NULL ;
 return ;
}

/*---------------------------------------------------------------------------*/
/* Add a STUB definition to the end of the current chain */
void AddStub(Name name,BOOLEAN exceptionFLAG)
{
 StubBlockPointer newStub ;

 newStub = (StubBlockPointer)mymalloc(sizeof(StubBlock)) ;

 newStub->symbol.key = (char *)mymalloc(name.length + 1) ;
 newStub->symbol.length = name.length ;
 memcpy(newStub->symbol.key,name.key,name.length) ;
 newStub->symbol.key[name.length] = '\0' ;
 newStub->except = exceptionFLAG ;

 newStub->next = NULL ;
 if (currentStubBlockPointer == NULL)
  stubBlockPointer = newStub ;
 else
  currentStubBlockPointer->next = newStub ;

 currentStubBlockPointer = newStub ; 
 return ;
}

/*---------------------------------------------------------------------------*/
/* Initialise the STUBs chain */
void InitStubs(void)
{
 /* This should be called at the start of each assembly pass */
 stubBlockPointer = NULL ;
 currentStubBlockPointer = NULL ;
 return ;
}

/*---------------------------------------------------------------------------*/
/*> EOF stubs/c <*/
