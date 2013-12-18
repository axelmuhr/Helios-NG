/*------------------------------------------------------------------------
--                                                                      --
--                      C Coroutine library for RS6000			--
--                      ------------------------------                  --
--                                                                      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
--      colib.c                                                         --
------------------------------------------------------------------------*/
/* RcsId: $Id: colib.c,v 1.1 1993/03/01 10:47:17 nick Exp $ */
/* Copyright (C) 1993, Perihelion Software Ltd.       			*/

#include "../helios.h"

/**
***
*** This library uses two context manipulation routines to do all the
*** processor specific stuff:
***
*** swapcontext( fromctx, toctx ) simply saves the current execution
*** 	context in fromctx and loads a new context from toctx. The context
***	must contain the PC, SP, and any registers defined to be preserved
***	across a call.
***
*** makecontext( ctx, fn, stack ) builds a context in ctx so that a call
***	to swapcontext will begin executing the function fn on the supplied
***	stack.
***
**/

#ifdef RS6000

typedef struct
{
	long	pc;
	long	sp;
	long	toc;
	long	cr;
	long	regs[32];	/* not all used			*/
} context;

extern int makecontext(context *ctx, void (*fn)(void), char *sp );
extern int swapcontext(context *fromctx, context *toctx);

#endif

#define STK_SIZE	0x15000

#include <errno.h>

typedef struct coroutine {
	context			co_context;
	struct	coroutine	*co_parent;
	char 			*stk_base;
} coroutine;

PRIVATE int	test;

PRIVATE struct coroutine	*RootCo;
        struct coroutine	*CurrentCo;
PRIVATE WORD			current_arg;

WORD InitCo ()
{ 
	RootCo = ( coroutine *) malloc (sizeof(coroutine)) ;

	if ( RootCo == NULL ) return( 0L );

	RootCo->co_parent = RootCo;

	CurrentCo = RootCo;

	return ( 1L );
}

ptr CreateCo ( function, size )
VoidFnPtr	function;
WORD		size;
{ coroutine *temp;
  
  temp = ( coroutine * ) malloc ( sizeof(coroutine) );
  if ( temp == NULL ) return (NULL);

  temp->co_parent = CurrentCo; 

  temp->stk_base = (char*)malloc(STK_SIZE); 
  if (temp->stk_base == NULL ) return (NULL); 
  
  makecontext(&(temp->co_context), function, temp->stk_base+STK_SIZE-0x200); 

  return( (ptr) temp);

}

WORD CallCo ( cortn, arg )
coroutine *cortn;
WORD arg;
{ coroutine *co = (coroutine*) cortn;

  co->co_parent = CurrentCo;
  CurrentCo = co;
  current_arg = arg;

  test = swapcontext( &(co->co_parent->co_context),&(co->co_context) );
  if ( test != 0){
                ServerDebug("server: swapcontext abort errno = %x \n",errno);
                exit(10);       
  }

  return ( current_arg );

}

WORD WaitCo (arg)
WORD arg;
{ coroutine *co = CurrentCo;

  current_arg = arg;
  CurrentCo = co->co_parent;

  test = swapcontext (&(co->co_context), &(co->co_parent->co_context));
  if ( test != 0){
                ServerDebug("server: swapcontext abort errno = %x \n",errno);
                exit(10);       
  }

  return(current_arg);

}

WORD DeleteCo(cortn)
ptr cortn;
{ coroutine *co = (coroutine*) cortn;

  if ( co->stk_base != NULL ) free(co->stk_base);
  free(co);
  return(TRUE);
}

