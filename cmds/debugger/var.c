/**
*
* Title:  Helios Debugger - Variable support.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/var.c,v 1.7 1993/07/06 14:09:44 nickc Exp $";
#endif

#include "tla.h"

extern TYPE *skiptypedef(TYPE *);

/**
*
* _dump(debug, exprstr);
*
* Dump memory.
*
**/
PUBLIC void _dump(DEBUG *debug, char *exprstr)
{
#ifdef EVALUATION
  DISPLAY *display = debug->display;
  EXPR *expr;
  word *addr;
  int i, j;

  if ((expr = parseexpr(debug->eval, exprstr, debug->thread->block)) == NULL) return;
  addr = (word *)evaladdr(debug->eval, expr);
  dstart(display);
  for (i = 0; i < 16; i++)
  {
    dprintf(display, "%08x:  ", addr);
    for (j = 0; j < 4; j++)
      dprintf(display, " %08x", peekword(debug, addr++));
    dputc(display, '\n');
  }
  dend(display, TRUE);
#endif
}

/**
*
* _print(debug, exprstr, format, chase);
*
* Print the value of an expression.
*
**/
PUBLIC void _print(DEBUG *debug, char *exprstr, FORMAT format, int chase)
{
#ifdef EVALUATION
  DISPLAY *display = debug->display;
  EXPR *expr;

  TYPE *type ; /* crf : identify type of expr */

  if ((expr = parseexpr(debug->eval, exprstr, debug->thread->block)) == NULL) return;

/*
-- crf : 22/07/91 - Bug 241
-- Problem : printing functions gives strange results ... e.g. -
-- 1. "? main" gives "Popped n bytes to little" (a value is pushed onto the 
--    stack in "genexpr", but is not popped off in "putvalue")
-- 2. "? main()" hangs system !!! ("genexpr" fails to identify 
--    "expr->generic.op")
-- 3. "? <user-defined function(param)>" gives inconsistent (arbitrary ?)
--    results 
-- 
-- The easiest (and hopefully safest) way to get around this is to prevent
-- entry into "genexpr" in these cases. This can be done by examining the type 
-- of the expression, and jumping over "genexpr" and "putvalue" in the
-- appropriate cases. It *appears* that the "? main()" case can be detected
-- when type is NULL & generic.op is T_Call ... isfunction(type) is TRUE for 
-- the other cases.
--
-- I'm not convinced that this is the best solution - it will require
-- extensive testing.
*/

#ifdef CRAIGS_CODE
  type = expr->identifier.entry->type ;
#else
  switch (expr->generic.op)
    {
    default:
      debugf( "not sure how to extract type ..." );
      type = expr->identifier.entry->type;
      break;

    case T_Indirect:
      type = expr->generic.expr1->identifier.entry->type;
      break;
    }  
#endif
  
/* 
-- crf - test for the problem cases
*/
  if (((type == NULL) && expr->generic.op == T_Call) || ((type != NULL) && isfunction( type )))
  {
    cmderr (debug, "Cannot print value of \"%s\"", exprstr) ;
#ifdef OLDCRF
    dstart(display);
    dprintf(display, "Cannot print value of \"%s\"\n", exprstr) ;
    checkstack(debug->eval); /* crf : this is probably redundant now */
    dend(display, TRUE);
#endif
  }
  else
  { 
/*
-- crf : 17/08/91 - Bug 714
-- if expression is larger than stack size, tla will hang
*/
    if (sizeofexpr(expr) >= STACK_SIZE)
      cmderr (debug, "expression too large (%d byte limitation)", STACK_SIZE) ;
    else
    {
      genexpr(debug->eval, expr);
      dstart(display);
      dprintf(display, "%s = ", exprstr);
      putvalue(debug, typeofexpr(expr), format, 0, chase);
      dputc(display, '\n');
      checkstack(debug->eval);
      dend(display, TRUE);
    }
  }
  freeexpr(expr);
#endif
}

/**
*
* _watchpoint(debug, exprstr, docmd, format, silent);
*
* Create a watchpoint.
*
**/
PUBLIC void _watchpoint(DEBUG *debug, char *exprstr, char *docmd, FORMAT format, BOOL silent)
{
  /* ACE: For now, expression must be an lvalue */
#ifdef EVALUATION
  EXPR *expr;
  WATCHPOINT *watchpoint = NULL;

  if ((expr = parseexpr(debug->eval, exprstr, debug->thread->block)) == NULL) return;

/*
-- crf : 17/08/91 - Bug 714
-- if expression is larger than stack size, tla will hang
*/
  
  if (sizeofexpr(expr) >= STACK_SIZE)
    cmderr (debug, "expression too large (%d byte limitation)", STACK_SIZE) ;
  else
    watchpoint = addwatchpoint(debug, exprstr, evaladdr(debug->eval, expr), sizeofexpr(expr), docmd, format, silent, debug->thread->block);

  /*
   * XXX - NC - 26/4/93
   *
   * We need to know the class of variable being watchpointed,
   * as the watchpoint scoping mechanism does not work.
   * See actualisewatchpoints() in monitor.c for more details.
   */
   
  if (expr->generic.op == T_Identifier)
    {
      watchpoint->Class = expr->identifier.entry->Class;
    }
  else
    {
      watchpoint->Class = C_Auto;  /* XXX - unknown */
    }
  
/*
-- crf : 14/08/91 - Bug 712
-- Memory leak ... expr not being freed
*/
  freeexpr (expr) ;

  vinsert(debug->display, watchpoint);
#endif
}

/**
*
* _whatis(debug, exprstr);
*
* Print declaration of a variable or type of an expression.
*
**/
PUBLIC void _whatis(DEBUG *debug, char *exprstr)
{
  DISPLAY *display = debug->display;
  EXPR *expr;
  
  if ((expr = parseexpr(debug->eval, exprstr, debug->thread->block)) == NULL) return;
  dstart(display);
  switch (expr->generic.op)
  {
    case T_Identifier:
    putentry(expr->identifier.entry, display->fileout);
    break;

    case T_Arrow:
    case T_Dot:
    putentry(expr->generic.expr2->identifier.entry, display->fileout);
    break;

    default:
    puttype(typeofexpr(expr), display->fileout);
    dputc(display, '\n');
    break;
  }
  dend(display, TRUE);
  freeexpr(expr);
}

/**
*
* _where(debug);
*
* Display backtrace for current thread.
*
**/
PRIVATE void putparam(DEBUG *debug, PARAM *param, int frame)
{
  DISPLAY *display = debug->display;

  
  dprintf(display, "%s = ", param->name);
  
  genparam(debug->eval, param, frame);
  
  putvalue(debug, param->type, Default, 0, 0);
  
  unless (param->link.next == NULL)
    dprintf( display, ", " );
  
  return;  
}

/* -- crf : 07/08/91 - "all" not used */
PUBLIC void _where(DEBUG *debug) /* , BOOL all) */
{
  DISPLAY *	display = debug->display;
  LOCATION	loc;
  int		offset;
  int		frame = 0;

  
  dstart(display);
  
  until ((offset = syswhere(debug, debug->thread->id, frame, &loc)) == -1)
  {
    ENTRY *entry;
    PARAM *param;

    if ((entry = findfunction(loc.module, offset)) == NULL)
      {
	break;
      }    
    
    dprintf(display, "%s(", entry->name);
    
    for (param = (PARAM *)skiptypedef(entry->type)->function.paramlist.head;
         param != NULL; param = (PARAM *)param->link.next)
      putparam(debug, param, frame);
    
    dprintf(display, "), line %d in \"%s\"\n", loc.line, loc.module->name);
    
    frame++;
  }
  
  dend(display, TRUE);

  return;
  
} /* _where */

/**
*
* _whereis(debug, name);
*
*
*
**/
PUBLIC void _whereis(DEBUG *debug, char *name)
{
  ENTRY *entry;

  if ((entry = findvar(debug->table, debug->thread->block, name)) == NULL AND
      (entry = findtype(debug->table, name)) == NULL)
    cmderr(debug, "Undefined variable %s", name);
  /* ACE: Print full object path */
}


/**
*
* _which(debug, name);
*
* Display full location of object of given name in scope.
*
**/
PUBLIC void _which(DEBUG *debug, char *name)
{
  DISPLAY *display = debug->display;
  ENTRY *entry;
  char buf[80];

  if ((entry = findvar(debug->table, debug->thread->block, name)) == NULL AND
      (entry = findtype(debug->table, name)) == NULL)
    cmderr(debug, "Undefined variable %s", name);
  formvarloc(buf, entry);
  dstart(display);
  dprintf(display, "%s\n", buf);
  dend(display, TRUE);
}
