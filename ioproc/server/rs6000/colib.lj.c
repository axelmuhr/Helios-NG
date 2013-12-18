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
/* RcsId: $Id: colib.c,v 1.4 1992/10/09 12:25:37 martyn Exp $ */
/* Copyright (C) 1993, Perihelion Software Ltd.       			*/

#include "../helios.h"

/**
*** The RS6000 coroutine library makes use of the SIGSTACK facilities
*** provided by certain Unix systems. Basically it works as follows:
***
*** 1) to create a new coroutine a new stack is allocated. This stack
***    is installed as the signal handler stack and then SIGUSR1 is
***    invoked.
*** 2) the signal handler performs an _setjmp, creating a valid context
***    on the coroutine stack. _setjmp does not save signal state.
*** 3) the signal handler then siglongjmps back into the CreateCo
***    routine, thus finishing the signal handling but leaving a valid
***    context on the coroutine stack.
*** 4) CreateCo now resets the signal handling stack. There is now a bit
***    of malloc'ed memory containing a valid stack, and a jump buffer
***    pointing at it.
*** 5) CallCo and WaitCo perform _setjmp followed by _longjmp, thus saving
***    the current stack context and switching to a different stack.
*** 6) DeleteCo frees the stack.
**/

#if 1
#define Dbg(a) ServerDebug a
#else
#define Dbg(a)
#endif

#define STK_SIZE	15000

typedef struct Coroutine {
	jmp_buf			 JmpBuf;
	struct Coroutine	*Parent;
	VoidFnPtr		 Fun;
	byte			 Stack[STK_SIZE];
} Coroutine;

	Coroutine	*CurrentCo;
PRIVATE WORD		 Arg;

WORD InitCo()
{
  CurrentCo = (Coroutine *) malloc (sizeof(Coroutine) - STK_SIZE);
  if (CurrentCo == NULL) return( 0L );
  CurrentCo->Parent	= CurrentCo;
  Dbg(("InitCo: root coroutine is at %x", CurrentCo));
  return( 1L );
}


PRIVATE void createco_aux(int sig)
{
  Dbg(("CreateCo: running on new coroutine stack (%x), CurrentCo %x", &sig, CurrentCo));
  Dbg(("CreateCo: saving new coroutine state in %x", CurrentCo));
  if (_setjmp(CurrentCo->JmpBuf, 0) == 0)
   { Dbg(("CreateCo: new coroutine, jumping back via %x", CurrentCo->Parent->JmpBuf));
     siglongjmp((sigjmp_buf *) CurrentCo->Parent->JmpBuf, 1);
   }

  Dbg(("New coroutine has been called, CurrentCo %x", CurrentCo));
  forever
   (*CurrentCo->Fun)();
}


ptr CreateCo(VoidFnPtr function, WORD size)
{ Coroutine		*newco;
  struct sigaction	 act_save, act_new;
  struct sigstack	 stack_save, stack_new;
  sigset_t		 set_save, set_new;
  void			(*fn)(int);

  newco = (Coroutine *) malloc (sizeof(Coroutine));
  if (newco == NULL) return (NULL);

  Dbg(("CreateCo: new coroutine is at %x", newco));
  Dbg(("CreateCo: saving current state in %x", CurrentCo->JmpBuf));
  
  if (sigsetjmp((sigjmp_buf *) CurrentCo->JmpBuf, 1) == 0)
   { 
     newco->Parent = CurrentCo;
     CurrentCo = newco;

	/* Block all signals except SIGUSR1	*/
     sigfillset(&set_new);
     sigdelset(&set_new, SIGUSR1);
     sigprocmask(SIG_SETMASK, &set_new, &set_save);

	/* Set up the coroutine stack as the signal handling stack	*/
     stack_new.ss_sp	  = (caddr_t) &(newco->Stack[STK_SIZE - sizeof(int)]);
     stack_new.ss_onstack = 0;
     sigstack(&stack_new, &stack_save);

	/* Install the signal handling routine.				*/
     sigaction(SIGUSR1, NULL, &act_save);
     act_new.sa_handler	  = createco_aux;
     act_new.sa_mask	  = act_save.sa_mask;
     act_new.sa_flags	  = act_save.sa_flags | SA_ONSTACK;
     sigaction(SIGUSR1, &act_new, NULL);

     Dbg(("CreateCo: raising signal to activate new coroutine %x", newco));
	/* Now get the coroutine running.				*/
     raise(SIGUSR1);
   }
  else
   {	/* createco_aux has done the siglongjmp...			*/
 	/* switch back to default signal stack				*/
     sigstack(&stack_save, NULL);
	/* restore the signal handler.					*/
     sigaction(SIGUSR1, &act_save, NULL);
	/* and unblock the various signals.				*/
     sigprocmask(SIG_SETMASK, &set_save, NULL);

     	/* restore CurrentCo						*/
     CurrentCo = CurrentCo->Parent;
     Dbg(("CreateCo: new coroutine has been initialised, CurrentCo %x", CurrentCo));
   }

  Dbg(("CreateCo: returning new coroutine %x", newco));
  return(newco);
}

WORD CallCo(Coroutine *cortn, word arg)
{
  Dbg(("CallCo: calling coroutine %x, CurrentCo %x", cortn, CurrentCo));
  cortn->Parent		= CurrentCo;
  CurrentCo		= cortn;
  Arg	= arg;

  Dbg(("CallCo: saving current state in %x", cortn->Parent->JmpBuf));
  if (_setjmp(cortn->Parent->JmpBuf) == 0)
   { Dbg(("CallCo: jumping to coroutine via %x", cortn->JmpBuf));
     _longjmp(cortn->JmpBuf, 1);
   }
  else
   return(Arg);
}

WORD WaitCo(word arg)
{ Coroutine	*current	= CurrentCo;

  Dbg(("WaitCo: CurrentCo is %x, parent is %x", CurrentCo, CurrentCo->Parent));
  CurrentCo	= current->Parent;
  Arg		= arg;

  Dbg(("WaitCo: saving current state in %x", current->JmpBuf));
  if (_setjmp(current->JmpBuf, 0) == 0)
   { Dbg(("WaitCo: jumping to parent coroutine via %x", CurrentCo->JmpBuf));
     _longjmp(CurrentCo->JmpBuf, 1);
   }
  else
   return(Arg);
}

WORD DeleteCo(Coroutine *coroutine)
{ Dbg(("DeleteCo: releasing coroutine %x", coroutine));
  free(coroutine);
  return(TRUE);
}

