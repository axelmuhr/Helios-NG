/*------------------------------------------------------------------------
--                                                                      --
--                      C Coroutine library for Unix			--
--                      ----------------------------                    --
--                                                                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
--      colib.c                                                         --
--                                                                      --
--         Basic coroutine library functions for unix written in C,     --
--                                                                      --
--         evolved from a 68000 assembler version for the SUN by DJCH,  --
--                                                                      --
--         which was based on a version for the Atari ST using Mark     --
--                                                                      --
--         Williams Assembler by BLV, that was ported from a version    --
--                                                                      --
--         for the Amiga played with by Alan Cosslet, et al. The Amiga  --
--                                                                      --
--         version originally ported by Nick Garnett, probably from the --
--                                                                      --
--         original code for Tripos by Martin Richards/Brian Knight.    --
--                                                                      --
--                                                                      --
--      Author:  Paul Beskeen 10/3/89, BLV 21.5.89, Tony Jan '94        --
--									--
--      This file contains three C coroutine libraries -		--
--		i) The first uses various nasty signal handling 	--
--		routines which should be portable to any BSD 4.3 system	--
--		implementing the __set_sigreturn() or __set_sigcontext()--
--		calls.  To use this library compile this file with	--
--		-DSIGNALS.						--
--		ii) The second coroutine library uses the lightweight	--
--		process library available under Sun OS 4.0 and possibly	--
--		other versions of unix. To use this library compile with--
--		 -DLWP.
--		iii) The third library uses the thread handling system	--
--		under SOLARIS, synchronised by semaphires.  To use this	--
--		library compile with -DTHREADS.				--
--									--
--	If none of the coroutine libraries are available you may have 	--
--	to use an assembler one : 680x0 and 80x86 versions are included --
--	with the server sources.					--
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: colib.c,v 1.5 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

#if (ARMBSD)
#include "helios.h"
#else
#include "../helios.h"
#endif

#if (TR5 || i486V4)

#define STK_SIZE	0x15000

#include <sys/regset.h>
#include <ucontext.h>
#include <signal.h>
#include <errno.h>

typedef struct coroutine {
	ucontext_t		co_context;
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

  test = getcontext( &(temp->co_context) );	
  if ( test != 0){
                ServerDebug("server: getcontext abort errno = %x \n",errno);
                exit(10);       
  }

  makecontext(&(temp->co_context), function, 0); 
  
  temp->stk_base = (char*)malloc(STK_SIZE); 
  if (temp->stk_base == NULL ) return (NULL); 

#ifdef i486V4
#define R_R31 UESP
#endif

  temp->co_context.uc_mcontext.gregs[R_R31] = temp->stk_base;
  temp->co_context.uc_mcontext.gregs[R_R31] += STK_SIZE - 200;

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

#endif



#ifdef SIGNALS

/**
*** This version of the coroutine library uses the __set_sigreturn() or
*** __set_sigcontext() calls to store the current context of a coroutine and
*** switch to a different one. The original implementation is for the Acorn
*** R140 unix box, based on the Acorn ARM chip. 
**/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#if ARMBSD
#define __set_sigreturn __set_sigcontext
extern int errno;
#endif
/* Some systems use one some the other ! */
/* RiscIx for example documents __set_sigreturn, but uses __set_sigcontext ! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* external interface to library: */
PUBLIC  WORD fn( InitCo,   (void));
PUBLIC  WORD fn( CallCo,   (ptr, WORD));
PUBLIC  WORD fn( WaitCo,   (WORD));
PUBLIC  WORD fn( DeleteCo, (ptr));
PUBLIC  ptr  fn( CreateCo, (VoidFnPtr, WORD));

typedef struct  co_routine {
        struct co_routine*      co_parent;
        VoidFnPtr               co_func;
        struct  sigcontext      co_save_context;
} co_routine;

#if ARMBSD
/* arm registers saved in above co_save_context.sc_arm_regs[] */
#define ARMSB   10
#define ARMFP   11
#define ARMSP   13
#define ARMLINK 14
#endif

/* globals */
PRIVATE co_routine*     RootCo;         /* coroutine for main() */
        co_routine*     CurrentCo;      /* current coroutine running */
PRIVATE WORD            common_arg;     /* argument passed between coroutines */
PRIVATE co_routine*     co_buffer;      /* memory allocated for coroutine + temp */

/*============================================================================
 * Success = InitCo()
 *
 * Initialises a root co-routine that never goes away.  It corresponds directly
 * to the main level of the program and is really just a list header for all
 * other co-routines that get started.  The memory allocation could go in the
 * main allocator in Init() but I've left it here for clarity.
 *===========================================================================*/

WORD InitCo()
{
        /* debug ServerDebug("@InitCo");*/

        /* get memory for coroutine vars, context save area BUT NOT stack */
        /* use the original stack for the root coroutine */
        if ( !( co_buffer = (co_routine *)malloc(sizeof(co_routine)) ) )
                return 0L;

        /* root coroutine (main()) is its own parent */
        co_buffer->co_parent = co_buffer;

        /* set root to current */
        RootCo = CurrentCo = co_buffer;

        return 1L; /* success */
}

/*============================================================================
 * coroutine = CreateCo( function,size )
 *
 * Creates and adds a co-routine with the required stacksize and start
 * function size is in bytes
 *
 *==========================================================================*/

ptr CreateCo( function, size)
VoidFnPtr       function;
WORD            size;
{
        /* debug ServerDebug("@CreateCo");*/

        /* get memory for coroutine vars, context save area and stack */

        if ( !( co_buffer = (co_routine *)malloc(sizeof(co_routine) + size)  ))
                return NULL;

        /* initialise header */
        co_buffer->co_parent = CurrentCo;
        co_buffer->co_func = function;

/*ServerDebug("before first set"); - debug*/

        /* save parents context */
        if (__set_sigreturn(&CurrentCo->co_save_context) != 0)
          return (ptr) common_arg;
                /* return to parent on receipt of sigreturn() from WaitCo */

/*ServerDebug("before second set");  - debug*/
     /* save childs context so we can 'interfere' with its stack & frame ptrs */
        if (__set_sigreturn(&co_buffer->co_save_context) == 0)
        {		/* dont use signal stack, no signal mask to restore */
                co_buffer->co_save_context.sc_onstack = 0;
                co_buffer->co_save_context.sc_mask = 0;

#if ARMBSD
                /* set SP and FP to point at our new stack space */
                co_buffer->co_save_context.sc_cpu_regs[ARMSP] = 
                     (int) ((char *)co_buffer + sizeof(co_routine) + size);
                             /* point stack at end of stack area - grows down */
                co_buffer->co_save_context.sc_cpu_regs[ARMFP] = 0;
                            /* no backtraces */
#else
                co_buffer->co_save_context.sc_sp = 
                   (int) ((char *)co_buffer + sizeof(co_routine) + size);
#endif
/*ServerDebug("before first sigret"); - debug*/
                /* returns to above if statement, but now with our own stack */
                sigreturn(&co_buffer->co_save_context);

                /* should never get this far */
                ServerDebug("server: sigreturn abort %d @%d\n",errno,__LINE__);
                exit(10);       
        }

        CurrentCo = co_buffer;  /* set new coroutine to current coroutine */

/*ServerDebug("before waitco in creatco\n"); - debug*/
      /* now save our new improved context - returning here after next callCo */
        WaitCo(CurrentCo); /* return to parent */
        for (;;)
                /* call coroutine startup fn. + never exit current fn */
                (*CurrentCo->co_func)(common_arg);
}


/*============================================================================
 *  Result = CallCo( coroutine,arg )
 *
 * Starts up a coroutine that was just created or did a WaitCo to return an
 * arg.
 *===========================================================================*/

WORD CallCo(co,arg)
co_routine*     co;
WORD            arg;
{
        /* debug ServerDebug("@CallCo");*/
        /*debug */ if (co == NULL)
        {
                ServerDebug("Null co passed to CallCo");
                exit(11);
        }

        /* save state of current routine */
        if (__set_sigreturn(&CurrentCo->co_save_context) != 0)
                return common_arg; /* return to caller after */

        /* adopt coroutine as currents child */
        co->co_parent = CurrentCo;

        /*set new routine to current */
        CurrentCo = co;

        common_arg = arg;
        sigreturn(&CurrentCo->co_save_context); /* call coroutine */

        /* should never get this far */
        ServerDebug("server: sigreturn abort %d @%d\n",errno,__LINE__);
        exit(10);       
        return 1; /* just to get rid of cornpiler wornings */
}


/*=============================================================================
 * Arg = WaitCo( arg )
 *
 * Returns control back to the parent with required argument.
 * Arg will eventually be returned when the coroutine doing the WaitCo is
 * called again with CallCo(coroutine,ARG) or ResumeCo(coroutine,ARG)
 ============================================================================*/

WORD WaitCo(arg)
WORD arg;
{
        /* debug ServerDebug("@WaitCo");*/
        co_buffer = CurrentCo;

        /* set current co to its parent and become an orphan */
        CurrentCo = co_buffer->co_parent;
        co_buffer->co_parent = NULL;

        /* save coroutines context */
        if (__set_sigreturn(&co_buffer->co_save_context) != 0)
                /* have been resumed or called again */
                return common_arg;

        common_arg = arg;
        sigreturn(&CurrentCo->co_save_context);
        
        /* should never get this far */
        ServerDebug("server: sigreturn abort %d @%d\n",errno,__LINE__);
        exit(10);
        return 1; /* just to remove compiler chaffe */
}


/*=============================================================================
 * success = DeleteCo( coroutine )
 *
 * Deletes the stack area being used by a coroutine that is no longer needed
 * and unlinks it from the chain - if chaining implemented.
 ============================================================================*/

WORD DeleteCo(co)
struct co_routine* co;
{
        /* debug ServerDebug("@DeleteCo");*/

        if (co == NULL || co->co_parent != NULL){
	        ServerDebug("server: DeleteCo %x abort @%d\n",co,__LINE__);
                return 0;
        }

        free((byte *)co);
        return 1;
}

#endif   /* SIGNALS */


#ifdef LWP

/**
*** SUN OS 4.0 has a sensible lightweight process library, with multiple
*** threads running in the same unix process sharing the same address
*** space. This makes it very easy to implement coroutines.
**/

#include <lwp/lwp.h>
#include <lwp/stackdep.h>

typedef struct coroutine {
	struct coroutine *parent;
	thread_t 	thread;
	stkalign_t	stack[5000];
} coroutine;

WORD CurrentCo;
PRIVATE WORD current_arg;

WORD InitCo()
{ coroutine *temp = (coroutine *) malloc(sizeof(coroutine));
  int x;

  if (temp eq NULL) 
    {
      return(FALSE);
    }
  CurrentCo = (WORD) temp;

  if (pod_setmaxpri(2) ne 0)
   { ServerDebug("Coroutine error : pod_setmaxpri failed with %d",
                 lwp_geterr());
     return(FALSE);
   }

  if (lwp_setpri(SELF, MINPRIO) eq -1)
   { ServerDebug("Coroutine error : lwp_setpri on root failed with %d",
		 lwp_geterr());
     return(FALSE);
   }
  
                                  /* thread id of main coroutine */
  if (lwp_self(&(temp->thread)) ne 0)
   { ServerDebug("Coroutine error : lwp_self on root failed with %d",
		lwp_geterr());
     return(FALSE);
   }

  temp->parent = (coroutine *) NULL;

  return(TRUE);
}

WORD CallCo(cortn, arg)
ptr *cortn;
WORD arg;
{ coroutine *co = (coroutine *) cortn;
  int x;
 
  co->parent    = (coroutine *) CurrentCo;
  CurrentCo     = (WORD) cortn;
  current_arg   = arg;

  if (lwp_yield(co->thread) eq -1)
   { ServerDebug("Coroutine error : CallCo, failed to yield to child with %d",
		lwp_geterr());
   }

  return(current_arg);
}

WORD WaitCo(arg)
WORD arg;
{ coroutine *co = (coroutine *) CurrentCo;
  current_arg = arg;
  CurrentCo   = (WORD) co->parent;

  if (lwp_yield(co->parent->thread) eq -1)
   { ServerDebug("Coroutine error : WaitCo, failed to yield to parent with %d",
		lwp_geterr());
   }

  return(current_arg);
}


WORD DeleteCo(cortn)
ptr cortn;
{ coroutine *co = (coroutine *) cortn;

  if (lwp_destroy(co->thread) eq -1)
   ServerDebug("Coroutine error : DeleteCo failed with %d",
		lwp_geterr());
  
  free(co);

  return(TRUE);
}

ptr CreateCo(fun, stck)
VoidFnPtr fun;
WORD stck;
{
  /* coroutine *co = (coroutine *) malloc(sizeof(coroutine) + 20000); */
  coroutine *co = (coroutine *) malloc(sizeof(coroutine) + 40000);
  stkalign_t *stack;
  int x;

  if (co eq (coroutine *) NULL)
   return(NULL);
  
  stack =  STKTOP( co->stack );

  if (lwp_create(&(co->thread), fun, MINPRIO, LWPNOLASTRITES,stack,  0) eq -1)
   { ServerDebug("Coroutine error : failed to create with %d",
		 lwp_geterr());
     return(NULL);
   }

  return((ptr) co);
}

#endif /* LWP */


#ifdef THREADS

/*
 * Method:
 *
 *	Solaris does not contain any explicit thread yield functions (ie: yield
 * to a specific thread).  The alternative of doing a thr_continue (new_thread)
 * followed by a thr_suspend (this_thread) hangs because between the two function
 * calls, new_thread runs and suspends.  Hence the system hangs, both threads
 * waiting for the other.
 *
 */

#include <thread.h>
#include <signal.h>

#include <synch.h>

/* #define CO_DEBUG 1 */

struct coroutine_str
{
	struct coroutine_str *	co_parent;

	thread_t	co_thread;

	sema_t		co_semaphore;

	int		co_state;
};
typedef struct coroutine_str	Coroutine;
#define CO_NEW		1	/* new coroutine, not running yet */
#define CO_SUSPENDED	2	/* already running, but stopped   */
#define CO_RUNNING	3
#define CO_DEAD		4	

#define CO_SEMA_TYPE	USYNC_THREAD

Coroutine *	CurrentCo;	/* currently running coroutine */
int		CurrentArg;

int InitCo (void)
{
	CurrentCo = (Coroutine *)(malloc (sizeof (Coroutine)));

	if (CurrentCo == NULL)
	{
		ServerDebug ("Failed to initialise coroutines");

		return FALSE;
	}

	if (sema_init (&(CurrentCo -> co_semaphore), 0, CO_SEMA_TYPE, NULL))
	{
		ServerDebug ("Failed to initialise semaphore for initial coroutine");

		free (CurrentCo);

		return FALSE;
	}

	CurrentCo -> co_parent = CurrentCo;

	CurrentCo -> co_thread = thr_self ();

	CurrentCo -> co_state = CO_RUNNING;

	return TRUE;
}

Coroutine * CreateCo (void	*(*func)(void *),
		      int	stack_size = 0)
{
	Coroutine *	new_co;
	int		error;

#ifdef NEVER
	sigset_t 	oset;

	if (thr_sigsetmask (0, NULL, &oset))
	{
		ServerDebug ("Failed to get signal mask for thread %d", thr_self ());
	}
	else
	{
		printf ("CreateCo () - signal mask for thread %d is 0x%lx / %d\n", thr_self (), oset, oset);
		if (sigismember (&oset, SIGCHLD))
		{
			printf ("CreateCo () - SIGCHLD is a member\n");
		}
		else
		{
			printf ("CreateCo () - SIGCHLD is not a member\n");
		}
	}
#endif
	new_co = (Coroutine *)(malloc (sizeof (Coroutine)));

	if (new_co == NULL)
	{
		ServerDebug ("Failed to allocate space for new coroutine");

		return NULL;
	}

	error = thr_create (NULL,		/* auto allocate stack */
			   stack_size,
			   func,
			   NULL,
			   THR_DETACHED | THR_SUSPENDED,
			   &(new_co -> co_thread));

	if (error != 0)
	{
		ServerDebug ("Failed to create thread (error = %d (0x%lx))",
					error, error);

		free (new_co);

		return NULL;
	}

	if (sema_init (&(new_co -> co_semaphore), 0, CO_SEMA_TYPE, NULL))
	{
		ServerDebug ("Failed to initialise semaphore for new coroutine");

		free (new_co);

		return NULL;
	}

	new_co -> co_parent = CurrentCo;

	new_co -> co_state = CO_NEW;

	return new_co;
}

int CallCo (long *	co_ptr,
	    int		arg)
{
	/* organise pointers, start up child, suspend current running thread */

	Coroutine *	child_co = (Coroutine *)(co_ptr);
	Coroutine *	parent_co = CurrentCo;

	int	child_start_state = child_co -> co_state;

	sema_t *	child_sema = &(child_co -> co_semaphore);
	sema_t *	parent_sema = &(CurrentCo -> co_semaphore);

	CurrentArg = arg;

#ifdef CO_DEBUG
	printf ("CallCo () - current thread = %d (%d), starting up thread %d (sema_post (0x%lx), sema_wait (0x%lx))\n", 
			parent_co -> co_thread, thr_self (),
			child_co -> co_thread,
			child_sema, parent_sema);
#endif

	child_co -> co_state = CO_RUNNING;
	parent_co -> co_state = CO_SUSPENDED;

	child_co -> co_parent = CurrentCo;
	CurrentCo = child_co;

	if (child_start_state == CO_NEW)
	{
		thr_continue (child_co -> co_thread);
	}
	else
	{
		sema_post (child_sema);
	}
	sema_wait (parent_sema);

	return CurrentArg;
}

int WaitCo (int	arg)
{
	Coroutine *	child_co = CurrentCo;
	Coroutine *	parent_co = CurrentCo -> co_parent;

	sema_t *	child_sema = &(child_co -> co_semaphore);
	sema_t *	parent_sema = &(parent_co -> co_semaphore);

	/* start up parent and suspend current running thread */
	
	CurrentArg = arg;

#ifdef CO_DEBUG
	printf ("WaitCo () - current thread = %d (%d), starting up thread %d (sema_post (0x%lx), sema_wait (0x%lx))\n", 
			child_co -> co_thread, thr_self (),
			parent_co -> co_thread,
			parent_sema, child_sema);
#endif
	child_co -> co_state = CO_SUSPENDED;
	parent_co -> co_state = CO_RUNNING;

	CurrentCo = parent_co;

	sema_post (parent_sema);
	sema_wait (child_sema);

	/* check to see if this thread has been killed in while waiting */
	if (child_co -> co_state == CO_DEAD)
	{
		int	dummy = 0;
		/* yep! */
		sema_destroy (child_sema);

		/* parent may have changed (?) */
		parent_co = child_co -> co_parent;

		parent_sema = &(parent_co -> co_semaphore);

		sema_post (parent_sema);

		thr_exit ((void *)(&dummy));
	}

	return CurrentArg;
}	

int DeleteCo (long *	co_ptr)
{
	Coroutine *	dead_co = (Coroutine *)(co_ptr);
	Coroutine *	parent_co = CurrentCo;

	sema_t *	dead_sema = &(dead_co -> co_semaphore);
	sema_t *	parent_sema = &(parent_co -> co_semaphore);

#ifdef CO_DEBUG
	printf ("DeleteCo () - current thread = %d (%d), killing thread %d\n",
			CurrentCo -> co_thread, thr_self (),
			dead_co -> co_thread);
#endif
	dead_co -> co_state = CO_DEAD;

	dead_co -> co_parent = parent_co;
	CurrentCo = dead_co;

	sema_post (dead_sema);
	sema_wait (parent_sema);

	/* restore the world */
	free (dead_co);

	CurrentCo = parent_co;

	return TRUE;
}


#endif
