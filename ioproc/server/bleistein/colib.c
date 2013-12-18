/*------------------------------------------------------------------------
--                                                                      --
--                      C Coroutine library for Unix                    --
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
--      BRS extension...                                                --
--      This file contains a third coroutine library, using setjmp()    --
--      and longjmp() calls. To use this library compile this file with --
--      -DSETJMP.                                                       --
--      AND do not to control the stacksize during the linkphase        --
--      by the -s option with the value 2*1024*1024 Bytes!              --
--                                                                      --
------------------------------------------------------------------------*/

#include "../helios.h"

/**
*** This version of the coroutine library uses the setjmp() or
*** longjmp() calls to store the current context of a coroutine and
*** switch to another one.
**/

/* external interface to library: */
PUBLIC  WORD fn( InitCo,   (void));
PUBLIC  WORD fn( CallCo,   (ptr, WORD));
PUBLIC  WORD fn( WaitCo,   (WORD));
PUBLIC  WORD fn( DeleteCo, (ptr));
PUBLIC  ptr  fn( CreateCo, (VoidFnPtr, WORD));

****************************************************************
/**
*** The Server depends heavily on coroutines. All the coroutines are held in
*** linked lists, using the following structure.
**/
typedef struct { Node node;           /* conodes held in linked list  */
                 word id;             /* unique integer identifier    */
                 word type;           /* usually zero, may be suicide */
                                      /* or timedout or ready         */
                 word flags;
                 word timelimit;      /* when stream should die       */
                 ptr  cobase;         /* from CreateCo                */
                 void (**handlers)(); /* request handlers             */
                 ptr  extra;          /* coroutines global data       */
                 char name[128];      /* a name field if required     */
} Conode;

/**
*** Various odds and ends for use with coroutines
**/
#define CoSuicide (654321L)
#define CoTimeout (666666L)
#define CoReady   (777777L)

#define CoFlag_Floppy       0x0001L
#define CoFlag_CtrlCed      0x0002L
#define CoFlag_FileStream   0x0004L
#define CoFlag_Waiting      0x0008L
#define CoFlag_EOFed        0x0010L


#include <setjmp.h>

typedef int jmp_buf[_JBLEN];

union context
{
  struct {
      int bx;	/*                                   set to 0 */
      int si;	/*                                   set to 0 */
      int di;	/*                                   set to 0 */
      int bp;	/* basepointer                       set to 0 */
      int sp;	/* stackpointer                      set to allocated stack */
      int dx;	/* programm counter (return adress)  set to function adress */
    } s;
  
  jmp_buf x;
};

typedef union context CONTEXT;
****************************************************************


#define SPOTMARKED	0		/*@@@ BRS modification */
#define SETJMPWaitCo	1		/*@@@ BRS modification */
#define SETJMPCreateCo	2		/*@@@ BRS modification */
#define SETJMPCallCo	3		/*@@@ BRS modification */


typedef struct { Node  node;		/*@@@ BRS modification */
                 char *stack;		/*@@@ BRS modification */
		 int   size;		/*@@@ BRS modification */
} Freelist;				/*@@@ BRS modification - should be integrated in structs.h */
/* BLV - NO. Freelist is hardware specific, use unix386.h instead */

typedef struct  co_routine {
        struct co_routine*      co_parent;
        VoidFnPtr               co_func;
        jmp_buf			co_save_context;/*@@@ BRS modification */
        int			co_sp;		/*@@@ BRS modification */
        int			co_dx;		/*@@@ BRS modification */
        VoidFnPtr               co_trap;	/*@@@ BRS modification */
} co_routine;


#define BX 0
#define SI 1
#define DI 2
#define BP 3
#define SP 4
#define DX 5

#define COSTACKSIZE	8*1024			/*@@@ BRS modification */

#define DUMPJB
#define DEBUGDUMPJB		for (i=0; i<_JBLEN; i++){printf("0x%08x\n", co_buffer->co_save_context[i]);}


/* globals */
PRIVATE co_routine*     RootCo;         /* coroutine for main() */
        co_routine*     CurrentCo;      /* current coroutine running */
PRIVATE WORD            common_arg;     /* argument passed between coroutines */
PRIVATE co_routine*     co_buffer;      /* memory allocated for coroutine + temp */

int topofstack, bottomofstack;


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
	int i;
	static jmp_buf tmp;
	static int first=1;
	
        /* debug ServerDebug("@InitCo");*/

        /* get memory for coroutine vars, context save area BUT NOT stack */
        /* use the original stack for the root coroutine */
	switch (setjmp(tmp))			/*@@@ BRS modification */
	  {
	    case SPOTMARKED:
	      /* debug ServerDebug("...state saved");*/
	      DUMPJB;
/*BRSNOTYET   cosinit();			 init coroutine stack */
	      if (first)
		{
		  topofstack=tmp[SP];
		  first=0;
		}
#ifdef BRSNOTYET
              printf("...Top of Stack=0x%08x\n", topofstack);
#endif	    
	      bottomofstack=topofstack-COSTACKSIZE;
#ifdef BRSNOTYET
	      printf("...Bottom of...=0x%08x\n", bottomofstack);
#endif
              break;
	
	    default:
	      /* debug ServerDebug("...after longjmp() reached");*/
	      return 0L;
	  }
	
	if ( !( co_buffer = (co_routine *)malloc(sizeof(co_routine)) ) )
                return 0L;

        /* root coroutine (main()) is its own parent */
        co_buffer->co_parent = co_buffer;
        co_buffer->co_sp = topofstack;
	
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
	int i;
	
        /* debug ServerDebug("@CreateCo");*/

        /* get memory for coroutine vars, context save area AND stack */

        if ( !( co_buffer = (co_routine *)malloc(sizeof(co_routine))  ))
                return NULL;

	if ( !( co_buffer->co_sp = cosalloc(size)  ))
	  {
                free(co_buffer);
                return NULL;
	  }
	
        /* initialise header */
        co_buffer->co_parent = CurrentCo;
        co_buffer->co_func = function;

/*ServerDebug("before first set"); - debug*/

        /* save parents context */
        switch (setjmp(CurrentCo->co_save_context)) /*@@@ BRS modification */
	  {
	    case SPOTMARKED:
	      /* debug ServerDebug("...state saved");*/
	      DUMPJB;
	      break;
	
	    default:
	      /* debug ServerDebug("...after longjmp() reached");*/
	      return (ptr) common_arg;
                /* return to parent on receipt of sigreturn() from WaitCo */
	  }
        
 /*ServerDebug("before second set");  - debug*/

	/* save childs context so we can 'interfere' with its stack & frame ptrs */
        switch (setjmp(co_buffer->co_save_context)) /*@@@ BRS modification */
	  {
	    case SPOTMARKED:
	      /* debug ServerDebug("...2nd state saved");*/
	      DUMPJB;
	      /* set SP to point at our new stack space */
	      co_buffer->co_save_context[BX]=0;
	      co_buffer->co_save_context[SI]=0;
	      co_buffer->co_save_context[DI]=0;
	      co_buffer->co_save_context[BP]=0;
	      co_buffer->co_save_context[SP]=co_buffer->co_sp;
	     /*BRSDONTKNOW co_buffer->co_save_context[DX]=(int)?; */
	      DUMPJB;

/*ServerDebug("before first sigret"); - debug*/
	      /* returns to above if statement, but now with our own stack */
	      longjmp(co_buffer->co_save_context, SETJMPCreateCo);	/*@@@ BRS modification */

	      /* should never get this far */
	      ServerDebug("server: sigreturn abort %d @%d\n",errno,__LINE__);
	      exit(10);
	
	    default:
	      /* debug ServerDebug("...after 2nd longjmp() reached");*/
	      break;
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
	int i;
	
        /* debug ServerDebug("@CallCo");*/
        /*debug */ if (co == NULL)
        {
                ServerDebug("Null co passed to CallCo");
                exit(11);
        }

        /* save state of current routine */
        switch (setjmp(CurrentCo->co_save_context)) /*@@@ BRS modification */
	  {
	    case SPOTMARKED:
	      /* debug ServerDebug("...state saved");*/
	      DUMPJB;
	      break;
	
	    default:
	      /* debug ServerDebug("...after longjmp() reached");*/
	      return common_arg; /* return to caller after */
	  }
        
        /* adopt coroutine as currents child */
        co->co_parent = CurrentCo;

        /*set new routine to current */
        CurrentCo = co;

        common_arg = arg;
        longjmp(CurrentCo->co_save_context, SETJMPCallCo); /* call coroutine */	/*@@@ BRS modification */

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
	int i;
	
        /* debug ServerDebug("@WaitCo");*/
        co_buffer = CurrentCo;

        /* set current co to its parent and become an orphan */
        CurrentCo = co_buffer->co_parent;
        co_buffer->co_parent = NULL;

        /* save coroutines context */
	switch (setjmp(co_buffer->co_save_context)) /*@@@ BRS modification */
	  {
	    case SPOTMARKED:
	      /* debug ServerDebug("...state saved");*/
	      DUMPJB;
	      break;
	
	    default:
	      /* debug ServerDebug("...after longjmp() reached");*/
                /* have been resumed or called again */
                return common_arg;
	  }

        common_arg = arg;
        longjmp(CurrentCo->co_save_context, SETJMPWaitCo);	/*@@@ BRS modification */

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

        if (co == NULL || co->co_parent == NULL)
                return 0;

        cosfree(co->co_sp);
        free((byte *)co);
        return 1;
}



