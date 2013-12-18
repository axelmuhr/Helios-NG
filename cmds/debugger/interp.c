/**
*
* Title:  Helios Debugger - Command Language Interpreter Support.
*
* Author: Andy England
*
* Date:   February 1989
*
*         (C) Copyright 1989 - 1992, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/interp.c,v 1.3 1992/10/27 15:41:18 nickc Exp $";
#endif

#include "tla.h"

PRIVATE void remmacro(MACRO *);

/**
*
* interp = newinterp(debug)
*
* Initialise the interpreter.
*
**/
PUBLIC INTERP *newinterp(DEBUG *debug)
{
  INTERP *interp;

  if ((interp = NEW(INTERP)) == NULL) return NULL;
  interp->debug = debug;
  InitList(&interp->aliaslist);
  InitList(&interp->definelist);
  interp->charindex = StackMax;
  InitSemaphore(&interp->lock, 1);
  return interp;
}

/**
*
* reminterp(interp);
*
* Free memory used by interpreter.
*
**/
PUBLIC void reminterp(INTERP *interp)
{
  (void)WalkList(&interp->aliaslist, (WordFnPtr)remmacro, 0);
  (void)WalkList(&interp->definelist, (WordFnPtr)remmacro, 0);
  freemem(interp);
}

/**
*
* lockinterp(interp);
*
* Gain exculsive acces to the interpreter.
*
**/
PUBLIC void lockinterp(INTERP *interp)
{
  Wait(&interp->lock);
}

/**
*
* unlockinterp(interp);
*
* Release the interpreter.
*
**/
PUBLIC void unlockinterp(INTERP *interp)
{
  Signal(&interp->lock);
}

/**
*
* remmacro(macro);
*
* Remove a macro definition.
*
**/
PRIVATE void remmacro(MACRO *macro)
{
  Remove(&macro->node);
  freemem(macro->name);
  freemem(macro->text);
  freemem(macro);
}

/**
*
* found = cmpname(macro, name)
*
* Support routine for alias(), define(), unalias() and undefine().
*
**/
PRIVATE BOOL cmpname(MACRO *macro, char *name)
{
  return strequ(name, macro->name);
}

/**
*
* alias(interp, name, cmd)
*
* Create an alias macro command.
*
**/
PUBLIC void alias(INTERP *interp, char *name, char *cmd)
{
  MACRO *macro;

  if ((macro = (MACRO *)SearchList(&interp->aliaslist, (WordFnPtr)cmpname, (word)name)) == NULL)
  {
    macro = NEW(MACRO);
    macro->name = strdup(name);
    AddHead(&interp->aliaslist, &macro->node);
  }
  else freemem(macro->text);
  macro->text = strdup(cmd);
}

/**
*
* define(interp, name, text)
*
* Create a define macro name.
*
**/
PUBLIC void define(INTERP *interp, char *name, char *text)
{
  MACRO *macro;

  if ((macro = (MACRO *)SearchList(&interp->definelist, (WordFnPtr)cmpname, (word)name)) == NULL)
  {
    macro = NEW(MACRO);
    macro->name = strdup(name);
    AddHead(&interp->definelist, &macro->node);
  }
  else freemem(macro->text);
  macro->text = strdup(text);
}

/**
*
* found = cmpmacro(macro, name)
*
* Support routine for getalias() and getdefine().
*
**/
PRIVATE BOOL cmpmacro(MACRO *macro, char *name)
{
  return optequ(name, macro->name);
}

/**
*
* cmd = getalias(interp, name)
*
* Get the alias macro for the specified command.
*
**/
PUBLIC char *getalias(INTERP *interp, char *name)
{
  MACRO *macro = (MACRO *)SearchList(&interp->aliaslist, (WordFnPtr)cmpmacro, (word)name);

  return (macro == NULL) ? NULL : macro->text;
}

/**
*
* text = getdefine(interp, name)
*
* Get the define macro for the specified name.
*
**/
PUBLIC char *getdefine(INTERP *interp, char *name)
{
  static char	buf[ WordMax + 1 ];  
  MACRO *	macro = (MACRO *)SearchList(&interp->definelist, (WordFnPtr)cmpmacro, (word)name);

  
  unless (macro == NULL) return macro->text;

  if (strequ(name, "buf"))
    {
      return interp->debug->line->buffer;
    }
  else if (strequ(name, "curfile"))
    {
      return interp->debug->thread->window->loc.module->name;
    }
  else if (strequ(name, "curline"))
    {
      sprintf(buf, "%d", interp->debug->thread->window->loc.line + interp->debug->thread->window->cur.row);
      return buf;
    }
  else if (strequ(name, "curloc"))
    {
      LOCATION loc = interp->debug->thread->window->loc;

      loc.line += interp->debug->thread->window->cur.row;
      return formloc(buf, loc);
    }
  else if (strequ(name, "file"))
    {
      return interp->debug->thread->loc.module->name;
    }
  else if (strequ(name, "line"))
    {
      sprintf(buf, "%d", interp->debug->thread->loc.line);
      return buf;
    }
  else if (strequ(name, "loc"))
    {
      return formloc(buf, interp->debug->thread->loc);
    }
  else if (strequ(name, "txt"))
    {
      return getcurtext(interp->debug->thread->window, buf);
    }
  
  return NULL;
}

/**
*
* unalias(interp, name);
*
* Remove an alias macro.
*
**/
PUBLIC void unalias(INTERP *interp, char *name)
{
  MACRO *macro;

  unless ((macro = (MACRO *)SearchList(&interp->aliaslist, 
          (WordFnPtr)cmpname, (word)name)) == NULL)
    remmacro(macro);
/*
-- crf : 18/08/91 - let the user know whats going on ...
*/
  else
    cmdmsg (interp->debug, "\"%s\" not aliased", name) ;
}

/**
*
* undefine(interp, name);
*
* Remove a define macro.
*
**/
PUBLIC void undefine(INTERP *interp, char *name)
{
  MACRO *macro;

  unless ((macro = (MACRO *)SearchList(&interp->definelist, (WordFnPtr)cmpname, (word)name)) == NULL)
    remmacro(macro);
/*
-- crf : 18/08/91 - let the user know whats going on ...
*/
  else
    cmdmsg (interp->debug, "\"%s\" not defined", name) ;
}

/**
*
-- crf : 17/07/91 - (minor) bug 678
*
* formattext(original, newtext)
*
* Format text for "list -a"
*
-- Tidy up alias listings (I'm using the same technique as for keys ... refer
-- key.c)
*
**/
PRIVATE void formattext(char *original, char *newtext)
{
  while (*original)
  {
    char this_char = *original ;
/*
-- crf : replace tabs and ';'s with newline + spaces
*/
    if ((this_char == '\t') || (this_char == ';'))
    {
      int count ;
      *newtext ++ = '\n' ;
      for (count = 0 ; count < strlen ("alias ") ; count ++)
        *newtext ++ = ' ' ;
      original ++ ;
      while ((*original == this_char) || (*original == ' '))
        original ++ ;
    }
    else
      *newtext ++ = *original ++ ;
  }
/*
-- crf : get rid of spaces, newlines at end of text
*/
  newtext -- ;
  while ((*newtext == '\n') || (*newtext == ' '))
    newtext -- ;
  *(++ newtext) = NULL ;
}

/**
*
* listaliases(interp, display);
*
* Display a list of all aliases.
*
**/
PRIVATE void putalias(MACRO *macro, DISPLAY *display)
{
  char newtext [1024] ;
/* 
-- crf : probably a bit big ... can't malloc 'cos I will be inserting 
-- characters into the text
*/
  formattext (macro->text, newtext) ;
  
  dprintf(display, "alias %s\n      [%s]\n", macro->name, newtext) ;

#ifdef OLDCODE
  dprintf(display, "alias %s [%s]\n", macro->name, macro->text);
#endif
}


PUBLIC void listaliases(INTERP *interp, DISPLAY *display)
{
  (void)WalkList(&interp->aliaslist, (WordFnPtr)putalias, display);
}

/**
*
* listdefines(interp, display);
*
* Display a list of all defines.
*
**/
PRIVATE void putdefine(MACRO *macro, DISPLAY *display)
{
  dprintf(display, "define %s [%s]\n", macro->name, macro->text);
}

PUBLIC void listdefines(INTERP *interp, DISPLAY *display)
{
  (void)WalkList(&interp->definelist, (WordFnPtr)putdefine, display);
}

/**
*
* pushchar(interp, c)
*
* Push a character onto the character stack.
*
**/
PUBLIC void pushchar(INTERP *interp, char c)
{
  if (interp->charindex == 0)
  {
    interp->charindex = StackMax;
    cmderr(interp->debug, "Character stack full");
  }
  interp->charstack[--interp->charindex] = c;
}

/**
*
* pushword(interp, text)
*
* Push a string onto the character stack.
*
**/
PUBLIC void pushword(INTERP *interp, char *text)
{
  int length = strlen(text);

  if (interp->charindex - length < 0)
  {
    interp->charindex = StackMax;
    cmderr(interp->debug, "Character stack full");
  }
  interp->charindex -= length;
  memcpy(&(interp->charstack[interp->charindex]), text, length);
}

/**
*
* pushcmd(interp, cmd)
*
* Push a command onto the character stack.
*
**/
PUBLIC void pushcmd(INTERP *interp, char *cmd)
{
  int length = strlen(cmd);

  if (length > 1 AND cmd[0] == '[' AND cmd[length - 1] == ']')
  {
    cmd++;
    length -= 2;
    cmd[length] = '\0';
  }
  pushchar(interp, '\0');
  pushword(interp, cmd);
}

/**
*
* c = popchar(interp)
*
* Pop a single character off the character stack.
*
**/
PUBLIC int popchar(INTERP *interp)
{
  return (interp->charindex == StackMax) ? '\0' :
    interp->charstack[interp->charindex++];
}

/**
*
* text = popword(interp, buffer, bufsiz)
*
* Pop a word off the character stack into the buffer
*
**/
PUBLIC char *
popword(
	INTERP *	interp,
	char *		buffer,
	uword		bufsiz )
{
#ifdef OLDCODE
  BOOL inhibit = FALSE;
#endif

  forever
    {
      char *	text;
      int	c;
      int	i = 0;
      

      do
	c = popchar( interp );
      while (isspace( c ));
      
      if (c == '\0')
	return NULL;
      else if (c == ';')
	{
	  buffer[ 0 ] = ';';
	  buffer[ 1 ] = '\0';
	  
	  return buffer;
	}
      else if (c == '[')
	{
	  int depth = 0;
	  
	  /*
	   * ACE: This version does not include the square brackets in the word.
	   */
	  
	  until ((c = popchar( interp )) == '\0')
	    {
	      if (c == '\\')
		{
		  buffer[ i++ ] = c;
		  
		  if (i >= bufsiz)
		    {
		      debugf( "warning - word buffer is full!" );
		      buffer[ i - 1 ] = '\0';
		      return buffer;
		    }
		  
		  if ((c = popchar( interp )) == '\0')
		    break;
		}
	      else if (c == '[')
		depth++;
	      else if (c == ']' AND --depth < 0)
		break;
	      
	      buffer[ i++ ] = c;
	      
	      if (i >= bufsiz)
		{
		  debugf( "warning - word buffer is full!" );
		  buffer[ i - 1 ] = '\0';
		  return buffer;
		}
	    }

	  buffer[ i ] = '\0';

	  return buffer;
	}
      else if (c == '"')
	{
	  until ((c = popchar( interp )) == '\0' OR c == '"')
	    {
	      if (c == '\\')
		{
		  buffer[ i++ ] = c;
		  
		  if (i >= bufsiz)
		    {
		      debugf( "warning - word buffer is full!" );
		      buffer[ i - 1 ] = '\0';
		      return buffer;
		    }
		  
		  if ((c = popchar( interp )) == '\0') break;
		}
	      
	      buffer[ i++ ] = c;
	      
	      if (i >= bufsiz)
		{
		  debugf( "warning - word buffer is full!" );
		  buffer[ i - 1 ] = '\0';
		  return buffer;
		}
	    }
	  
	  buffer[ i ] = '\0';
	  
	  return buffer;
	}
      else 
	{
#ifdef OLDCODE
	  if (c == '\'')
	    {
	      inhibit = TRUE;
	      
	      c = popchar(interp);
	    }
#endif
	  until (isspace( c ) OR c == ';' OR c == '\n' OR c == '\0')
	    {
	      if (c == '\\')
		{
		  buffer[ i++ ] = c;
		  
		  if (i >= bufsiz)
		    {
		      debugf( "warning - word buffer is full!" );
		      buffer[ i - 1 ] = '\0';
		      return buffer;
		    }
		  
		  if ((c = popchar( interp )) == '\0') break;
		}
	      
	      buffer[ i++ ] = c;
	      
	      if (i >= bufsiz)
		{
		  debugf( "warning - word buffer is full!" );
		  buffer[ i - 1 ] = '\0';
		  return buffer;
		}
	      
	      c = popchar( interp );
	    }
	  
	  pushchar( interp, c );
	  
	  buffer[ i ] = '\0';
	  
#ifdef OLDCODE
	  if (inhibit OR (text = getdefine(interp, buffer)) == NULL) return buffer;
#else
	  unless (buffer[ 0 ] == '$')
	    return buffer;
	  
	  if ((text = getdefine( interp, buffer + 1 )) == NULL)
	    {
	      buffer[ 0 ] = '\0';
	      
	      return buffer;
	    }
#endif
	  /* 
	    -- crf : 16/07/91 - bug698
	    -- If a command contains the string "$buf" - e.g. "print $buf" - then 
	    -- everytime the word "$buf" is popped from the stack, it gets expanded - 
	    -- i.e. into "print $buf" - and pushed back onto the stack. This process
	    -- will naturally never end ...
	    -- This solution is a bit simplistic. If the command popped from the stack
	    -- contains the text "$buf", it is assumed that the text is actually the
	    -- expansion of the argument "$buf". In this case do the following :
	    -- 1. change "$buf" in interp->...->buffer to "buf" to prevent further 
	    -- expansion (this part is not essential ... its just so that if the user
	    -- feels to need to repeat a silly command using the space bar, the error
	    -- message will be in context)
	    -- 2. push "buf" on to the stack (i.e. compress the expansion back and 
	    -- prevent further expansion)
	    */
	    {
	      char *	buf_ptr = strstr( text, "$buf" );
	      

	      if (buf_ptr)
		{
		  strcpy( buf_ptr, "buf" );
		  strcpy( interp->debug->line->buffer, text );
		  pushword( interp, "buf" );
		} 
	      else
		{
		  pushword( interp, text );
		}	      
	    }
	}
    }
  
} /* popword */

/**
*
* text = expandcmd(interp, buf, cmd, bufsiz);
*
* Perform macro expansion on a command.
*
**/
PUBLIC char *
expandcmd(
	  INTERP *	interp,
	  char *	buf,
	  char *	cmd,
	  uword		bufsiz )
{
  uword len = 0;

  
  pushcmd( interp, cmd );
  
  until (popword( interp, buf + len, bufsiz - len ) == NULL)
    {
      len = strlen( buf );
      
      buf[ len++ ] = ' ';
    }
  
  if (len == 0)
    buf[ len ] = '\0';
  else
    buf[ len - 1 ] = '\0';
  
  return buf;
}
