/**
*
* Title:  Helios Debugger - Module support.
*
* Author: Andy England
*
* Date:   April 1989
*
*         (c) Copyright 1989 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/module.c,v 1.5 1993/03/17 17:43:45 nickc Exp $";
#endif

#include "tla.h"

/**
*
* addmodule(debug, name, modnum);
*
* Add a module.
*
**/
PUBLIC BOOL addmodule(DEBUG *debug, char *name, int modnum)
{
  MODULE *module = NEW(MODULE);

  module->name       = strdup(name);
  module->modnum     = modnum;
  module->debug      = debug;
#ifdef OLDCODE
  module->source     = loadsource(debug, name);
#else
  module->source     = NULL;
#endif
  
#ifdef SYMBOLS
  module->outerblock = loadinfo(debug, module);

  if (module->outerblock == NULL)
    {
      /* unable to load .dbg file for this module */

      debugf( "Unable to load .dbg file for module %s", name );
      IOdebug( "Debugger: Unable to load .dbg file for module %s", name );
    }  
#else
  module->outerblock = NULL;
#endif
  
  AddTail( &debug->modulelist, &module->node );
  
#ifndef BUG
 if (module->outerblock == NULL)
     return(FALSE);
 else
   return(TRUE);
#endif
}

/**
*
* remmodule(module);
*
* Remove a module.
*
**/
PUBLIC void remmodule(MODULE *module)
{
  Remove(&module->node);
  freemem(module->name);
  unless (module->source == NULL) unloadsource(module->source);
#ifdef SYMBOLS
  unless (module->outerblock == NULL) walkblock(module->outerblock, freeblock, 0);
#endif
  freemem(module);
}

/**
*
* found = cmpmodnum(module, modnum)
*
* Support routine for getmodule().
*
**/
PRIVATE BOOL cmpmodnum(MODULE *module, int modnum)
{
  return module->modnum == modnum;
}

/**
*
* module = getmodule(debug, modnum);
*
* Find a module.
*
**/
PUBLIC MODULE *getmodule(DEBUG *debug, int modnum)
{
  return (MODULE *)SearchList(&debug->modulelist, (WordFnPtr)cmpmodnum, modnum);
}

/**
*
* found = cmpmodule(module, name)
*
* Support routine for findmodule().
*
**/
PRIVATE BOOL cmpmodule(MODULE *module, char *name)
{
  return strequ(module->name, name);
}

/**
*
* module = findmodule(debug, name);
*
* Find a module.
*
**/
PUBLIC MODULE *findmodule(DEBUG *debug, char *name)
{
  return (MODULE *)SearchList(&debug->modulelist, (WordFnPtr)cmpmodule, name);
}

/**
*
* source = getsource(module);
*
* Get the source for a module.
*
**/
PUBLIC SOURCE *getsource(MODULE *module)
{
  SOURCE *source;

  debugf("getsource(%s, %d)", module->name, module->modnum);
  if ((source = module->source) == NULL)
  {
    module->source = source = loadsource(module->debug, module->name);
  }
  return source;
}
