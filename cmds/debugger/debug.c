/**
*
* Title:  Helios Debugger - Program support.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/debug.c,v 1.4 1992/11/04 14:33:06 nickc Exp $";
#endif

#include "tla.h"

#ifdef PARSYTEC
#ifdef HE12
PRIVATE void worksig(void);
#endif
#endif

PUBLIC  Semaphore loadlock;
PUBLIC BOOL debugging;

#ifdef V1_1
PUBLIC  FILE my_iob[_MYSYS_OPEN];
#endif

/**
*
* sighandler();
*
* Signal handler.
*
**/
#ifdef PARSYTEC
PRIVATE void sighandler(int i)  /* CR: int was void */
{
  i = 1 ;   /* CR: just a dummy to keep compiler quite */
#endif
PRIVATE void sighandler () 
{
  tidyup();
  _exit(0);
}

/**
*
* initdebug(argc, argv);
*
* Initialise the world.
*
**/
PUBLIC void initdebug(int argc, char **argv)
{
  BOOL memchecking = FALSE;
  char *arg;

  debugging = FALSE;
  
  until ((arg = *++argv) == NULL)
  {
    if (arg[0] == '-')
    {
      switch (arg[1])
      {
      	case 'd':
/*
-- crf : 17/08/91 - report option
*/
        IOdebug ("tla : debugging enabled") ;
      	debugging = TRUE;
      	break;

      	case 'm':
/*
-- crf : 17/08/91 - report option
*/
        IOdebug ("tla : memory checking enabled") ;
      	memchecking = TRUE;
      	break;

      	default:
      	break;
      }
    }
  }
  initmem(memchecking);
  initsource();
#ifdef V1_1
  /* JMP: locking mechanism for fopen/fdopen requests*/
  InitSemaphore(&loadlock, 1);
  /* initialisation of iob table for my_fdopen my_fopen */
  memset(my_iob,0,sizeof(my_iob));
#endif

#ifdef PARSYTEC
#ifndef HE12
  signal(SIGINT, sighandler);
#else
  Fork ( 2000, worksig, 0 );
#endif
#endif

#ifdef OLDCODE
  signal(SIGINT, sighandler);
#endif
/*
-- crf : 16/07/91 - Bug 706
-- Trap signals to terminate debugger
*/
  {
    struct sigaction act;
    act.sa_handler = sighandler;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGINT, &act, NULL);
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGTERM, &act, NULL);
  }
}

/*
*
* debugf(format, ...);
*
* Formatted output to standard error.
*
**/
PUBLIC void debugf(char *format, ...)
{
  if (debugging)
  {
    va_list args;

    va_start(args, format);
#ifdef MYDEBUG
    fprintf(stderr, "MYTLA: ");
#else    
    fprintf(stderr, "TLA: ");
#endif    
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
  }
}

/**
*
* debug = newdebug(name);
*
* Create a new debugging session.
*
**/
PUBLIC DEBUG *newdebug(char *name)
{
  DEBUG *debug = NEW(DEBUG);

  /* ACE: Name must be stored in ObjNode structure */
  strcpy(debug->name, name);
  InitList(&debug->modulelist);
  InitList(&debug->breakpointlist);
  InitList(&debug->watchlist);
  InitList(&debug->watchpointlist);
  InitList(&debug->threadlist);
  debug->thread = NULL;
  return debug;
} 

/**
*
* startdebug(debug, port, reply);
*
* Start debugging using the specified ports.
*
**/
PUBLIC void startdebug(DEBUG *debug, Port port, Port reply)
{
  BOOL read_ok;

  
  debugf("startdebug()");
  debug->port = port;
  debug->reply = reply;
  GetEnv(port, &debug->env);
  debugf("open display");
  debugf("windowserver = %s", debug->env.Objv[OV_CServer]->Name);

  if (debug->env.Objv[OV_CServer]->Name != NULL AND 
      debug->env.Objv[OV_CServer]->Name != "\0")
  {
    debug->display = dopen(debug, debug->name, 
                           debug->env.Objv[OV_CServer]->Name);
  }
  else
  {
    debug->display = dopen(debug, debug->name, "/window");
  }
  
  debugf("new line");
  debug->line = newline(debug->display);
  debugf("new eval");
  debug->eval = neweval(debug);

  cmdmsg(debug, "Loading symbols for %s", debug->env.Objv[OV_Code]->Name);

#ifdef SYMBOLS
  debugf("new table");
  debug->table = (CHAIN *)newtable(); /* CR: cast for compiler */
#endif
  debugf("read code");

  read_ok = readcode(debug);
  /* returns false if .dbg is not located */

  Fork( 20000, interp, sizeof(DEBUG *), debug );

  cmdmsg(debug, "");
  debugf("startdebug() done");

  receiver(debug);
}

/**
*
* remdebug(debug);
*
* Remove debug instance.
*
**/
PUBLIC void remdebug(DEBUG *debug)
{
  debugf("remdebug()");
  (void)WalkList(&debug->modulelist, (WordFnPtr)remmodule, 0);
  debugf("free breakpoints");
  (void)WalkList(&debug->breakpointlist, (WordFnPtr)freebreakpoint, 0);
#ifdef NEWCODE
  (void)WalkList(&debug->watchlist, (WordFnPtr)freewatch, 0);
#endif
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)freewatchpoint, 0);
  debugf("free threads");
  (void)WalkList(&debug->threadlist, (WordFnPtr)remthread, 0);
  debugf("close display");
  dclose(debug->display);
  debugf("free port");
  FreePort(debug->port);
/*#ifdef OLDCODE   CR: I think this is necessary  */
  FreePort(debug->reply);
/* #endif                 *********/
#ifdef SYMBOLS
  walktable(debug->table, freesymbol, 0);
  freemem(debug->table);
#endif
  debugf("free interp");
  reminterp(debug->interp);
  debugf("free line");
  remline(debug->line);
  debugf("free eval");
  remeval(debug->eval);
/*
-- crf : 12/08/91 - close Objv, Strv
*/
  { 
    int i;
    WORD err ;
    if (debug->env.Objv != Null(Object *))
    { 
      for (i = 0; debug->env.Objv[i] != Null(Object); i++)
      {
        if (debug->env.Objv[i] != (Object *) MinInt)
        {
          if ((err = Close(debug->env.Objv[i])) != 0)
            IOdebug ("TLA: Error closing Objv[%d] : %x", i, err) ;
        }
      }
    }
    if (debug->env.Strv != Null(Stream *))
    { 
      for (i = 0; debug->env.Strv[i] != Null(Stream); i++)
      {
        if (debug->env.Strv[i] != (Stream *) MinInt)
        {
          if ((err = Close(debug->env.Strv[i])) != 0)
          {
#ifdef CRs
/*
-- crf : Strv[1] & [2] : fault c2098007 ("... invalid or corrupt Stream")
*/
            IOdebug ("TLA: Error closing Strv[%d] : %x", i, err) ;
#endif
          }
        }
      }
    }
/*
-- crf : 12/08/91 - free Argv
*/
    if ((err = Free (debug->env.Argv)) != 0)
      IOdebug ("TLA: Error freeing Argv : %x", err) ;
  }
  debugf("free debug");
  freemem(debug);
  debugf("remdebug() done");
}

#ifdef PARSYTEC
#ifdef HE12
/************************************************************
*
*
*	to terminate the debugger on ctrl C
*
*	CR: necessary to be compatible to Helios 1.2 cause the
*	    signalhandling is different (no new thread for a 
*	    signalhandler)
*
*
************************************************************/


PRIVATE void worksig(void)
{
  sigset_t *sigset = 0;
  
  signal(SIGINT,sighandler);
  sigsuspend(sigset);

}
#endif
#endif
