/**
*
* Title:  Helios Debugger - Monitor support.
*
* Author: Andy England
*
* Date:   February 1989
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/monitor.c,v 1.6 1993/07/06 14:10:32 nickc Exp $";
#endif

#include "tla.h"

#ifdef OLDCODE
PRIVATE void watchexpr(DEBUG *, WATCHPOINT *, EXPR *);
PRIVATE void addwatchmem(DEBUG *, WATCHPOINT *, EXPR *);
#endif

/**
*
* stopped(debug, id, modnum, line);
*
* Reports execution location.
*
**/
PUBLIC void stopped(DEBUG *debug, int id, int modnum, int line)
{
  THREAD *thread;
  LOCATION loc;
  BREAKPOINT *breakpoint;

  debugf("stopped(%d, %d)", modnum, line);

  if ((thread = findthread( debug, id )) == NULL)
    return;
  
  loc.module  = getmodule(debug, modnum);
  loc.line    = line;
  
  if (loc.module == NULL)
    {
      return;      
    }
  
  thread->loc = loc;

#ifdef SYMBOLS
  thread->block = findblock(loc);
#endif
  
  wgoto(thread->window, loc);
  
  cmdmsg(debug, "Stopped at line %d", loc.line);
  
  unless ((breakpoint = findbreakpoint(debug, loc)) == NULL OR breakpoint->docmd == NULL)
  {
    (void)cmdexec(debug, breakpoint->docmd);
  }
}

/**
*
* traced(debug, id, modnum, line);
*
* Reports execution location.
*
**/
PUBLIC void traced(DEBUG *debug, int id, int modnum, int line)
{
  THREAD *thread;
  LOCATION loc;

  debugf("traced(%d, %d)", modnum, line);

  if ((thread = findthread(debug, id)) == NULL) return;

  loc.module  = getmodule(debug, modnum);
  loc.line    = line;

  if (loc.module == NULL)
    {
      return;      
    }
  
  thread->loc = loc;

  
#ifdef SYMBOLS
  thread->block = findblock(loc);
#endif
  wgoto(thread->window, loc);
  cmdmsg(debug, "Traced line %d", loc.line);
  fprintf(thread->window->traceout," %s: line %d: traced\n",loc.module->name,loc.line) ;
  resume(thread);
}

/**
*
* entered(debug, id, modnum, offset);
*
* Notification of entry to a function.
*
**/
PUBLIC void entered(DEBUG *debug, int id, int modnum, int offset)
{
  ENTRY *entry;
  THREAD *thread;
  
  thread = findthread(debug, id);
  if ((entry = findfunction(getmodule(debug, modnum), offset)) == NULL)
  {
    cmdmsg(debug, "Entered unknown function");
    fprintf(thread->window->traceout,"Entered unknown function\n") ;
    return;
  }
  cmdmsg(debug, "Entered %s", entry->name);
  fprintf(thread->window->traceout,"Entered %s\n", entry->name);   
}

/**
*
* returned(debug, id, modnum, offset);
*
* Notification of return from a function.
*
**/
PUBLIC void returned(DEBUG *debug, int id, int modnum, int offset)
{
  ENTRY *entry;
  THREAD *thread;
  
  thread = findthread(debug, id);  
  if ((entry = findfunction(getmodule(debug, modnum), offset)) == NULL)
  {
    cmdmsg(debug, "Returned from unknown function");
    fprintf(thread->window->traceout, "Returned from unknown function\n");
    return;
  }
  cmdmsg(debug, "Returned from %s", entry->name);
  fprintf(thread->window->traceout, "Returned from %s\n",entry->name);
}

/**
*
* endthread(debug, id);
*
* Notification of termination of a thread.
*
**/
PUBLIC void endthread(DEBUG *debug, int id)
{
  THREAD *thread;

  if ((thread = findthread(debug, id)) == NULL) return;
  if (debug->thread == thread) nextthread(debug);
  remthread(thread);
}

/**
*
* addbreakpoint(debug, loc, toggle, count, docmd);
*
* Add a breakpoint.
*
**/
PUBLIC void addbreakpoint(DEBUG *debug, LOCATION loc, int count, char *docmd)
{
  BREAKPOINT *breakpoint;

#ifdef SYMBOLS
  unless (validline(loc)) cmderr(debug, "No statements on line %d", loc.line);
#endif
  if ((breakpoint = findbreakpoint(debug, loc)) == NULL)
  {
    if ((breakpoint = NEW(BREAKPOINT)) == NULL) cmderr(debug, "No memory");
    AddHead(&debug->breakpointlist, &breakpoint->node);
  }
  else unless (breakpoint->docmd == NULL) freemem(breakpoint->docmd);
  breakpoint->loc = loc;
  breakpoint->count = count;
  breakpoint->docmd = (docmd == NULL) ? NULL : strdup(docmd);
  sysaddbreak(debug, loc.module->modnum, loc.line, count);
  cmdmsg(debug, "Breakpoint set at %d", loc.line);
}

/**
*
* rembreakpoint(debug, loc);
*
* Remove a breakpoint.
*
**/
PUBLIC void rembreakpoint(DEBUG *debug, LOCATION loc)
{
  BREAKPOINT *breakpoint;

  if ((breakpoint = findbreakpoint(debug, loc)) == NULL)
    cmderr(debug, "No breakpoint at line %d", loc.line);
  sysrembreak(debug, loc.module->modnum, loc.line);
  freebreakpoint(breakpoint);
  cmdmsg(debug, "Breakpoint cleared at %d", loc.line);
}

/**
*
* breakpoint = findbreakpoint(debug, loc);
*
* Find the breakpoint at a given location.
*
**/
PRIVATE BOOL cmpbreak(BREAKPOINT *breakpoint, LOCATION *loc)
{
  return breakpoint->loc.module == loc->module AND
         breakpoint->loc.line == loc->line;
}

PUBLIC BREAKPOINT *findbreakpoint(DEBUG *debug, LOCATION loc)
{
  return (BREAKPOINT *)SearchList(&debug->breakpointlist, (WordFnPtr)cmpbreak, (word)&loc);
}

/**
*
* freebreakpoint(breakpoint);
*
* Free a breakpoint structure.
*
**/
PUBLIC void freebreakpoint(BREAKPOINT *breakpoint)
{
  Remove(&breakpoint->node);
  unless (breakpoint->docmd == NULL) freemem(breakpoint->docmd);
  freemem(breakpoint);
}

/**
*
* listbreakpoints(debug);
*
* Display list of all breakpoints.
*
**/
PRIVATE void putbreakpoint(BREAKPOINT *breakpoint, DISPLAY *display)
{
  char buf[80];

  dprintf(display, "breakpoint");
  unless (breakpoint->count == 1)
    dprintf(display, " -count %d", breakpoint->count);
  unless (breakpoint->docmd == NULL)
    dprintf(display, " -do [%s]", breakpoint->docmd);
  dprintf(display, " %s\n", formloc(buf, breakpoint->loc));
}

PUBLIC void listbreakpoints(DEBUG *debug)
{
  (void)WalkList(&debug->breakpointlist, (WordFnPtr)putbreakpoint, (word)debug->display);
}

#ifdef OLDCODE
PUBLIC void addwatch(DEBUG *debug, EXPR *expr)
{
  WATCH *watch = NEW(WATCH);

  watch->expr = expr;
  AddHead(&debug->watchlist, watch);
}

PRIVATE void putwatch(WATCH *watch)
{
  evaluate(watch->expr);
  putvalue(typeofexpr(watch->expr));
}

PUBLIC void putwatches(DEBUG *debug)
{
  (void)WalkList(&debug->watchlist, (WordFnPtr)putwatch, 0);
}

PUBLIC void freewatch(WATCH *watch)
{
  freeexpr(watch->expr);
  freemem(watch);
}

/**
*
* addwatchpoint(debug, expr); (OLDCODE)
*
* Add a watchpoint expression.
*
**/
PUBLIC void addwatchpoint(DEBUG *debug, EXPR *expr)
{
  WATCHPOINT *watchpoint;

  if ((watchpoint = NEW(WATCHPOINT)) == NULL) cmderr(debug, "No memory");
  watchpoint->expr = expr;
  InitList(&watchpoint->watchelementlist);
  watchpoint->recalc = FALSE;
  AddHead(&debug->watchpointlist, watchpoint);
  watchexpr(debug, watchpoint, expr);
}

PRIVATE BOOL cmpwatchelement(WATCHELEMENT *watchelement, MEM_LOCATION *loc)
{
  return watchelement->addr == loc->addr AND
         watchelement->size == loc->size;
}

PRIVATE void notifywatchpoint(WATCHPOINT *watchpoint, MEM_LOCATION *loc)
{
  if (SearchList(&watchpoint->watchelementlist, cmpwatchelement, (word)loc)
    == NULL) return;
  watchpoint->recalc = TRUE;
}

PUBLIC void notifywatchpoints(DEBUG *debug, byte *addr, int size)
{
  MEM_LOCATION loc;

  loc.addr = addr;
  loc.size = size;
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)notifywatchpoint, (word)&loc);
}

/* ACE: should really redisplay the value ? */
PRIVATE BOOL checkwatchpoint(WATCHPOINT *watchpoint)
{
  if (watchpoint->recalc)
  {
    watchpoint->recalc = FALSE;
    return TRUE;
  }
  return FALSE;
}

PUBLIC BOOL checkwatchpoints(DEBUG *debug)
{
  return (SearchList(&debug->watchpointlist, checkwatchpoint) == NULL) ?
    FALSE : TRUE;
}

PUBLIC void tracepoint(DEBUG *debug, EXPR *expr)
{
}

PRIVATE void watchexpr(DEBUG *debug, WATCHPOINT *watchpoint, EXPR *expr)
{
  if (expr == NULL) return;
  switch (expr->generic.op)
  {
    case T_IDENTIFIER: /* must watch */
    addwatchmem(debug, watchpoint, expr);
    return;

    case T_CONSTANT:
    case T_STRING:
    return;

    case T_CAST:
    case T_CONVERT:
    watchexpr(debug, watchpoint, expr->convert.expr);
    return;

    case T_INDIRECT:
    addwatchmem(debug, watchpoint, expr);
    watchexpr(debug, watchpoint, expr->generic.expr1);
    return;

    default:
    watchexpr(debug, watchpoint, expr->generic.expr1);
    watchexpr(debug, watchpoint, expr->generic.expr2);
    watchexpr(debug, watchpoint, expr->generic.expr3);
    return;
  }
}

PRIVATE void addwatchmem(DEBUG *debug, WATCHPOINT *watchpoint, EXPR *expr)
{
  WATCHELEMENT *watchelement;

  if ((watchelement = NEW(WATCHELEMENT)) == NULL) cmderr(debug, "No memory");
  watchelement->addr = evaladdress(expr);
  watchelement->size = sizeofexpr(expr);
  AddTail(&watchpoint->watchelementlist, watchelement);
  sysaddwatch(debug, watchelement->addr, watchelement->size);
}

PRIVATE void freewatchelement(WATCHELEMENT *watchelement)
{
  freemem(watchelement);
}

PUBLIC void freewatchpoint(WATCHPOINT *watchpoint)
{
  freeexpr(watchpoint->expr);
  (void)WalkList(&watchpoint->watchelementlist, (WordFnPtr)freewatchelement, 0);
  freemem(watchpoint);
}
#else

/**
*
* watchpoint = addwatchpoint(debug, expr, addr, size, docmd, format, silent);
*
* Add a watchpoint expression. (this is the actual one)
*
**/
PUBLIC WATCHPOINT *
addwatchpoint(
	      DEBUG * debug,
	      char *  expr,
	      void *  addr,
	      int     size,
	      char *  docmd,
	      FORMAT  format,
	      BOOL    silent,
	      BLOCK * block )
{
  WATCHPOINT *        watchpoint;

  
  if ((watchpoint = NEW(WATCHPOINT)) == NULL)
    cmderr(debug, "No memory");
  
  watchpoint->expr    = strdup(expr);
  watchpoint->addr    = addr;
  watchpoint->size    = size;
  watchpoint->docmd   = (docmd == NULL) ? NULL : strdup(docmd);
  watchpoint->format  = format;
  watchpoint->silent  = silent;
  watchpoint->block   = block; 	       	/* CR: save contents where wp has been defined */
  watchpoint->scope   = 0;		/* CR: needed to check definition level */
  watchpoint->thread  = debug->thread ;	/* CR: save thread where wp has been defined */
  
  debug->ifwatchpoint = NEW_Watchpoint;	/* CR: needed in locatestack */
    
  AddTail(&debug->watchpointlist, &watchpoint->node);
  
  sysaddwatch(debug, addr, size);
  
  return watchpoint;
}

/**
*
* remwatchpoint(debug, watchpoint);
*
* Remove a watchpoint.
*
**/
PUBLIC void remwatchpoint(DEBUG *debug, WATCHPOINT *watchpoint)
{
  sysremwatch(debug, watchpoint->addr, watchpoint->size);
  freewatchpoint(watchpoint);
}

PRIVATE BOOL cmpwatchpoint(WATCHPOINT *watchpoint, MEM_LOCATION *loc)
{
/*  debugf("cmpwatchpoint called");*/
/*  debugf("watchpoint->addr = %x\n       loc->addr = %x\n       watchpoint->size = %x\n       loc->size = %x", watchpoint->addr, loc->addr, watchpoint->size, loc->size);*/
  return watchpoint->addr == loc->addr AND
         watchpoint->size == loc->size;
}

#ifdef OLDCODE
PRIVATE void notifywatchpoint(WATCHPOINT *watchpoint, MEM_LOCATION *loc)
{
  if (cmpwatchpoint(watchpoint, loc))
  {
    IOdebug( "TLA: Watchpoint change value" );
  }
}
#endif



/**
*
* freewatchpoint(watchpoint);
*
* Free a watchpoint structure.
*
**/
PUBLIC void freewatchpoint(WATCHPOINT *watchpoint)
{
  Remove(&watchpoint->node);
  freemem(watchpoint->expr);
  unless (watchpoint->docmd == NULL) freemem(watchpoint->docmd);
  freemem(watchpoint);
}

/**
*
-- crf : 24/07/91 - Bug 708
*
* num_elements = num_watch_elements (watchpoint, debug) ;
*
* Return number of elements in watchpointed structure
*
-- This routine is used by "vinsert()" and "vdelete()" (within "display.c") to
-- ascertain the size of the watch window that must be allocated to the 
-- watchpointed variable.
-- I check if the type of the watchpointed expression is a structure 
-- (TI_Struct), and if so, walk through the list of members (I pinched this
-- last part from "putvalue()" (in "eval.c")).
**/
PUBLIC int num_watch_elements (WATCHPOINT *watchpoint, DEBUG *debug)
{
  EXPR *expr;
  TYPE *type ;
  MEMBER *member ;
  int num_elements = 0 ;

  if ((expr = parseexpr(debug->eval, watchpoint->expr, 
                        debug->thread->block)) == NULL)
    return num_elements ;

  genexpr(debug->eval, expr);
  type = skipreuse (typeofexpr(expr)) ;

/*
-- crf : 14/08/91 - Bug 712
-- Memory leak ... expr is not being freed
*/
  freeexpr (expr) ;

  if (type->generic.id == TI_Typedef)
    type = skiptypedef (type) ;

/*
-- crf : 11/08/91 - related to Bug 543a
-- Cater for 2-D arrays. This is a bit of a cludge ... am catering *only*
-- for 2-D arrays of simple types.
*/
  if (type->generic.id == TI_Array)
  {
    int size = type->array.size;
    type = type->array.host ;
    if (type->generic.id == TI_Array)
      return size ;
  }

  if (type->generic.id == TI_Struct)
  {
    for (member = (MEMBER *)type->structure.memberlist.head; member != NULL;
         member = (MEMBER *)member->link.next)
    {
      num_elements ++ ;    
    }
  }
  return num_elements ;
}

PUBLIC void putwatchpoint(WATCHPOINT *watchpoint, DEBUG *debug)
{
  EXPR *expr;
  
  debugf("called putwatchpoint for %s",watchpoint->expr);
  unless (watchpoint->silent)
  {
    dprintf(debug->display, "%s = ", watchpoint->expr);
    expr = parseexpr(debug->eval, watchpoint->expr, watchpoint->block);
    debugf("expr = %x",expr);
    if (expr != NULL)
    {
      genexpr(debug->eval, expr);
      putvalue(debug, typeofexpr(expr), watchpoint->format, 0, 0);

/*
-- crf : 14/08/91 - Bug 712
-- Memory leak ... expr is not being freed
*/
      freeexpr (expr) ;
    }
  }
  else
  {
    dprintf(debug->display, "%s = silent", watchpoint->expr);
  }
}

/**
*
* listwatchpoints(debug);
*
* Display list of all watchpoints.
*
**/
PRIVATE void listwatchpoint(WATCHPOINT *watchpoint, DISPLAY *display)
{
  dprintf(display, "watchpoint");
  unless (watchpoint->docmd == NULL)
    dprintf(display, " -do [%s]", watchpoint->docmd);
  dprintf(display, " %s\n", watchpoint->expr);
}

PUBLIC void listwatchpoints(DEBUG *debug)
{
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)listwatchpoint, (word)debug->display);
}
#endif


/********************************************************************
*
*	actualisewatchpoints
*
*	CR:   	
*
*	to actualise the watchpoints and their frame scope
*	and automatic removing of undefined wp (scope < 0 )
*
*	
*	loc.addr and loc.size are significant for a watchpoint, and
*	therefore used to identify the changed wp
*	maybe one should think about sending the actual wp-pointer
*	instead of generating it
*
********************************************************************/



#ifdef PARSYTEC
PUBLIC int actualisewatchpoints(DEBUG *debug, word addr, int size, word scope)
#endif

/* 
-- crf : 05/08/91 - additional parameter "del_watch_id" 
-- keep record of watchpoint numbers to be deleted
-- refer "system.c" (case DBG_Changed, DBG_DelWatchIds)
*/
PUBLIC int
actualisewatchpoints(
		     DEBUG *	debug,
		     word	addr,
		     int	size,
		     word	scope,
		     UWORD *	del_watch_id )

{
  WATCHPOINT *		watchpoint;
  MEM_LOCATION		loc;
  DISPLAY *		display = debug->display;
  UWORD			k;


  loc.addr = (void *) addr;
  loc.size = size;
  
  for (k = 0; k < display->varsize; k++)
    {
      watchpoint = display->varvec[ k ];
      
      if (cmpwatchpoint( watchpoint, &loc )) 	/* CR: means wp identified */
	{
	  if (scope <= -10)
	    {
	      /* 
	       * -- crf : keep record of watchpoint numbers to be deleted
	       */

	      if (watchpoint->scope < 0 && watchpoint->Class != C_Extern)
		{
		  *del_watch_id += 1UL << k ;
		}
	    }
	  else if (scope != 0) 			/* CR: means frame has changed no actualisation necessary */
	    {
	      watchpoint->scope += (int)scope;  /* CR: indicate frame */
	    }
	  else if (watchpoint->scope >= 0) 	/* CR: wp has changed, actualise it */
	    {
	      THREAD *	thread = debug->thread;	/* CR: to save the momentary thread */

	      
	      debug->thread = watchpoint->thread; /* CR: recall definition thread */
	      
	      
	      unless (watchpoint->silent) 
		{
		  cmdmsg( debug, "%s changed value", watchpoint->expr );
		  
		  /*
		   * XXX - NC - 26/4/93
		   *
		   * There is a bug with register variables.  More than one variable
		   * can be held in a register, (providing that they are local
		   * variables with different scopes).  Unfortunately we only have
		   * one static area of memory where registers are stored, so that
		   * when this area changes, all variables being watched and sharing
		   * the same register were being updated.  We fix this by storing
		   * the class of the variable being watched (see _watchpoint() in var.c)
		   * and if the variable is a register variable, only updating its
		   * contents when its scope is 0.  A scope of zero means that we
		   * are in the function where the watchpoint was set, which, since
		   * the variable is a local, must the function where the variable
		   * was defined.
		   */
		  
		  if (watchpoint->Class != C_Register || watchpoint->scope == 0)
		    {
		      vupdate( debug->display, watchpoint );
		    }
		}
	      
	      unless (watchpoint->docmd == NULL)
		{
		  debug->ifwatchpoint   = OLD_Watchpoint;
		  debug->tempwatchpoint = watchpoint->addr;
		  debug->thread->block  = watchpoint->block;
		  
		  (void)cmdexec( debug, watchpoint->docmd );
		  
		  debug->ifwatchpoint = NEW_Watchpoint;
		}
	      
	      debug->thread = thread;/* CR: restore old thread */
	    }

	  if (watchpoint->scope == -1 && 	/* CR: undefined wp, remove it */
	      watchpoint->Class != C_Extern )	/* XXX - NC - 26/4/93 - do not remove watchpoints on gloabls - they are never out of scope */
	    {
	      return TRUE;
	    }
	}
    }
  
  return FALSE;
}
