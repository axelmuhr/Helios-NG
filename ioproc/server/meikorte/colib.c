/*------------------------------------------------------------------------
--                                                                      --
--                  C Coroutine library for Meiko RTE			--
--                  ---------------------------------                   --
--                                                                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
--     Author : BLV 22.8.89                                             --
--                                                                      --
------------------------------------------------------------------------*/

#include "../helios.h"



/**
*** RTE : these have been written from scratch. Coroutines are implemented
*** as transputer processes synchronising via channels.
**/

typedef struct coroutine {
	VoidFnPtr fn;
	WORD      arg;
	CHAN      sync;
	struct    coroutine *parent;
	BYTE      stack[4];
} coroutine;

coroutine *CurrentCo;

WORD InitCo()
{ CurrentCo = (coroutine *) malloc(sizeof(coroutine));
  if (CurrentCo eq (coroutine *) NULL)
   return(0L);
   
  CurrentCo->sync   = MINT;
  CurrentCo->parent = CurrentCo;
  return(1L);
}

WORD CallCo(co, value)
coroutine *co;
WORD      value;
{ BYTE junk[1];
  coroutine *myco = CurrentCo;

  co->arg    = value;
  co->parent = myco;
  CurrentCo  = co;
  cwrite(&(co->sync), (void *) &(junk[0]), 1);
  cread(&(myco->chan), (void *) &(junk[0]), 1);
  CurrentCo = myco;
  return(myco->arg);
}

WORD WaitCo(value)
WORD value;
{ BYTE  junk[1];
  coroutine *parent = CurrentCo->parent;
  coroutine *myco   = CurrentCo;
  
  parent->arg = value;
  cwrite(&(parent->sync), (void *) &(junk[0]), 1);
  
  if (value eq CoDeleted)
   stopp();

  cread(&(myco->sync), (void *) &(junk[0]), 1);
  return(myco->arg);
}


WORD DeleteCo(co)
coroutine *co;
{ free(co)
  return(1L);
}

PRIVATE coroutine *newco;

PRIVATE void co_aux()        /* little routine to wait for the first CallCo */
{ coroutine *myco = newco;
  BYTE junk[1];
  
  setpri(1);
  cread(&(myco->sync), (void *) &(junk[0]), 1);
  
  (*(myco->fn)(myco->arg);
}

WORD CreateCo(function, stacksize)
VoidFnPtr function;
WORD      stacksize;
{ newco = (coroutine *) malloc(sizeof(coroutine) + stacksize);
  if (newco eq (coroutine *) NULL) return(0L);
  newco->fn = function;
  newco->sync = MINT;
  runp(&co_aux, 0, &(newco->stack[0]), stacksize);
  return(newco);
}

