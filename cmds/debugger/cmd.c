/**
*
* Title:  Helios Debugger - Commands.
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
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/cmd.c,v 1.9 1993/03/19 16:50:48 nickc Exp $";
#endif

#include "tla.h"

extern BOOL debugging;

#define CMD_Alias	0x01
#define CMD_Break	0x02
#define CMD_Breakpoint	0x03
#define CMD_Cd		0x04
#define CMD_Continue	0x05
#define CMD_Cursor	0x06
#define CMD_Define	0x07
#define CMD_Delete	0x08
#define CMD_Dialog	0x09
#define CMD_Do		0x0a
#define CMD_Dump	0x0b
#define CMD_Edit	0x0c
#define CMD_Exit	0x0d
#define CMD_Free	0x0e
#define CMD_Go		0x0f
#define CMD_Help	0x10
#define CMD_If		0x11
#define CMD_Input       0x12
#define CMD_Key		0x13
#define CMD_Kill	0x14
#define CMD_List	0x15
#define CMD_Make	0x16
#define CMD_Menu	0x17
#define CMD_Page	0x18
#define CMD_Print	0x19
#define CMD_PrintEnv	0x1a
#define CMD_Profile	0x1b
#define CMD_Pwd         0x1c
#define CMD_Quit	0x1d
#define CMD_Refresh	0x1e
#define CMD_Return	0x1f
#define CMD_Search	0x20
#define CMD_Shell	0x21
#define CMD_Signal	0x22
#define CMD_Step	0x23
#define CMD_Stop	0x24
#define CMD_Thread	0x25
#define CMD_Timeout	0x26
#define CMD_Trace	0x27
#define CMD_Unalias	0x28
#define CMD_Undefine	0x29
#define CMD_Version	0x2a
#define CMD_View	0x2b
#define CMD_Watchpoint	0x2c
#define CMD_WhatIs	0x2d
#define CMD_Where	0x2e
#define CMD_WhereIs	0x2f
#define CMD_Which	0x30
#define CMD_While	0x31
#define CMD_Window	0x32

#define CMD_Map		0x33


/*
-- crf : 17/08/91
-- I think its better to restrict this to the "-d" option in the command line
*/
#ifdef PARSYTEC
#define CMD_Pdebug	0x33
#endif

typedef struct
  {
    char *	name;
    int		id;
  }
BUILTIN;

PRIVATE void _breakpoint(DEBUG *, char *, BOOL, BOOL, int, char *);
PRIVATE void _delete(DEBUG *, char *);
PRIVATE void _dialog(DEBUG *, char *, char *, BOOL);
PRIVATE void _free(DEBUG *, BOOL);
PRIVATE void _go(DEBUG *, BOOL, int, LOCATION);
PRIVATE void _if(DEBUG *, char *, char *, char *);
PRIVATE void _input(DEBUG *, char  *, BOOL);
PRIVATE void _key(DEBUG *, char *, char *);
PRIVATE void _keyoff(DEBUG *, char *);
PRIVATE void _kill(DEBUG *, BOOL);
PRIVATE void _list(DEBUG *, char *);
PRIVATE void _printenv(DEBUG *);
PRIVATE void _profile(DEBUG *, char *, BOOL);
PRIVATE void _quit(DEBUG *);
PRIVATE void _refresh(DEBUG *);
PRIVATE void _search(DEBUG *, BOOL, BOOL, char *);
PRIVATE void _signal(DEBUG *, char *);
PRIVATE void _step(DEBUG *, BOOL, BOOL, int);
PRIVATE void _stop(DEBUG *, BOOL);
PRIVATE void _trace(DEBUG *, BOOL, char *, char *, BOOL, BOOL, BOOL);
PRIVATE void _version(DEBUG *);
PRIVATE void _while(DEBUG *, char *, char *);

#ifdef PARSYTEC
BUILTIN builtins[53] =
#endif
BUILTIN builtins[52] =
{
  { "alias",      CMD_Alias },
  { "break",      CMD_Break },
  { "breakpoint", CMD_Breakpoint },
  { "cd",         CMD_Cd },
  { "continue",   CMD_Continue },
  { "cursor",     CMD_Cursor },
  { "define",     CMD_Define },
  { "delete",     CMD_Delete },
  { "dialog",     CMD_Dialog },
  { "do",         CMD_Do },
  { "dump",       CMD_Dump },
  { "edit",       CMD_Edit },
  { "exit",       CMD_Exit },
  { "free",       CMD_Free },
  { "go",         CMD_Go },
  { "help",       CMD_Help },
  { "if",         CMD_If },
  { "input",      CMD_Input },
  { "key",        CMD_Key },
  { "kill",       CMD_Kill },
  { "list",       CMD_List },
  { "make",       CMD_Make },
  { "menu",       CMD_Menu },
  { "page",       CMD_Page },
  { "print",      CMD_Print },
  { "printenv",   CMD_PrintEnv },
  { "profile",    CMD_Profile },
  { "pwd",        CMD_Pwd },
  { "quit",       CMD_Quit },
  { "refresh",    CMD_Refresh },
  { "return",     CMD_Return },
  { "search",     CMD_Search },
  { "shell",      CMD_Shell },
  { "signal",     CMD_Signal },
  { "step",       CMD_Step },
  { "stop",       CMD_Stop },
  { "thread",     CMD_Thread },
  { "timeout",    CMD_Timeout },	
  { "trace",      CMD_Trace },
  { "unalias",    CMD_Unalias },
  { "undefine",   CMD_Undefine },
  { "version",    CMD_Version },
  { "view",       CMD_View },
  { "watchpoint", CMD_Watchpoint },
  { "whatis",     CMD_WhatIs },
  { "where",      CMD_Where },
  { "whereis",    CMD_WhereIs },
  { "which",      CMD_Which },
  { "while",      CMD_While },
  { "window",     CMD_Window },
#ifdef PARSYTEC
  { "pdebug",	  CMD_Pdebug},
#endif
  { "map", 	  CMD_Map },
  NULL
};

/**
*
* builtin = getcmd(name)
*
* Support routine for _do().
*
**/
PRIVATE int getcmd(char *name)
{
  BUILTIN *builtin;

  for (builtin = &builtins[0]; builtin->name; builtin++)
    {
      if (strequ(builtin->name, name))
	return builtin->id;
    }
  return -1;
}

/*
 * the Map command is used to create recursive key
 * maps, (so that, for example, ctrl-x-o can be
 * implemented).  It takes a keymap as its argument,
 * gets the next key press from the user and then
 * tries to match that key against the key map.  If a
 * match is found, the corresponding command is
 * executed.
 */

PRIVATE void
_map(
     DEBUG *	pDebug,
     char *	pMap )
{
  int	c;


  /* get next key press from user */
  
  c = dgetkey( pDebug->line->display );

  /* push the key map onto the command evaluation stack */
  
  pushcmd( pDebug->interp, pMap );

  /* now pop words off the stack one at a time */
  
  forever
    {
      char	pBuffer[ WordMax + 1 ];
      

      /* get a word */
      
      if (popword( pDebug->interp, pBuffer, WordMax ) == NULL)
	{
	  /* no more words - produce an error message */
	  
	  formkeyname( pBuffer, c );
	  
	  cmderr( pDebug, "no key binding for key '%s'", pBuffer );
	  
	  break;
	}

      /* see if the word in the keymap matches the key pressed by the user */
      
      if (getkeyname( pBuffer ) == c)
	{
	  /* it does, now get the next word (the action to be performed) */
	  
	  if (popword( pDebug->interp, pBuffer, WordMax ) == NULL)
	    {
	      formkeyname( pBuffer, c );
	      
	      cmderr( pDebug, "No command for key '%s'", pBuffer );
	      
	      break;
	    }
	  else
	    {
	      /* execute the command */
	      
	      _do( pDebug, pBuffer );
	    }

	  /* empty rest on interptretation stack */
	  
	  while (popword( pDebug->interp, pBuffer, WordMax ) != NULL)
	    ;

	  /* break out of the forever loop */
	  
	  break;
	} 
     else
	{
	  /* the key and word did not match - skip the action for this word */
	  
	  if (popword( pDebug->interp, pBuffer, WordMax ) == NULL)
	    break; /* error ? */
	}      
    }

  /* finished */
  
  return;
  
} /* _map */
  
/**
*
* _do(debug, cmd)
*
* Execute a command string.
*
**/
PUBLIC void _do(DEBUG *debug, char *cmd)
{
  INTERP *interp = debug->interp;
  char buffer[WordMax + 1];
  char *argv[ArgMax + 1];
  int argc = 0;
  int level;
  jmp_buf savehome;


  memcpy(savehome, debug->interp->home, sizeof(jmp_buf));

  unless ((level = setjmp(debug->interp->home)) == 0)
    {
      while (--argc >= 0)
	{
	  freemem(argv[argc]);
	}
      
      until (popword(interp, buffer, WordMax) == NULL);
      
      memcpy(debug->interp->home, savehome, sizeof(jmp_buf));

      if (level > CommandLevel)
	{
	  longjmp(debug->interp->home, level);
	}
    
      return;
    }
  
  pushcmd(interp, cmd);
  
  forever
  {
    char *p;

    if ((p = popword(interp, buffer, WordMax)) == NULL OR strequ(buffer, ";"))
    {
      if (argc > 0)
      {
      	argv[argc] = NULL;
	
        switch (getcmd(argv[0]))
        {
          case CMD_Alias:
          if (argc < 3) cmderr(debug, "Too few arguments");
          argv[2] = formword(argv + 2); argc = 3;
          alias(interp, argv[1], argv[2]);
          break;

          case CMD_Break:
          longjmp(interp->home, BreakLevel);
          break;

          case CMD_Breakpoint:
          {
            BOOL off = FALSE;
            BOOL toggle = FALSE;
            char *docmd = NULL;
            int count = 1;
            int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-c[ount]"))
              {
                if (++i >= argc) cmderr(debug, "Missing count");
                count = atoi(argv[i]);
/*
-- crf : 18/08/91 - test for count < 1
*/
                if (count < 1) cmderr (debug, "Invalid count") ;
              }
              else if (optequ(argv[i], "-d[o]"))
              {
                if (++i >= argc) cmderr(debug, "Missing command");
                docmd = argv[i];
              }
              else if (optequ(argv[i], "-o[ff]")) off = TRUE;
              else if (optequ(argv[i], "-t[oggle]")) toggle = TRUE;
              else break;
            }
            unless (i < argc) cmderr(debug, "Missing location");
            _breakpoint(debug, argv[i], off, toggle, count, docmd);
          }
          break;

	case CMD_Cd:
          _cd(debug, argv[1]);
          break;

	case CMD_Continue:
          longjmp(interp->home, LoopLevel);
          break;

	case CMD_Cursor:
          unless (argc == 2) cmderr(debug, "Bad arguments");
          if (optequ(argv[1], "-u[p]")) cursorup(debug->thread->window);
          else if (optequ(argv[1], "-d[own]")) cursordown(debug->thread->window);
          else if (optequ(argv[1], "-l[eft]")) cursorleft(debug->thread->window);
          else if (optequ(argv[1], "-r[ight]")) cursorright(debug->thread->window);
          else if (optequ(argv[1], "-g[row]")) cursorgrow(debug->thread->window);
          else if (optequ(argv[1], "-s[hrink]")) cursorshrink(debug->thread->window);
          else cmderr(debug, "Unknown option '%s'", argv[1]);
          break;

	case CMD_Define:
          if (argc < 3) cmderr(debug, "Too few arguments");
          argv[2] = formword(argv + 2); argc = 3;
          define(interp, argv[1], argv[2]);
          break;

	case CMD_Delete:
	  if (argc <= 2)
	    cmderr(debug, "Missing number/option");

	  /*
	   * -- crf : 15/08/91 - Bug 713
	   * -- "delete -trace" interpreted as "delete -watchpoint"
	   */

	  if (optequ( argv[ 1 ], "-t[race]" ))
	    _do( debug, "trace -off" ) ;
          else
	    {
	      if (argc > 2)
		{
		  if (optequ(argv[1], "-a[lias]")) unalias(interp, argv[2]);
		  else if (optequ(argv[1], "-b[reakpoint]")) _breakpoint(debug, argv[2], TRUE, FALSE, 0, NULL);
		  else if (optequ(argv[1], "-d[efine]")) undefine(interp, argv[2]);
		  else if (optequ(argv[1], "-k[ey]")) _keyoff(debug, argv[2]);
#ifdef OLDCODE
		  else if (optequ(argv[1], "-t[race]")) _delete(debug, argv[2]);
#endif
		  else if (optequ(argv[1], "-w[atchpoint]")) _delete(debug, argv[2]);
		  else cmderr(debug, "Unknown option '%s'", argv[1]);
		}
	      else _delete(debug, argv[1]);
	    }
          break;

	case CMD_Dialog:
          {
       	    char *prompt = NULL;
       	    BOOL query = FALSE;
       	    int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-p[rompt]"))
              {
              	unless (++i < argc) cmderr(debug, "Missing prompt");
              	prompt = argv[i];
              }
              else if (optequ(argv[i], "-q[uery]")) query = TRUE;
              else break;
            }
            unless (i < argc) cmderr(debug, "Missing name");
            _dialog(debug, argv[i], prompt, query);
          }
          break;

	case CMD_Do:
          if (argc < 2) cmderr(debug, "Missing command");
          argv[1] = formword(argv + 1); argc = 2;
          _do(debug, argv[1]);
          break;

          case CMD_Dump:
#ifdef OLDCODE
          _dump(debug);
#else
          if (argc < 2) cmderr(debug, "Missing expression");
          argv[1] = formword(argv + 1); argc = 2;
          _dump(debug, argv[1]);
#endif
          break;

          case CMD_Edit:
          {
            LOCATION loc;

            if (argc > 1)
            {
              loc = getloc(debug, argv[1]);
              if (loc.module == NULL) cmderr(debug, "Bad location '%s'", argv[1]);
            }
            else loc = debug->thread->loc;
            _edit(debug, loc);
          }
          break;

          case CMD_Exit:
          longjmp(interp->home, FileLevel);
          break;

          case CMD_Free:
          if (argc > 1 AND optequ(argv[1], "-a[ll]")) _free(debug, TRUE);
          else _free(debug, FALSE);
          break;

          case CMD_Go:
          {
            BOOL all = FALSE;
            LOCATION loc;
            int frame = -1;
            int i;

            loc.module = NULL;
            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-a[ll]")) all = TRUE;
              else if (optequ(argv[i], "-f[rame]"))
              {
                unless (++i < argc) cmderr(debug, "Missing count");
                frame = atoi(argv[i]);
              }
              else if (optequ(argv[i], "-r[eturn]")) frame = 1;
              else if (optequ(argv[i], "-u[ntil]"))
              {
                unless (++i < argc) cmderr(debug, "Missing location");
                loc = getloc(debug, argv[i]);
                if (loc.module == NULL) cmderr(debug, "Bad location '%s'", argv[i]);
              }
              else cmderr(debug, "Unknown option '%s'", argv[i]);
            }
            _go(debug, all, frame, loc);
          }
          break;

          case CMD_Help:
          _help(debug, argv[1]);
          break;

          case CMD_If:
          {
            char *thencmd = NULL;
            char *elsecmd = NULL;
            int i;

            /* ACE: Have to rebuild expression */
            if (argc < 2) cmderr(debug, "Missing expression");
            for (i = 2; i < argc; i++)
            {
              if (optequ(argv[i], "-t[hen]") OR optequ(argv[i], "-d[o]"))
              {
                unless (++i < argc) cmderr(debug, "Missing command");
                thencmd = argv[i];
                continue;
              }
              if (optequ(argv[i], "-e[lse]"))
              {
                unless (++i < argc) cmderr(debug, "Missing command");
                elsecmd = argv[i];
                continue;
              }
            }
            _if(debug, argv[1], thencmd, elsecmd);
          }
          break;

          case CMD_Input:
          {
            BOOL silent = FALSE;
            int i;

            for (i = 1; i < argc; i++)
            {
               if (optequ(argv[i], "-s[ilent]")) silent = TRUE;
               else if (optequ(argv[i], "-v[erbose]")) silent = FALSE;
               else break;
            }
            unless (i < argc) cmderr(debug, "Missing filename");
            _input(debug, argv[i], silent);
          }
          break;

          case CMD_Key:
          if (argc < 3) cmderr(debug, "Too few arguments");
          if (optequ(argv[1], "-o[ff]")) _keyoff(debug, argv[2]);
          else
          {
            argv[2] = formword(argv + 2); argc = 3;
            _key(debug, argv[1], argv[2]);
          }
          break;

          case CMD_Kill:
          if (argc > 1 AND optequ(argv[1], "-a[ll]")) _kill(debug, TRUE);
          else _kill(debug, FALSE);
          break;

          case CMD_List:
          if (argc < 2) cmderr(debug, "Too few arguments");
          _list(debug, argv[1]);
          break;

          case CMD_Make:
          _make(debug);
          break;

          case CMD_Menu:
          {
      	    int cmdc = 0;
            char *title = NULL;
            char *cmdv[ArgMax + 1];
            char *labv[ArgMax + 1];
            int i = 1;

            if (argc > 1 AND optequ(argv[i], "-t[itle]"))
            {
              unless (++i < argc) cmderr(debug, "Missing title");
              title = argv[i++];
            }
            unless (i < argc) cmderr(debug, "No menu items");
            for (; i < argc; i++)
            {
              cmdv[cmdc] = argv[i];
              if (i + 1 < argc AND optequ(argv[i + 1], "-l[abel]"))
              {
              	i++;
              	unless (++i < argc) cmderr(debug, "Missing label");
                labv[cmdc++] = argv[i];
              }
              else labv[cmdc++] = argv[i];
            }
            cmdv[cmdc] = labv[cmdc] = NULL;
            _menu(debug, title, cmdc, cmdv, labv);
          }
          break;

          case CMD_Page:
          if (argc > 2) cmderr(debug, "Bad arguments");
          if (argc > 1)
          {
            if (optequ(argv[1], "-u[p]")) pageup(debug->thread->window);
            else if (optequ(argv[1], "-d[own]")) pagedown(debug->thread->window);
            else if (optequ(argv[1], "-f[irst]")) pagefirst(debug->thread->window);
            else if (optequ(argv[1], "-l[ast]")) pagelast(debug->thread->window);
            else cmderr(debug, "Unknown option '%s'", argv[1]);
          }
#ifdef OLDCODE
          else pageup(debug->thread->window);
#endif
/*
-- crf : 26/07/91 - related to (minor) bug 689
-- I've reversed the operation of "page -up" and "page -down" (refer
-- "display.c"). I think that "page" should be equivalent to "page -down". 
*/
          else pagedown(debug->thread->window);
          break;

          case CMD_Print:
          {
            FORMAT format = Default;
            int chase = 0;
            int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-a[scii]")) format = Ascii;
              else if (optequ(argv[i], "-b[inary]")) format = Binary;
              else if (optequ(argv[i], "-c[hase]"))
              {
              	unless (++i < argc) cmderr(debug, "Missing count");
                chase = atoi(argv[i]);
              }
              else if (optequ(argv[i], "-d[ecimal]"))     format = Decimal;
              else if (optequ(argv[i], "-e[rror]"))       format = Error;
              else if (optequ(argv[i], "-f[loat]"))       format = Float;
              else if (optequ(argv[i], "-h[exadecimal]")) format = Hexadecimal;
              else if (optequ(argv[i], "-i[ndirect]"))    chase  = 1;
              else if (optequ(argv[i], "-o[ctal]"))       format = Octal;
              else if (optequ(argv[i], "-s[tring]"))      format = STring;
              else if (optequ(argv[i], "-u[nsigned]"))    format = Unsigned;
              else break;
            }
            unless (i < argc) cmderr(debug, "Missing expression");
            argv[i] = formword(argv + i); argc = i + 1;
            _print(debug, argv[i], format, chase);
          }
          break;

          case CMD_PrintEnv:
          _printenv(debug);
          break;

          case CMD_Profile:
          {
            int i;
            char *name = NULL;
            BOOL off = FALSE;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-i[n]"))
              {
                unless (++i < argc) cmderr(debug, "Missing function name");
                name = argv[i];
              }
              else if (optequ(argv[i], "-o[ff]")) off = TRUE;
              else cmderr(debug, "Unknown option '%s'", argv[i]);
            }
            _profile(debug, name, off);
          }
          break;

          case CMD_Pwd:
          _pwd(debug);
          break;

          case CMD_Quit:
          _quit(debug);
          break;

          case CMD_Refresh:
          _refresh(debug);
          break;

          case CMD_Return:
          longjmp(interp->home, CommandLevel);
          break;

          case CMD_Search:
          {
            BOOL backward = FALSE;
            BOOL wrap = FALSE;
            int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-f[orward]")) backward = FALSE;
              else if (optequ(argv[i], "-b[ackward]")) backward = TRUE;
              else if (optequ(argv[i], "-n[owrap]")) wrap = FALSE;
              else if (optequ(argv[i], "-w[rap]")) wrap = TRUE;
              else break;
            }
            _search(debug, backward, wrap, argv[i]);
          }
          break;

          case CMD_Shell:
          if (argc > 1)
          {
            argv[1] = formword(argv + 1); argc = 2;
          }
          _shell(debug, argv[1]);
          break;

          case CMD_Signal:
          _signal(debug, argv[1]);
          break;

          case CMD_Step:
          {
            BOOL all = FALSE;
            BOOL over = FALSE;
            int count = 1;
            int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-a[ll]")) all = TRUE;
              else if (optequ(argv[i], "-o[ver]")) over = TRUE;
              else if (optequ(argv[i], "-c[ount]"))
              {
              	unless (++i < argc) cmderr(debug, "Missing count");
              	count = atoi(argv[i]);
              }
              else cmderr(debug, "Unknown option '%s'", argv[i]);
            }
            _step(debug, all, over, count);
          }
          break;

          case CMD_Stop:
          if (argc > 1 AND optequ(argv[1], "-a[ll]")) _stop(debug, TRUE);
          else _stop(debug, FALSE);
          break;

          case CMD_Thread:
          if (argc > 2) cmderr(debug, "Bad arguments");
          if (argc > 1)
          {
            if (optequ(argv[1], "-u[p]")) nextthread(debug);
            else if (optequ(argv[1], "-d[own]")) prevthread(debug);
            else cmderr(debug, "Unknown option '%s'", argv[1]);
          }
          else nextthread(debug);
          break;

          case CMD_Timeout:
          if (argc > 1 AND optequ(argv[1], "-a[ll]")) systimeoutall(debug);
          else systimeout(debug, debug->thread->id);
          break;

          case CMD_Trace:
          {
            int i;
            char *name = NULL;
            char *fname = NULL;
            BOOL all = FALSE;
            BOOL onentry = FALSE;
            BOOL onreturn = FALSE;
            BOOL off = FALSE;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-i[n]"))
              {
                unless (++i < argc) cmderr(debug, "Missing function name");
                name = argv[i];
              }
	      else if (optequ(argv[i], "-t[o]")) 
	      {
                unless (++i < argc) cmderr(debug, "Missing file name");
                fname = argv[i];
	      }
              else if (optequ(argv[i], "-a[ll]")) all = TRUE;
              else if (optequ(argv[i], "-o[ff]")) off = TRUE;
              else if (optequ(argv[i], "-e[ntry]")) onentry = TRUE;
              else if (optequ(argv[i], "-r[eturn]")) onreturn = TRUE;
	      else cmderr(debug, "Unknown option '%s'", argv[i]);
            }
            _trace(debug, all, name, fname, onentry, onreturn, off);
          }
          break;

          case CMD_Unalias:
	  if (argc < 2) cmderr(debug, "Missing alias name");
          unalias(interp, argv[1]);
          break;

          case CMD_Undefine:
	  if (argc < 2) cmderr(debug, "Missing name");
          undefine(interp, argv[1]);
          break;

          case CMD_Version:
          _version(debug);
          break;

          case CMD_View:
          {
            LOCATION loc;

            if (argc < 2) cmderr(debug, "Missing location");
            loc = getloc(debug, argv[1]);
            if (loc.module == NULL) cmderr(debug, "Bad location '%s'", argv[1]);
            view(debug->thread->window, loc);
          }
          break;

#ifdef SYMBOLS
          case CMD_Watchpoint:
          {
            FORMAT format = Default;
            char *docmd = NULL;
            BOOL silent = FALSE;
            int i;

            for (i = 1; i < argc; i++)
            {
              if (optequ(argv[i], "-do"))
              {
                if (++i >= argc) cmderr(debug, "Missing command");
                docmd = argv[i];
              }
              else if (optequ(argv[i], "-a[scii]")) format = Ascii;
              else if (optequ(argv[i], "-b[inary]")) format = Binary;
              else if (optequ(argv[i], "-de[cimal]")) format = Decimal;
              else if (optequ(argv[i], "-e[rror]")) format = Error;
              else if (optequ(argv[i], "-f[loat]")) format = Float;
              else if (optequ(argv[i], "-h[exadecimal]")) format = Hexadecimal;
              else if (optequ(argv[i], "-o[ctal]")) format = Octal;
              else if (optequ(argv[i], "-u[nsigned]")) format = Unsigned;
              else if (optequ(argv[i], "-s[ilent]")) silent = TRUE;
              else if (optequ(argv[i], "-v[erbose]")) silent = FALSE;
              else break;
            }
            unless (i < argc) cmderr(debug, "Missing expression");
            argv[i] = formword(argv + i); argc = i + 1;
            _watchpoint(debug, argv[i], docmd, format, silent);
          }
          break;

          case CMD_WhatIs:
          if (argc < 2) cmderr(debug, "Missing expression");
          argv[1] = formword(argv + 1); argc = 2;          
          _whatis(debug, argv[1]);
          break;

          case CMD_Where:
          {
#ifdef OLDCODE
            BOOL all = FALSE;

            if (argc > 1 AND optequ(argv[1], "-a[ll]")) all = TRUE;

            _where(debug, all);
#endif
/* -- crf : 07/08/91 - "all" not used in "_where" */
            _where(debug);
          }
          break;

	case CMD_WhereIs:
          if (argc < 2) cmderr(debug, "Missing name");
          _whereis(debug,argv[1]);
          break;

	case CMD_Which:
          if (argc < 2) cmderr(debug, "Missing name");
          _which(debug, argv[1]);
          break;
#endif

	case CMD_While:
          /* ACE: probably have to rebuild the expression string */
          if (argc < 2) cmderr(debug, "Missing expression");
          _while(debug, argv[1], argv[3]);
          break;
	  
	case CMD_Window:
          if (argc > 2) cmderr(debug, "Bad arguments");
          if (argc > 1)
          {
            if (optequ(argv[1], "-g[row]")) wgrow(debug->thread->window);
            else if (optequ(argv[1], "-s[hrink]")) wshrink(debug->thread->window);
          }
          else wgrow(debug->thread->window);
          break;

#ifdef PARSYTEC
	case CMD_Pdebug:
	  if(debugging) debugging = FALSE;
	  else debugging = TRUE;
	  break;
#endif
	  
	case CMD_Map:
	  if (argc < 2)
	    cmderr( debug, "Missing keymap" );
	  else
	    _map( debug, argv[ 1 ] );
	  break;
	  
	default:
          cmderr(debug, "Unknown command '%s'", argv[0]);
          break;
        }
        while (--argc >= 0) freemem(argv[argc]);
        argc = 0;
      }
      if (p == NULL)
      {
        memcpy(debug->interp->home, savehome, sizeof(jmp_buf));
        return;
      }
    }
    else if (strequ(buffer, "<"))
    {
      if (popword(interp, buffer, WordMax) == NULL)
        cmderr(debug, "Missing name for redirection");
    }
    else if (strequ(buffer, ">") OR strequ(buffer, ">>"))
    {
      if (popword(interp, buffer, WordMax) == NULL)
        cmderr(debug, "Missing name for redirection");
    }
    else
    {
      if (argc == 0)
      {
        char *text;

        unless ((text = getalias(interp, buffer)) == NULL)
        {
          pushword(interp, text);
          continue;
        }
      }
      unless (argc < ArgMax) cmderr(debug, "Too many words");
      argv[argc++] = strdup(buffer);
    }
  }
}

/**
*
* _breakpoint(debug, locstr, count, docmd, toggle, off);
*
* Set or delete a breakpoint.
*
**/
PRIVATE void _breakpoint(DEBUG *debug, char *locstr, BOOL off, BOOL toggle, int count, char *docmd)
{
  LOCATION loc = getloc(debug, locstr);

  if (loc.module == NULL) cmderr(debug, "Bad location '%s'", locstr);
  if (toggle)
  {
    if (findbreakpoint(debug, loc) == NULL) addbreakpoint(debug, loc, count, docmd);
    else rembreakpoint(debug, loc);
  }
  else if (off) rembreakpoint(debug, loc);
#ifdef OLDCODE
  else addbreakpoint(debug, loc, count, docmd);
#endif
/*
-- crf : 18/08/91 - Bug 715
-- If multiple breakpoints are set at the same line, it is not possible to
-- remove the breakpoint from that line. 
-- Solution : if a breakpoint already exists at the location, remove it 
-- before adding the new one.
*/
  else
  {
    if (findbreakpoint(debug, loc) != NULL) rembreakpoint(debug, loc);
    addbreakpoint(debug, loc, count, docmd);
  }
}

/**
*
* _delete(debug, number);
*
* Delete a monitor.
*
**/
PRIVATE void _delete(DEBUG *debug, char *number)
{
  vdelete(debug->display, atoi(number));
}

/**
*
* _dialog(debug, name, prompt, query);
*
* Prompts user for input and sets a define variable to the result.
*
**/
PRIVATE void _dialog(DEBUG *debug, char *name, char *prompt, BOOL query)
{
  char buffer[80];

  if (prompt == NULL) prompt = "? ";
  if (getinput(debug->line, buffer, prompt) == NULL)
    {
      longjmp(debug->interp->home, CommandLevel);
    }  
  if (strlen(buffer) > 0) define(debug->interp, name, buffer);
  if (query AND !optequ(buffer, "y[es]"))
    {      
      longjmp(debug->interp->home, CommandLevel);
    }  
}

#ifdef OLDCODE
/**
*
* _dump(debug);
*
* Dump the symbol tables.
*
**/
PUBLIC void _dump(DEBUG *debug)
{
  DISPLAY *display = debug->display;

  dstart(display);
#ifdef SYMBOLS
  walktable(debug->table, putsymbol, (word)display->fileout);
#endif
  dend(display, TRUE);
}
#endif

/**
*
* _free(debug, all);
*
* Free threads.
*
**/
PRIVATE void _free(DEBUG *debug, BOOL all)
{
  if (all)
  {
    sysfreeall(debug);
    longjmp(debug->interp->home, TopLevel);
  }
  sysfree(debug, debug->thread->id);
}

/**
*
* _go(debug, all, frame, loc);
*
* Resume threads.
*
**/
PRIVATE void _go(DEBUG *debug, BOOL all, int frame, LOCATION loc)
{
  if (all)
  {
    (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
    if (loc.module == NULL)
    {
      if (frame == -1) sysgoall(debug);
      else sysgotoframeall(debug, frame);
    }
    else sysgoto(debug, debug->thread->id, loc.module->modnum, loc.line);
  }
  else
  {
    if (loc.module == NULL)
    {
      resume(debug->thread);
      if (frame == -1) sysgo(debug, debug->thread->id);
      else sysgotoframe(debug, debug->thread->id, frame);
    }
    else sysgotoall(debug, loc.module->modnum, loc.line);
  }
}

/**
*
* _if(debug, exprstr, thencmd, elsecmd);
*
* Conditional execute command lists.
*
**/
PRIVATE void _if(DEBUG *debug, char *exprstr, char *thencmd, char *elsecmd)
{
#ifdef EVALUATION
  if (evalcond(debug->eval, exprstr, debug->thread->block)) _do(debug, thencmd);
  else unless (elsecmd == NULL) _do(debug, elsecmd);
#endif
}

/**
*
* _input(debug, name, silent);
*
* Execute a command file.
*
**/
PRIVATE void _input(DEBUG *debug, char *name, BOOL silent)
{
  Object *object;
  FILE *file;
  char *cmd = NULL;
  int level;
  jmp_buf savehome;

  
  if ((object = Locate(debug->env.Objv[OV_Cdir], name)) == NULL)
  {
    unless (silent) cmderr(debug, "Unable to find '%s'", name);
    return;
  }
  
  if ((file = my_fopen(object->Name, "r")) == NULL)
  {
    Close(object);
    unless (silent) cmderr(debug, "Unable to open '%s'", name);
    return;
  }
  
  Close(object);
  
  memcpy(savehome, debug->interp->home, sizeof(jmp_buf));
  
  if ((level = setjmp(debug->interp->home)) == 0)
    {
      if ((cmd = (char *)newmem(10 * LineMax)) == NULL) 
	cmderr(debug, "No memory");

      until (fgets(cmd, LineMax, file) == NULL)
	{
	  int len = strlen(cmd);

	  
	  cmd[ len-- ] = '\0';

	  unless (cmd[ 0 ] == '#')
	    {
	      while (cmd[len - 1] == '\\')
		{
		  cmd[len--] = '\0';
		  if (len > 9 * LineMax) cmderr(debug, "Line too long");

		  if (fgets(cmd + len, LineMax, file) == NULL) break;
		  len += strlen(cmd + len);
		  cmd[len--] = '\0';
		}

	      if (testbreak( debug->display ))
		{
		  longjmp(debug->interp->home, FileLevel);
		}
	      
	      _do(debug, cmd);
	    }
	}
    }

/*
-- crf : 18/08/91 - let the user know whats going on ...
*/
  cmdmsg (debug, "Execution of command file completed") ;

  if (cmd != NULL)
    freemem(cmd);
  
  fclose(file);
  
  memcpy(debug->interp->home, savehome, sizeof(jmp_buf));
  
  if (level > FileLevel)
    {
      longjmp(debug->interp->home, level);
    }
  
  return;  
}

/**
*
* _key(debug, keyname, cmd);
*
* Define a key binding.
*
**/
PRIVATE void _key(DEBUG *debug, char *keyname, char *cmd)
{
  int c;

  if ((c = getkeyname(keyname)) == -1) cmderr(debug, "Bad keyname '%s'", keyname);
  addkey(debug->line->keymap, c, cmd);
}

/**
*
* _keyoff(debug, keyname);
*
* Undefine a key binding.
*
**/
PRIVATE void _keyoff(DEBUG *debug, char *keyname)
{
  int c;

  if ((c = getkeyname(keyname)) == -1) cmderr(debug, "Bad keyname '%s'", keyname);
  remkey(debug->line->keymap, c);
}

/**
*
* _kill(debug, all);
*
* Kill threads.
*
**/
PRIVATE void _kill(DEBUG *debug, BOOL all)
{
  if (all)
  {
    syskillall(debug);
    longjmp(debug->interp->home, TopLevel);
  }
  syskill(debug, debug->thread->id);
}

/**
*
* _list(debug, object);
*
* Display a list of the requested object.
*
**/
PRIVATE void _list(DEBUG *debug, char *object)
{
  DISPLAY *display = debug->display;

  dstart(display);
  if (optequ(object, "-a[liases]")) listaliases(debug->interp, display);
  else if (optequ(object, "-b[reakpoints]")) listbreakpoints(debug);
  else if (optequ(object, "-d[efines]")) listdefines(debug->interp, display);
  else if (optequ(object, "-k[eys]")) listkeys(debug->line->keymap, display);
  else if (optequ(object, "-w[atchpoints]")) listwatchpoints(debug);
  dend(display, TRUE);
}

/**
*
* _printenv(debug);
*
* Display the tasks environment.
*
**/
PRIVATE void _printenv(DEBUG *debug)
{
  DISPLAY *display = debug->display;
  char **argv = debug->env.Argv;
  char **envv = debug->env.Envv;
  char *arg;
  Stream **strv = debug->env.Strv;
  Stream *str;
  Object **objv = debug->env.Objv;
  Object *obj;

  dstart(display);
  dprintf(display, "Arguments:");
  until ((arg = *argv++) == NULL) dprintf(display, " %s", arg);
  dprintf(display, "\n\n");
  dprintf(display, "Environment variables:\n\n");
  until ((arg = *envv++) == NULL) dprintf(display, "%s\n", arg);
  dprintf(display, "\n");
  dprintf(display, "Streams:");
  until ((str = *strv++) == NULL)
    unless (str == (Stream *)MinInt) dprintf(display, " %s", str->Name);
  dprintf(display, "\n\n");
  dprintf(display, "Objects:");
  until ((obj = *objv++) == NULL) dprintf(display, " %s", obj->Name);
  dprintf(display, "\n\n");
  dend(display, TRUE);
}

/**
*
* _profile(debug, name, off);
*
* control profile information gathering.
*
**/
PRIVATE void _profile(DEBUG *debug, char *name, BOOL off)
{
  if (name == NULL) sysprofile(debug, debug->thread->id, -1, 0, off);
  else
  {
    ENTRY *entry;

    if ((entry = findvar(debug->table, debug->thread->block, name)) == NULL)
      cmderr(debug, "Undefined variable '%s'", name);

    if (entry == NULL || entry->block == NULL)
      cmderr( debug, "No block" );
    else
      sysprofile(debug, debug->thread->id, entry->block->module->modnum, entry->offset, off);
  }

  cmdmsg( debug, "OK" );

  return;  
}

/**
*
* _quit(debug);
*
* quit the current session.
*
**/
PRIVATE void _quit(DEBUG *debug)
{
  syskillall(debug);
  longjmp(debug->interp->home, TopLevel);
}

/**
*
* _refresh(debug);
*
* Refresh the display.
*
**/
PRIVATE void _refresh(DEBUG *debug)
{
  DISPLAY *display = debug->display;

  dlock(display);
  drefresh(display);
  dunlock(display);
}

/**
*
* _search(debug, backward, wrap, str);
*
* Search for a string in the current source file.
*
**/
PRIVATE void _search(DEBUG *debug, BOOL backward, BOOL wrap, char *str)
{
  LOCATION loc = debug->thread->window->loc;

  if ((loc.line = search(getsource(loc.module), str, loc.line + debug->thread->window->cur.row, backward, wrap)) == 0)
    cmderr(debug, "String not found");
  view(debug->thread->window, loc);
}

/**
*
* _signal(debug, signame);
*
* Send a signal to the client program.
*
**/
PRIVATE void _signal(DEBUG *debug, char *signame)
{
  Stream *stream;
  int sig = 0;

  debugf("signal(%s)", signame);
  if (signame == NULL) sig = SIGINT;
  else if (strequ( signame, "sigkill" )) sig = SIGKILL;
  else if (strequ( signame, "sigabrt" )) sig = SIGABRT;
  else if (strequ( signame, "sigfpe"  )) sig = SIGFPE;
  else if (strequ( signame, "sigill"  )) sig = SIGILL;
  else if (strequ( signame, "sigint"  )) sig = SIGINT;
  else if (strequ( signame, "sigsegv" )) sig = SIGSEGV;
  else if (strequ( signame, "sigterm" )) sig = SIGTERM;
  else if (strequ( signame, "sigstak" )) sig = SIGSTAK;
  else if (strequ( signame, "sigalrm" )) sig = SIGALRM;
  else if (strequ( signame, "sighup"  )) sig = SIGHUP;
  else if (strequ( signame, "sigpipe" )) sig = SIGPIPE;
  else if (strequ( signame, "sigquit" )) sig = SIGQUIT;
  else if (strequ( signame, "sigtrap" )) sig = SIGTRAP;
  else if (strequ( signame, "sigusr1" )) sig = SIGUSR1;
  else if (strequ( signame, "sigusr2" )) sig = SIGUSR2;
  else cmderr(debug, "Unknown signal name '%s'", signame);

  unless ((stream = Open( debug->env.Objv[ OV_Task ], "", O_ReadWrite )) == NULL)
    {
      word	res;

      
      if ((res = SendSignal( stream, sig )) < Err_Null)
	{
	  cmdmsg( debug, "unable to send signal, error code = %x", res );
	}
      else
	{
	  cmdmsg( debug, "signal sent" );    
	}
    
      Close( stream );
    }
  else
    {
      cmdmsg( debug, "unable to contact task" );
    }
  
  return;  
}

/**
*
* _step(debug, all, over, count);
*
* Single step threads.
*
**/
PRIVATE void _step(DEBUG *debug, BOOL all, BOOL over, int count)


{
  int i;
  if (all)
  {
    (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
    if (over) sysgotoframeall(debug, 0);
    else sysstepall(debug);
  }
  else
  {
    resume(debug->thread);
    if (over) sysgotoframe(debug, debug->thread->id, 0);
    else
    {
    	for (i = 1; i <= count; i++)  /** to step over count number of commands **/
    	{
    		sysstep(debug, debug->thread->id);
    	}
    }
  }
}

/**
*
* _stop(debug, all);
*
* stop threads.
*
**/
PRIVATE void _stop(DEBUG *debug, BOOL all)
{
  if (all) sysstopall(debug);
  else sysstop(debug, debug->thread->id);
}

/**
*
* _trace(debug, all, name, fname, onentry, onreturn, off);
*
* trace current thread.
*
**/
PRIVATE void _trace(DEBUG *debug, BOOL all, char *name, char *fname, BOOL onentry, BOOL onreturn, BOOL off)
{
  int mode = 0;

  /* ACE: support all */

  if (onentry)  mode |= TraceEntry;
  if (onreturn) mode |= TraceReturn;  
  if (all)      mode = TraceEntry | TraceReturn | TraceCommand;
  
  unless (onentry OR onreturn OR fname != NULL OR all)
    mode = TraceCommand;
  
  if (off)
    {
      mode |= TraceOff;
      
      unless (onentry OR onreturn OR all)  
	{ 
	  if (debug->thread->window->traceout != NULL)
	    {
	      fclose (debug->thread->window->traceout);
	      
	      debug->thread->window->traceout = NULL;
	      
	      fname = NULL;
	    }
	}
    }
  
  if (fname != NULL)
    {
      if (debug->thread->window->traceout != NULL)
	fclose (debug->thread->window->traceout);
      
      debug->thread->window->traceout = fopen (fname, "a+");
      
      if (debug->thread->window->traceout == NULL)
	cmderr(debug, "Cannot open file '%s'", fname);
    }
  
  if (name == NULL)
    {
      systrace(debug, debug->thread->id, -1, 0, mode);
    }
  else
    {
      ENTRY *	entry;

      
      if ((entry = findvar(debug->table, debug->thread->block, name)) == NULL)
	cmderr(debug, "Undefined function '%s'", name);

      if (entry == NULL || entry->block == NULL)
	cmderr( debug, "No Block" );
      else
	systrace(debug, debug->thread->id, entry->block->module->modnum, entry->offset, mode);
    }

  return;
  
} /* _trace */

/**
*
* _version(debug);
*
* Display version number of the debugger.
*
**/
PRIVATE void _version(DEBUG *debug)
{
  DISPLAY *display = debug->display;

  dstart(display);
/*
-- crf : 18/08/91 - the birth of Version 2.00 (Beta) ...
*/
#ifdef OLDCODE
  dprintf(display, "Helios Source Debugger Version 2.00 Beta 18/08/91\n");
#else
/*
-- crf : 01/10/91 - Version 2.00
-- Notes :
-- 1. did not get back any reports on the Beta version (!)
-- 2. only one change has been made to the Beta version (facility to
--    set environment variable to locate program sources)
*/
  dprintf(display, "Helios Source Debugger Version 2.01 Novermber 1992\n");
#endif
  dprintf(display, "(c) Copyright 1989-92 Perihelion Software Ltd.\n");

#ifdef OLDCODE
#ifdef V1_1
  dprintf(display, "Parsytec Helios Source Debugger Version 1.2 alpha 4/07/91\n");
#ifdef HE12
  dprintf(display, " (1.21 Version)\n");
#endif
  dprintf(display, "(c) Copyright 1989,1990 Perihelion Software Ltd.\n");
  dprintf(display, "(c) Copyright 1990,1991 Parsytec GmbH by CR \n");  
#else
  dprintf(display, "Helios Source Debugger Version 1.00 18/10/89\n");
  dprintf(display, "(c) Copyright 1989 Perihelion Software Ltd.\n");
#endif
#endif
  dprintf(display, "All rights reserved.\n\n");
  
  dprintf( display, "RCS ID: $Id: cmd.c,v 1.9 1993/03/19 16:50:48 nickc Exp $\n" );
  
  dend(display, TRUE);
}

/**
*
* _while(debug, exprstr, docmd);
*
* Repeatedly execute command lists.
*
**/
PRIVATE void _while(DEBUG *debug, char *exprstr, char *docmd)
{
#ifdef EVALUATION
  int level;
  jmp_buf savehome;

  memcpy(savehome, debug->interp->home, sizeof(jmp_buf));
  if ((level = setjmp(debug->interp->home)) < BreakLevel)
  {
    (void)testbreak(debug->display);
    while (evalcond(debug->eval, exprstr, debug->thread->block))
    {
      if (testbreak(debug->display))
      {
      	cmdmsg(debug, "Break");
        longjmp(debug->interp->home, FileLevel);
      }
      _do(debug, docmd);
    }
  }
  memcpy(debug->interp->home, savehome, sizeof(jmp_buf));
  if (level > BreakLevel)
    {
      longjmp(debug->interp->home, level);
    }
  
#endif
}

/**
*
* startup(debug);
*
* Execute all the startup files.
*
**/
PRIVATE void startup(DEBUG *debug)
{
  char *home;

  debugf("startup()");
  lockinterp(debug->interp);

  if (setjmp(debug->interp->home) == 0)
  {
    _input(debug, "/helios/etc/debugrc", TRUE);
      
    unless ((home = getvar(debug->env.Envv, "HOME")) == NULL)
    {
      char debugrc[PathMax + 1];

      strcpy(debugrc, home);
      strcat(debugrc, "/debugrc");
      _input(debug, debugrc, TRUE);
    }
    _input(debug, "debugrc", TRUE);
  }
  unlockinterp(debug->interp);

/*
-- crf : 26/09/91 - clear the command line
*/
  cmdmsg (debug, "") ;

  debugf("startup done");
}

PUBLIC void interp(DEBUG *debug)
{
  char *cmd;
#ifdef CR
	printf("debug 1 = %#x \n",debug);
#endif

  debugf("interp()");
  if ((debug->interp = newinterp(debug)) == NULL)
  {
    cmdmsg(debug, "No memory");
    return;	
  }
  
  startup(debug);
  
#ifdef CR
  printf("debug 2 = %#x \n",debug);
#endif

  until ((cmd = getline(debug->line)) == NULL)
    {
      if (cmdexec(debug, cmd) == TopLevel) break;
    }

  debugf("interp process terminating");

  return;  
}

/**
*
* level = cmdexec(debug, cmd);
*
* Execute a command.
*
**/
PUBLIC int cmdexec(DEBUG *debug, char *cmd)
{
  int level;
  jmp_buf savehome;
#ifdef CR
	printf("debug 10 = %#x \n",debug);
#endif

  lockinterp(debug->interp);
  memcpy(savehome, debug->interp->home, sizeof(jmp_buf));
#ifdef CR
	printf("debug 11 = %#x \n",debug);
#endif
  if ((level = setjmp(debug->interp->home)) == 0) _do(debug, cmd);
#ifdef CR
	printf("debug 12 = %#x \n",debug);
#endif
  memcpy(debug->interp->home, savehome, sizeof(jmp_buf));
  unlockinterp(debug->interp);
  return level;
}

/**
*
* cmderr(debug, format, ...);
*
* Display a command error and recover.
*
**/
PUBLIC void cmderr(DEBUG *debug, char *format, ...)
{
  DISPLAY *display = debug->display;
  va_list args;

  va_start(args, format);
  dlock(display);
  dcursor(display, display->height - 1, 0);
  deol(display);
  vfprintf(display->fileout, format, args);
  dunlock(display);
  va_end(args);
  longjmp(debug->interp->home, ErrorLevel);
}

/**
*
* cmdmsg(debug, format, ...);
*
* Display a command error and recover.
*
**/
PUBLIC void cmdmsg(DEBUG *debug, char *format, ...)
{
  DISPLAY *display = debug->display;
  va_list args;

  va_start(args, format);
  dlock(display);
  dcursor(display, display->height - 1, 0);
  deol(display);
  vfprintf(display->fileout, format, args);
  dunlock(display);
  va_end(args);
}

/**
*
* cmdjmp(debug, level);
*
* Perform a long jump.
*
**/
PUBLIC void cmdjmp(DEBUG *debug, int level)
{
  longjmp(debug->interp->home, level);
}
