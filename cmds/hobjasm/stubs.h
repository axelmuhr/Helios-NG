/*> stubs/h <*/
/*---------------------------------------------------------------------------*/
/* Provide code to handle external function call stubs			     */
/*---------------------------------------------------------------------------*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.	     */
/*---------------------------------------------------------------------------*/

#ifndef stubs_h
#define stubs_h

/*---------------------------------------------------------------------------*/

typedef struct StubBlock *StubBlockPointer ;

typedef struct StubBlock {
			  Name             symbol ;  /* label STUB accesses */
			  BOOLEAN	   except ;  /* exception STUB */
			  StubBlockPointer next ;    /* next active STUB */
                         } StubBlock ;

/*---------------------------------------------------------------------------*/

void StubFileEnd(void) ;		/* empty the STUB chain */
void AddStub(Name name,BOOLEAN type) ;	/* add a STUB for this symbol */
void InitStubs(void) ;			/* initialise the STUB chain */

/*---------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------------*/
/*> EOF stubs/h <*/
