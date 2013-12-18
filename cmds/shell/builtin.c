/**
*Title:  Helios Shell - Built-in Command Support.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
#include "shell.h"
#include <termios.h>

#ifdef __TRAN
char *sccsid = "@(#)builtin.c	1.18\t12/4/89 Copyright (C) 1988, Perihelion Software Ltd.";
#endif
  
#ifdef		DEBUGGING
char *version = "1.24\t02/08/93.\tDebug Version    Copyright (C) 1987 - 1993, Perihelion Software Ltd.";
#else
char *version = "1.24\t02/08/93.\t    Copyright (C) 1987 - 1993, Perihelion Software Ltd.";
#endif


static int determine_signal(char *str);
static int mystrcmp(char *ms1, char *ms2);
static void show_signals(void);

BUILTIN  builtins[MAX_BUILT] =
{
  "alias", b_alias,
  "alloc", b_alloc,
  "bg", b_bg,
  "break", b_break,
  "breaksw", b_breaksw,
  "case", b_case,
  "cd", b_cd,
  "chdir", b_cd,
  "continue", b_continue,
  "default", b_default,
  "dirs", b_dirs,
  "echo", b_echo,
  "else", b_else,
  "end", b_end,
  "endif", b_endif,
  "endsw", b_endsw,
  "eval", b_eval,
  "exec", b_exec,
  "exit", b_exit,
#ifdef HELIOS
  "fault", b_fault,
#endif
  "fg", b_fg,
  "foreach", b_foreach,
  "glob", b_glob,
  "goto", b_goto,
  "hashstat", b_hashstat,
  "history", b_history,
  "if", b_if,
  "jobs", b_jobs,
  "kill", b_kill,
  "limit", b_limit,
/*  "login", b_login, - BLV 1.2 changes */
  "logout", b_logout,
  "nice", b_nice,
  "nohup", b_nohup,
  "notify", b_notify,
  "onintr", b_onintr,
  "popd", b_popd,
#ifdef HELIOS
  "printenv", b_printenv,
#endif
  "pushd", b_pushd,
#ifdef HELIOS
  "pwd", b_pwd,
#endif
  "rehash", b_rehash,
  "repeat", b_repeat,
  "set", b_set,
  "setenv", b_setenv,
  "shift", b_shift,
  "source", b_source,
  "stop", b_stop,
  "suspend", b_suspend,
  "switch", b_switch,
  "time", b_time,
  "umask", b_umask,
  "unalias", b_unalias,
  "unhash", b_unhash,
  "unlimit", b_unlimit,
  "unset", b_unset,
  "unsetenv", b_unsetenv,
  "version", b_version,
  "wait", b_wait,
  "which", b_which,
  "while", b_while,
  "@", b_at,
  0, 0
};

BUILTIN extrabuiltins[2] =
{
  ":", b_label,
  "%", b_job
};

int mode;
SUBSTATE wordstate;

char *getword(char *line )
{
  int c;

#ifdef	DEBUGGING
  DEBUG("getword()");
#endif
  wordstate = NEUTRAL;
  forever
  {
    c = *line++;
    switch (wordstate)
    {
      case NEUTRAL:
      switch (c)
      {
        case ';':
        strcpy(wordbuffer, ";");
#ifdef	DEBUGGING
        DEBUG("='%s' (%S)",wordbuffer,line);
#endif
        return line;

        case '&':
        if (*line == '&')
        {
          line++;
          strcpy(wordbuffer, "&&");
          return line;
        }
        strcpy(wordbuffer, "&");
        return line;
       
        case '|':
        if (*line == '|')
        {
          line++;
#ifdef CDL
          if (usingcdl AND *line == '|')
          {
            line++;
            strcpy(wordbuffer, "|||");
            return line;
          }
#endif
          strcpy(wordbuffer, "||");
          return line;
        }
#ifdef CDL
        if (usingcdl)
        {
          if (*line == '<')
          {
            line++;
            strcpy(wordbuffer, "|<");
            return line;
          }
          if (*line == '>')
          {
            line++;
            strcpy(wordbuffer, "|>");
            return line;
          }
        }
#endif
        strcpy(wordbuffer, "|");
        return line;

        case '<':
        if (*line == '<')
        {
          line++;
          strcpy(wordbuffer, "<<");
          return line;
        }
#ifdef CDL
        if (usingcdl)
        {
          if (*line == '>')
          {
            line++;
            strcpy(wordbuffer, "<>");
            return line;
          }
          if (*line == '|')
          {
            line++;
            strcpy(wordbuffer, "<|");
            return line;
          }
        }
#endif
        strcpy(wordbuffer, "<");
        return line;
       
        case '>':
                       	
        if (*line == '>')
        {
          line++;
          if (*line == '2')
          {
            line++;
            strcpy(wordbuffer, ">>2");
#ifdef	DEBUGGING
  	    DEBUG(" = '%s' (%s)",wordbuffer,line);
#endif
            return line;
          }
          strcpy(wordbuffer, ">>");
          return line;
        }
        if (*line == '2')
        {
          line++;
          strcpy(wordbuffer, ">2");
#ifdef	DEBUGGING
  	  DEBUG(" = '%s' (%s)",wordbuffer,line);
#endif
          return line;
        }
#ifdef CDL
        if (usingcdl AND *line == '|')
        {
          line++;
          strcpy(wordbuffer, ">|");
          return line;
        }
#endif
        strcpy(wordbuffer, ">");
        return line;

#ifdef CDL
        case '^':
        if (usingcdl AND *line == '^')
        {
          line++;
          strcpy(wordbuffer, "^^");
          return line;
        }
        line--;
        wordstate = INWORD;
        continue;
#endif

        case '(':
        strcpy(wordbuffer, "(");
        parencount++;
        return line;

        case ')':
        strcpy(wordbuffer, ")");
        parencount--;
        return line;

	case '@':			/* the @ symbol might refer to	*/
					/* the at shell function or is	*/
					/* part of an encoded capability*/
	{
  	  int i;
	  for (i = 0; i < 16; i++)
	    if (line[i] < 'a' || line[i] > 'p') break;

	  if (i == 16 && line[i] == '/')
	  {
	    line--;			/* seems to be a capability...	*/
	    wordstate = INWORD;		/* read it as a word.		*/
	    continue;
	  }
          strcpy(wordbuffer, "@");
#ifdef	DEBUGGING
  	  DEBUG(" = '%s' (%s)",wordbuffer,line);
#endif
          return line;
        }

        case '\n':
        case '\0':
        return NULL;

        case ' ':
        case '\t':
        continue;

        default:
        line--;
        wordstate = INWORD;
        continue;
      }

      case INWORD:
      switch (c)
      {
        case ';':
        case '&':
        case '|':
        case '<':
        case '>':
        case '(':
        case ')':
        case '\n':
        case '\0':
        case ' ':
        case '\t':
        ignore endword();
        wordstate = NEUTRAL;
        return line - 1;

#ifdef CDL
        case '^':
        if (usingcdl AND *line == '^')
        {
          ignore endword();
          wordstate = NEUTRAL;
          return line - 1;
        }
        addchar('^');
        continue;
#endif

        case '\\':
        if (*line == '\n')
        {
          line++;
          if (wordindex > 0) addchar(' ');
          else wordstate = NEUTRAL;
          continue;
        }
        addchar('\\');
        addchar(*line++);
        continue;

        case '$':
        addchar('$');
        if (*line == '\n') continue;
        if (*line == '{')
        {
          line++;
          addchar('{');
          if (*line == '\n') continue;
        }
	if(*line != ' ')
        addchar(*line++);
        continue;

        case '\'':
        addchar('\'');
        wordstate = INSQUOTE;
        continue;
        
        case '`':
        addchar('`');
        wordstate = INBQUOTE;
        continue;

        case '\"':
        addchar('"');
        wordstate = INDQUOTE;
        continue;

        case '#':
        unless (interactive)
        {
          do c = *line++; until (c == '\0' OR c == '\n');
          return NULL;
        }
        default:
        addchar(c);
        continue;
      }

      case INSQUOTE:
      case INBQUOTE:
      case INDQUOTE:
      switch (c)
      {
        case '\n':
        case '\0':
        ignore endword();
        return line - 1;

#if 0
        case '\\':
        addchar('\\');
        addchar(*line++);
        continue;
#endif

        case '\'':
        addchar('\'');
        if (wordstate == INSQUOTE) wordstate = INWORD;
        continue;

        case '`':
        addchar('`');
        if (wordstate == INBQUOTE) wordstate = INWORD;
        continue;

        case '"':
        addchar('"');
        if (wordstate == INDQUOTE) wordstate = INWORD;
        continue;

        default:
        addchar(c);
        continue;
      }
    }
  }
}

void getlastword(char *line )
{
  until ((line = getword(line)) == NULL);
}

BOOL isspecial(char *name)
{
  BUILTIN *builtin;

  unless ((builtin = findbuiltin(name)) == NULL)
  {
    int (*func)() = builtin->func;

    return (BOOL)
           (func == b_else  OR func == b_exit OR func == b_foreach OR
           func == b_if    OR func == b_set  OR func == b_switch  OR
           func == b_while OR func == b_at);
  }
  return FALSE;
}

BOOL needsfullsubs(int (*func)() )
{
  return (BOOL)
         (func == b_cd      OR func == b_echo   OR func == b_eval   OR
          func == b_foreach OR func == b_glob   OR func == b_pushd  OR
          func == b_repeat  OR func == b_source OR func == b_switch OR
          func == b_time    OR func == b_set );
}

BUILTIN *findbuiltin(char *name )
{
  BUILTIN *builtin;

  if (name[strlen(name) - 1] == ':') return &extrabuiltins[0];
  if (name[0] == '%') return &extrabuiltins[1];
  for (builtin = builtins; builtin->name; builtin++)
  {
    if (strequ(name, builtin->name)) return builtin;
  }
  return NULL;
}

int b_alias(
	    int argc,
	    char *argv[] )
{
  if (argc < 2) putalias();
  else
  {
    char *name = argv[1];

    if (argc == 2)
    {
      unless ((argv = findalias(name)) == NULL) putargv(argv, TRUE);
    }
    else
    {
      if (strequ(name, "alias") OR strequ(name, "unalias"))
        return error(ERR_DANGEROUS, name);
      alias(name, dupargv(argv + 2));
    }
  }
  return OK;
}

int b_alloc(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  putmem();
  return OK;
}

int b_bg(
	 int argc,
	 char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_break(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  unless (inloop()) return error(ERR_NOTINLOOP, argv[0]);
  setmode(MODE_BREAK);
  setmode(MODE_END);
  return OK;
}

int b_breaksw(
	      int argc,
	      char *argv[] )
{
  char line[LINE_MAX + 1];
  int nestcount = 1;

  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  do
  {
    unless (getline(line, TRUE)) return error(ERR_NOENDSW, argv[0]);
    unless (getword(line) == NULL)
    {
      if (strequ("switch", wordbuffer)) nestcount++;
      if (strequ("endsw", wordbuffer)) nestcount--;
    }
  } while (nestcount > 0);
  return OK;
}

int b_case(
	   int argc,
	   char *argv[] )
{
  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_cd(
	 int argc,
	 char *argv[] )
{
  char *name;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    if ((name = getvar("home")) == NULL) return error(ERR_NOHOME, NULL);
  }
  else name = argv[1];
  unless (changedir(name)) return syserr(name);
  return OK;
}

int b_continue(
	       int argc,
	       char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  unless (inloop()) return error(ERR_NOTINLOOP, argv[0]);
  setmode(MODE_END);
  return OK;
}

int b_default(
	      int argc,
	      char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_dirs(
	   int argc,
	   char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  unless (dirs()) return syserr(argv[0]);
  return OK;
}

int b_echo(
	   int argc,
	   char *argv[] )
{
  if (argc >= 1)
  {
    BOOL newline = TRUE;

    if (strequ(*++argv, "-n")) 
    {
      argv++;
      newline = FALSE;
    }
    putargv(argv, newline);
  }
  return OK;
}

int b_else(
	   int argc,
	   char *argv[] )
{
  int nestcount = 1;
  char line[LINE_MAX + 1];

  do
  {
    char *lineptr;

    unless (getline(line, TRUE)) return error(ERR_NOENDIF, argv[0]);
    unless ((lineptr = getword(line)) == NULL)
    {
      if (strequ("endif", wordbuffer)) nestcount--;
      if (strequ("if", wordbuffer))
      {
        getlastword(lineptr);
        if (strequ("then", wordbuffer)) nestcount++;
      }
    }
  } while (nestcount > 0);
  return OK;
  argc = argc;				/* keep the compiler happy...	*/
}

int b_end(
	  int argc,
	  char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  unless (inloop()) return error(ERR_NOTINLOOP, argv[0]);
  setmode(MODE_END);
  return OK;
}

int b_endif(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_endsw(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_eval(
	   int argc,
	   char *argv[] )
{
  char line[LINE_MAX + 1];

  sputargv(line, argv + 1, ' ');
  return runcmdline(line);
  argc = argc;				/* keep the compiler happy...	*/
}

int b_exec(
	   int argc,
	   char *argv[] )
{
  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  executecmd(argv[1], argv+1);
  return error(ERR_NOTFOUND, argv[1]);	/* CFL	Fix to bug #261		*/
  return OK;
}

int b_exit(
	   int argc,
	   char *argv[] )
{
  int code;

  if (argc > 1)
  {
    EXPR *expr;

    initexprparse(argv + 1);
    code = evaluate(expr = readexpr(0));
    freeexpr(expr);
  }
  else
  {
    char *status;

    if ((status = getvar("status")) == NULL) code = OK;
    else code = atoi(status);
  }
  freeargv(argv);
  logout(code);
  return OK;
}

#ifdef HELIOS
int b_fault(
	    int argc,
	    char *argv[] )
{
  int code;

  if (argc > 1)
  {
    char *cmd = *argv++;
    char *arg;

    until ((arg = *argv++) == NULL)
    {
      if (strlen(arg) < 8) arg = readdecimal(arg, &code);
      else arg = readhex(arg, &code);
      unless (*arg == '\0') return error(ERR_BADNUMBER, cmd);
      fault(code);
    }
  }
  else
  {
    char *error;

    if ((error = getvar("error")) == NULL) code = OK;
    else code = atoi(error);
    fault(code);
  }
  return OK;
}
#endif

int b_fg(
	 int argc,
	 char *argv[] )
{
  JOB *job;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    if ((job = currentjob()) == NULL) return error(ERR_NOCURJOB, argv[0]);
  }
  else
  {
    char *arg = argv[1];

    unless (*arg++ == '%') return error(ERR_NOSUCHJOB, argv[0]);
    if (arg[0] == '\0' OR
        ((arg[0] == '%' OR arg[0] == '+') AND arg[1] == '\0'))
    {
      if ((job = currentjob()) == NULL) return error(ERR_NOCURJOB, argv[0]);
    }
    else if (arg[0] == '-' AND arg[1] == '\0')
    {
      if ((job = previousjob()) == NULL) return error(ERR_NOPREVJOB, argv[0]);
    }
    else
    {
      int number = JOBS_MAX + 1;		/* illegal value */

      ignore readdecimal(arg, &number);
      if ((job = getjob(number)) == NULL) return error(ERR_NOSUCHJOB, argv[0]);
    }
  }
  putcmd(job->cmd);
  printf("\n");
  ignore setpgid(job->pid, tcgetpgrp(0));
  waitforcmd(job->pid);
  return OK;
}

int b_foreach(
	      int argc,
	      char *argv[] )
{
  char name[VARNAME_MAX + 1];
  int savemode = mode;
  int retval = OK;
  char *arg;
  int i;
  long start, end;

  if (argc < 4) return error(ERR_TOOFEWARGS, argv[0]);
  arg = readname(argv[1], name);
  unless (*arg == '\0') return error(ERR_INVALIDVAR, argv[0]);
  unless (strequ(argv[2], "(") AND strequ(argv[argc - 1], ")"))
    return error(ERR_WORDLIST, argv[0]);
  start = note();
  {
    int nestcount = 1;
    char line[LINE_MAX + 1];

    do
    {
      unless (getline(line, TRUE)) return error(ERR_NOEND, argv[0]);
      unless (getword(line) == NULL)
      {
        if (strequ("while", wordbuffer) OR
            strequ("foreach", wordbuffer)) nestcount++;
        if (strequ("end", wordbuffer)) nestcount--;
      }
    } while (nestcount > 0);
  }
  end = note();
  if (argc > 4)
  {
    newloop();
    unsetmode(MODE_HISTORY);

    for (i = 3; i < argc - 1; i++)
    {
      point(start);
      set(name, makeargv(argv[i]));
      do
      {
        unless (docmdline())
        {
          error(ERR_NOEND, argv[0]);
          retval = !OK;			
        }
      } until ((mode & MODE_END) || retval != OK);    
      unsetmode(MODE_END);
      if ((mode & MODE_BREAK) || retval != OK)		
      {
        unsetmode(MODE_BREAK);
        point(end);
        break;
      }
    }
    mode = savemode;
    oldloop();
    point(end);				/* CFL	Fix to bug #130		*/
  }
  return retval;			
}

int b_glob(
	   int argc,
	   char *argv[] )
{
  while (--argc)
  {
    printf("%s", *++argv);
    if (argc > 1) putchar('\0');
    fflush(stdout);
  }
  return OK;
}

int b_goto(
	   int argc,
	   char *argv[] )
{
  char line[WORD_MAX + 1];
  char label[WORD_MAX + 1];

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  rewindinput();
  strcpy(label, argv[1]);
  strcat(label, ":");
  forever
  {
    unless (getline(line, TRUE)) return error(ERR_NOLABEL, argv[1]);
    unless (getword(line) == NULL)
    {
      if (strequ(label, wordbuffer)) break;
    }
  }
  return OK;
}

int b_hashstat(
	       int argc,
	       char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_history(
	      int argc,
	      char *argv[] )
{
  char *history;
  char *cmd = *argv++;
  BOOL reverse = FALSE;
  BOOL nonames = FALSE;
  int number = 0;
  char *arg;

#ifdef	DEBUGGING
  DEBUG("b_history (%V)", argv);
#endif
  unless ((history = getvar("history")) == NULL)
    number = atoi(history);

  while ((arg = *argv) != NULL AND *arg++ == '-')
  {
    argv++;
    while (*arg != '\0')
      switch (*arg++)
      {
        case 'h' : nonames = TRUE; break;
        case 'r' : reverse = TRUE; break;
        default  : return error(ERR_BADOPTION, cmd);
      }
  }

  if ((arg = *argv) != NULL)
  {
    arg = readdecimal(arg, &number);
    if (*arg != '\0') return error(ERR_BADNUMBER, cmd);
  }
  putsublist(&historylist, number, nonames, TRUE, reverse);

  return OK;
  argc = argc;				/* keep the compiler happy...	*/
}

int b_if(
	 int argc,
	 char *argv[] )
{
  EXPR *expr;
  int value;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  initexprparse(argv + 1);
  value = evaluate(expr = readexpr(0));
  freeexpr(expr);
  if (token == T_NEWLINE) return error(ERR_EMPTYIF, argv[0]);
  if (strequ(currentword, "then"))
  {
    readexprtoken();
    unless (token == T_NEWLINE) return error(ERR_THEN, argv[0]);
    unless (value)
    {
      int nestcount = 1;
      char line[LINE_MAX + 1];

      do
      {
        char *lineptr;

        unless (getline(line, TRUE)) return error(ERR_NOELSE, "then");
        unless ((lineptr = getword(line)) == NULL)
        {
          if (nestcount == 1 AND strequ("else", wordbuffer))
          {
            char **newargv = nullargv();

            until ((lineptr = getword(lineptr)) == NULL)
              newargv = addword(newargv, wordbuffer);
            ignore waitforcmd(runsimplecmd(newargv, NULL, READ, WRITE, CHECKBUILTIN));
            break;
          }
          if (strequ("endif", wordbuffer)) nestcount--;
          if (strequ("if", wordbuffer))
          {
            getlastword(lineptr);
            if (strequ("then", wordbuffer)) nestcount++;
          }
        }
      } while (nestcount > 0);
    }
  }
  else if (value)
    ignore waitforcmd(runsimplecmd(dupargv(wordlist), NULL, READ, WRITE, CHECKBUILTIN));
  return OK;
}

int b_job(
	  int argc,
	  char *argv[] )
{
  JOB *job;
  char *arg = argv[0] + 1;

  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  if (arg[0] == '\0' OR
      ((arg[0] == '%' OR arg[0] == '+') AND arg[1] == '\0'))
  {
    if ((job = currentjob()) == NULL) return error(ERR_NOCURJOB, argv[0]);
  }
  else if (arg[0] == '-' AND arg[1] == '\0')
  {
    if ((job = previousjob()) == NULL) return error(ERR_NOPREVJOB, argv[0]);
  }
  else
  {
    int number = JOBS_MAX + 1;		/* illegal value */

    ignore readdecimal(arg, &number);
    if ((job = getjob(number)) == NULL) return error(ERR_NOSUCHJOB, argv[0]);
  }
  putcmd(job->cmd);
  printf("\n");
  waitforcmd(job->pid);
  return OK;
}

int b_jobs(
	   int argc,
	   char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  putjobtable();
  return OK;
}

int b_kill(
	   int argc,
	   char *argv[] )
{
  char *arg, *name = argv[0];
  int sig = SIGTERM;
  BOOL rungeneralkill = FALSE;
  char **newargv = nullargv();

  newargv = addword(newargv, argv[0]);

  if (argc < 2) 
	{
	  freeargv(newargv);
	  return error(ERR_TOOFEWARGS, name);
	}
  until ((arg = *++argv) == NULL)
  {
    int pid;

    if (arg[0] == '-')
	{
	if (arg[1] == 'l' || arg[1] == 'L')
		{
		if(argc != 2)
		  {
		  freeargv(newargv);
	      	  return error(ERR_TOOMANYARGS, name);
		  }
		show_signals();
		freeargv(newargv);
	  	return OK;
		}

	if (arg[1] == 'a' || arg[1] == 'A')
		{
			/* execute general kill command */
                newargv = addword(newargv, arg);
		rungeneralkill = TRUE;
		continue;
		}

        if (argc < 3) 
	  {
	  freeargv(newargv);
	  return error(ERR_TOOFEWARGS, name);
	  }

	if(argv[1][0] == '-')
		{
		freeargv(newargv);
		return error(ERR_SYNTAX, name);
		}

	if((sig = determine_signal( &arg[1] )) < 0)
		{	
	 	freeargv(newargv);
	  	return error(ERR_BADSIGNAL, name);
		}

	newargv = addword(newargv, arg);

	continue;
	}

    if (arg[0] == '%')
    {
      JOB *job;

      if (arg[1] == '\0' OR
          ((arg[1] == '%' OR arg[1] == '+') AND arg[2] == '\0'))
      {
        if ((job = currentjob()) == NULL) 
	  {
	  freeargv(newargv);
	  return error(ERR_NOCURJOB, *argv);
	  }
      }
      else if (arg[1] == '-' AND arg[2] == '\0')
      {
        if ((job = previousjob()) == NULL) 
	  {
	  freeargv(newargv);
	  return error(ERR_NOPREVJOB, *argv);
	  }
      }
      else
      {
        int number = JOBS_MAX + 1;		/* illegal value */

        ignore readdecimal(&arg[1], &number);
        if ((job = getjob(number)) == NULL) 
	  {
	  freeargv(newargv);
	  return error(ERR_NOSUCHJOB, *argv);
	  }
      }
      pid = job->pid;
    }
    else
    {
      arg = readdecimal(arg, &pid);
      unless (*arg == '\0')
		{
			/* execute general kill command */
                newargv = addword(newargv, arg);
		rungeneralkill = TRUE;
                continue;
		}
    }
    if (kill(pid, sig) == -1)
	{
	freeargv(newargv);
	return syserr(NULL);
	}
  }
  if(rungeneralkill)
     return(waitforcmd(runsimplecmd(newargv, NULL, READ, WRITE, NOBUILTIN)));
  freeargv(newargv);
  return OK;
}

int b_label(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return OK;
}

int b_limit(
	    int argc,
	    char *argv[] )
{
#ifdef UNIX
  int i;

  if (argc > 3) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    for (i = 1; i < RESOURCE_MAX; i++) putlimit(i);
  }
  else
  {
    if ((i  = findresource(argv[1])) == -1) return error(ERR_LIMIT, argv[0]);
    if (argc == 2) putlimit(i);
    else
    {
      int limit = JOBS_MAX + 1;		/* illegal value */
      char *arg = readdecimal(argv[2], &limit);

      unless (*arg == '\0') return error(ERR_SCALEFACTOR, argv[0]);
      setlimit(i, limit);
    }
  }
  return OK;
#else
  if (argc > 1) error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
#endif
}

int b_logout(
	     int argc,
	     char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  if (login)
  {
    freeargv(argv);
    logout(OK);
  }
  return error(ERR_NOTLOGIN, NULL);
}

int b_nice(
	   int argc,
	   char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_nohup(
	    int argc,
	    char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_notify(
	     int argc,
	     char *argv[] )
{
  JOB *job;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    if ((job = currentjob()) == NULL) return error(ERR_NOCURJOB, argv[0]);
  }
  else
  {
    char *arg = argv[1];

    unless (*arg++ == '%') return error(ERR_NOSUCHJOB, argv[0]);
    if (arg[0] == '\0' OR
        ((arg[0] == '%' OR arg[0] == '+') AND arg[1] == '\0'))
    {
      if ((job = currentjob()) == NULL) return error(ERR_NOCURJOB, argv[0]);
    }
    else if (arg[0] == '-' AND arg[1] == '\0')
    {
      if ((job = previousjob()) == NULL) return error(ERR_NOPREVJOB, argv[0]);
    }
    else
    {
      int number = JOBS_MAX + 1;		/* illegal value */

      ignore readdecimal(arg, &number);
      if ((job = getjob(number)) == NULL) return error(ERR_NOSUCHJOB, argv[0]);
    }
  }
  job->notify = TRUE;
  return OK;
}

int b_onintr(
	     int argc,
	     char *argv[] )
{
  if (interactive) return error(ERR_TERMINAL, argv[0]);
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_popd(
	   int argc,
	   char *argv[] )
{
  char path[PATH_MAX + 1];

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    unless (popdir(path)) return error(ERR_STACKEMPTY, argv[0]);
    unless (changedir(path)) return syserr(path);
    dirs();
    return OK;
  }
  else if (argv[1][0] == '+')
  {
    int number = JOBS_MAX + 1;		/* illegal value */

    char *arg = readdecimal(argv[1] + 1, &number);

    if (*arg == '\0')
    {
      if(!number)
	{
        unless (popdir(path)) return error(ERR_STACKEMPTY, argv[0]);
	}
      else
	{
        unless (getdir(number-1, path)) return error(ERR_NOTTHATDEEP, argv[0]);
	}
      unless (changedir(path)) return syserr(path);
      return OK;
    }
  }
  return error(ERR_BADDIR, argv[0]);
}

#ifdef HELIOS
int b_printenv(
	       int argc,
	       char *argv[] )
{
  char **envp = environ;
  char *env;

  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  until ((env = *envp++) == NULL) printf("%s\n", env);
  return OK;
}
#endif

int b_pushd(
	    int argc,
	    char *argv[] )
{
  char path[PATH_MAX + 1];

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    unless (popdir(path)) return error(ERR_NOOTHERDIR, argv[0]);
    unless (pushdir(path)) return syserr(path);
    return OK;
  }
  else if (argv[1][0] == '+')
  {
    int number = JOBS_MAX + 1;		/* illegal value */
    char *arg = readdecimal(argv[1] + 1, &number);

    if (*arg == '\0')
    {
      if(!number)
	 {
         unless (popdir(path)) return error(ERR_NOOTHERDIR, argv[0]);
	 }
      else
	 {
         unless (getdir(number-1, path)) return error(ERR_NOTTHATDEEP, argv[0]);
	 }
      unless (pushdir(path)) return syserr(path);
      return OK;
    }
  }
  unless (pushdir(argv[1])) return syserr(argv[1]);
  return OK;
}

#ifdef HELIOS
int b_pwd(
	  int argc,
	  char *argv[] )
{
  char path[PATH_MAX + 1];

  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  if (getcwd(path, PATH_MAX) == NULL) return syserr(argv[0]);
  printf("%s\n", path);
  return OK;
}
#endif

int b_rehash(
	     int argc,
	     char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  hash();
  return OK;
}

int b_repeat(
	     int argc,
	     char *argv[] )
{
  int count;
  char *arg;

  if (argc < 3) return error(ERR_TOOFEWARGS, argv[0]);
  arg = readdecimal(argv[1], &count);
  unless (*arg == '\0') return error(ERR_BADNUMBER, argv[0]);
  while (count--)
  {
    if (testbreak()) return !OK;			
    ignore waitforcmd(runsimplecmd(dupargv(argv + 2), NULL, READ, WRITE, CHECKBUILTIN));
  }
  return OK;
}

int b_set(
	  int argc,
	  char *argv[] )
{
#ifdef	DEBUGGING
  DEBUG("bset(%s)",argv[1]);
#endif

  if (argc < 2) putvars();
  else
  {
    char *cmd = *argv++;
    char *arg;

    until ((arg = *argv++) == NULL)
    {
      char name[VARNAME_MAX + 1];
      char *text;
      char **newargv;
      int index;

      arg = readname(arg, name);
      
      if (name[0] == '\0')
	{
	  fprintf( stderr, "set: unable to read name of variable to set\n" );
	  
	  return error(ERR_SYNTAX, cmd);
	}
      
      switch (*arg++)
      {
        case '\0':
        if (strequ(*argv, "="))
        {
          argv++;
          if (strequ(*argv, "("))
          {
            newargv = nullargv();

            argv++;
            until (strequ(arg = *argv++, ")")) newargv = addword(newargv, arg);
             set(name, newargv); 
          }
          else set(name, makeargv(*argv++));
        }
        else set(name, makeargv(""));
        break;
 
        case '[':
        arg = readdecimal(arg, &index);
        unless (*arg++ == ']')
	  {
	    fprintf( stderr, "set: no closing ']' to match '['\n" );
	    return error(ERR_SYNTAX, cmd);
	  }
	
        if (*arg == '\0')
        {
          if (strequ(*argv, "="))
          {
            argv++;
            text = *argv++;
          }
          else text = "";
        }
        else
        {
          unless (*arg++ == '=')
	    {
	      fprintf( stderr, "no '=' following '[...]'\n" );
	      
	      return error(ERR_SYNTAX, cmd);
	    }
          text = arg;
        }
        if ((newargv = findvar(name)) == NULL)
          return error(ERR_VARIABLE, name);
        unless (setword(name, newargv, index, text))
          return error(ERR_SUBSCRIPT, cmd);
        break;

        case '=':
        unless (*arg == '\0') set(name, makeargv(arg));
        else
        {
          if (strequ(*argv, "("))
          {
            newargv = nullargv();

            argv++;
            until (strequ(arg = *argv++, ")")) newargv = addword(newargv, arg);
            set(name, newargv); 
          }
          else set(name, makeargv(""));
        }
        break;

        default:
	fprintf( stderr, "unknown character in set string: '%c'\n", *(arg - 1) );
        return error(ERR_SYNTAX, cmd);
      }
    } 
  }
  return OK;
}

int b_setenv(
	     int argc,
	     char *argv[] )
{
  if (argc < 3) return error(ERR_TOOFEWARGS, argv[0]);
  if (argc > 3) return error(ERR_TOOMANYARGS, argv[0]);
  setenv(argv[1], argv[2]);
  return OK;
}

int b_shift(
	    int argc,
	    char *argv[] )
{
  char *name;
  char **oldargv;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc == 1) name = "argv";
  else name = argv[1];
  unless ((oldargv = findvar(name)) == NULL)
  {
    unless (lenargv(oldargv)) return error(ERR_NOMOREWORDS, argv[0]);
    set(name, dupargv(oldargv + 1));
    return OK;
  }
  return error(ERR_VARIABLE, name);
}

int b_source(
	     int argc,
	     char *argv[] )
{
  int savemode = mode;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  if (argc > 3) return error(ERR_TOOMANYARGS, argv[0]);
  if (strequ("-h", argv[1]))
  {
  	if (argc < 3) return error(ERR_TOOFEWARGS, argv[0]); /*jd*/
    if (record(argv[2])) return OK;
    return syserr(argv[2]);
  }
  setmode(MODE_EXECUTE);
  unsetmode(MODE_HISTORY);

  if (source(argv[1]))
  {
    mode = savemode;
    return OK;
  }
  mode = savemode;
  return syserr(argv[1]);
}

int b_stop(
	   int argc,
	   char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_suspend(
	      int argc,
	      char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
}

int b_switch(
	     int argc,
	     char *argv[] )
{
  char line[LINE_MAX + 1];
  char *str;
  int nestcount = 1;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  if (argc < 4) return error(ERR_SYNTAX, argv[0]);
  if (argc > 4) return error(ERR_SYNTAX, argv[0]);
  initexprparse(argv + 1);
  unless (token == T_LPAREN) return error(ERR_SYNTAX, argv[0]);
  readexprtoken();
  str = currentword;
  readexprtoken();
  unless (token == T_RPAREN) return error(ERR_SYNTAX, argv[0]);
  do
  {
    char *lineptr;

    unless (getline(line, TRUE)) break;
    unless ((lineptr = getword(line)) == NULL)
    {
      if (strequ("switch", wordbuffer)) nestcount++;
      if (strequ("endsw", wordbuffer)) nestcount--;
      if (nestcount == 1)
      {
        if (strequ("default", wordbuffer)) break;
        if (strequ("default:", wordbuffer)) break;
        if (strequ("case", wordbuffer))
        {
          int length;

          unless (getword(lineptr) == NULL)
          {
            if (((length = strlen(wordbuffer)) > 0) AND
                (wordbuffer[length - 1] == ':')) wordbuffer[length - 1] = '\0';
            if (match(str, wordbuffer)) break;
          }
        }
      }
    }
  } while (nestcount > 0);
  return OK;
}

int b_time(
	   int argc,
	   char *argv[] )
{
#ifdef NEWCODE
  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {

  }
  else
  {
    waitforcmd(runsimplecmd(dupargv(argv + 1), NULL, READ, WRITE, CHECKBUILTIN));
  }
  return OK;
#else
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
#endif
}

int b_umask(
	    int argc,
	    char *argv[] )
{
#ifdef UNIX
  char *arg;
  int mask;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    mask = umask(0);
    umask(mask);
    printf("%o\n", mask);
    return OK;
  }
  arg = readoctal(argv[1], &mask);
  unless (*arg == '\0') return error(ERR_MASK, argv[0]);
  umask(mask);
  return OK;
#else
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
#endif
}

int b_unalias(
	      int argc,
	      char *argv[] )
{
  char *arg;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  until ((arg = *argv++) == NULL) unalias(arg);
  return OK;
}

int b_unhash(
	     int argc,
	     char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  unhash();
  return OK;
}

int b_unlimit(
	      int argc,
	      char *argv[] )
{
#ifdef UNIX
  int i;

  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  if (argc < 2)
  {
    for (i = 0; i < RESOURCE_MAX; i++) setlimit(i, -1);
  }
  else
  {
    if ((i = findresource(argv[1])) == -1) return error(ERR_LIMIT, argv[0]);
    setlimit(i, -1);
  }
  return OK;
#else
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  return error(ERR_NOTINCLUDED, argv[0]);
#endif
}

int b_unset(
	    int argc,
	    char *argv[] )
{
  char *arg;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  until ((arg = *argv++) == NULL) unset(arg);
  return OK;
}

int b_unsetenv(
	       int argc,
	       char *argv[] )
{
  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  if (argc > 2) return error(ERR_TOOMANYARGS, argv[0]);
  delenv(argv[1]);
  return OK;
}

int b_version(
	      int argc,
	      char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  printf("%s\n", version);
  return OK;
}

int b_wait(
	   int argc,
	   char *argv[] )
{
  if (argc > 1) return error(ERR_TOOMANYARGS, argv[0]);
  waitforcmd(-1);
  return OK;
}

int b_while(
	    int argc,
	    char *argv[] )
{
  int savemode = mode;
  int retval = OK;
  EXPR *expr;
  int value;
  long position;
  long start, end;

  if (argc < 2) return error(ERR_TOOFEWARGS, argv[0]);
  initexprparse(argv + 1);
  value = evaluate(expr = readexpr(0));
  freeexpr(expr);
  unless (token == T_NEWLINE) return error(ERR_EXPSYNTAX, argv[0]);
  position = lineposition;
  start = note();
  {
    int nestcount = 1;
    char line[LINE_MAX + 1];

    do
    {
      unless (getline(line, TRUE)) return error(ERR_NOEND, argv[0]);
      unless (getword(line) == NULL)
      {
        if (strequ("while", wordbuffer) OR
            strequ("foreach", wordbuffer)) nestcount++;
        if (strequ("end", wordbuffer)) nestcount--;
      }
    } while (nestcount > 0);
  }
  end = note();

  if (value)
  {
    newloop();
    unsetmode(MODE_HISTORY);

    point(start);
    do
    {
      unless (docmdline())
      {
        error(ERR_NOEND, argv[0]);
        retval = !OK;			
      }
    } until ((mode & MODE_END) || retval != OK);    
    unsetmode(MODE_END);
    if ((mode & MODE_BREAK) || retval != OK) {
#if 0
      unsetmode(MODE_BREAK); /* unneeded due to savemode */
#endif
      point(end);
      mode = savemode;
      oldloop();
      if (!inloop())
	setmode(MODE_HISTORY);
    }
    else {
      point(position);
      mode = savemode;
      unsetmode(MODE_HISTORY);
      oldloop();
    }
  }
  else if (!inloop())
	setmode(MODE_HISTORY);

  return retval;
}

int b_at(
	 int argc,
	 char *argv[] )
{
  if (argc == 1) putvars();
  else
  {
    char *cmd = *argv++;
    char *arg;

    until ((arg = *argv) == NULL)
    {
      char name[VARNAME_MAX + 1];
      char buffer[NUMSTR_MAX + 1];
      EXPR *expr;
      char **newargv;
      char *text;
      int index = -1;

      arg = readname(arg, name);	/* read var name		*/
      if (name[0] == '\0') return error(ERR_SYNTAX, cmd);

      if (*arg == '[')			/* read index			*/
      {
        arg = readdecimal(arg + 1, &index);
        unless (*arg++ == ']') return error(ERR_SYNTAX, cmd);
      }

      if (*arg == '\0' && *++argv != NULL)	/* skip argument end	*/
        arg = *argv;

      if (*arg == '=')			/* '=' found, evaluate expr.	*/
      {
        if (*++arg == '\0')		/* end of argv element:		*/
        {
          if (*++argv != NULL)		/* take next element		*/
            arg = *argv;
          else 				/* or report error		*/
            return error(ERR_SYNTAX, cmd);
        }
        else    			/* expr starts in this element	*/
        {
          text = *argv;			/* copy it to the start		*/
          while ((*text++ = *arg++) != '\0');
        }
        initexprparse(argv);
        text = streval(buffer, expr = readexpr(0));
        freeexpr(expr);
        argv = wordlist;
      }
      else				/* no expression found		*/
      {
      	if (*arg == '\0')		/* nothing left in the element	*/
      	  argv++;
      	else				/* something more to process	*/
      	{
          text = *argv;			/* copy it to the start		*/
          while ((*text++ = *arg++) != '\0');
        }
      	text = "";			/* set empty value		*/
      }
      if (index < 0)
        set(name, makeargv(text));
      else
      {
        if ((newargv = findvar(name)) == NULL)
          return error(ERR_VARIABLE, name);
        unless (setword(name, newargv, index, text))
          return error(ERR_SUBSCRIPT, cmd);
      }
    }
  }
  return OK;
}

/**
*** A table of the known signals. This is used when examining the argument
*** specifying the signal number, and for the -l option.
**/
typedef struct signal_definition {
	char	*name;
	int	number;
} signal_definition;

static	signal_definition known_signals[] =
{	{ "ZERO",	 0	},
	{ "ABRT",	 1	},
	{ "FPE",	 2	},
	{ "ILL",	 3  	},
	{ "INT",	 4	},
	{ "SEGV",	 5	},
	{ "TERM",	 6	},
	{ "STAK",	 7	},
	{ "ALRM",	 8	},
	{ "HUP",	 9	},
	{ "PIPE",	10	},
	{ "QUIT",	11	},
	{ "TRAP",	12	},
	{ "USR1",	13	},
	{ "USR2",	14	},
	{ "CHLD",	15	},
	{ "URG",	16	},
	{ "CONT",	17	},
	{ "STOP",	18	},
	{ "TSTP",	19	},
	{ "TTIN",	20	},
	{ "TTOU",	21	},
	{ "WINCH",	22	},
	{ "SIB",	23	},
	{ "KILL",	31	},
	{ Null(char),	-1	}
};

/**
*** Work out the signal from the given argument. This signal may be:
***     1) a number between 0 and 31
***	2) a string corresponding to one of the known signals
**/
static	int	determine_signal(char *str)
{ int	result = 0;
  int	i;
  
  if (isdigit(*str))
   { char	*temp = str;
     while (isdigit(*temp))
      { result = (10 * result) + (*temp - '0');
        temp++;
      }
     if (*temp == '\0')
      { if ((result < 0) || (result > 31))
       	  return(-1);
      	return(result);
      }
   }

	/* The string is not a simple number */
  for (i = 0; known_signals[i].name != Null(char); i++)
   if (!mystrcmp(known_signals[i].name, str))
    return(known_signals[i].number);

  return(-1);    
}


/**
*** String comparison routine which is not case sensitive. It returns the
*** same result as strcmp, i.e. 0 for identical strings
**/
static int mystrcmp(char *ms1, char *ms2)
{ char *s1 = ms1;
  char *s2 = ms2; 
#define ToUpper(x) (islower(x) ? toupper(x) : x)
  
  for (;;)
   { if (*s1 == '\0')
       return((*s2 == '\0') ? 0 : -1);
     elif (*s2 == '\0')
       return(1);
     elif(ToUpper(*s1) < ToUpper(*s2))
       return(-1);
     elif(ToUpper(*s1) > ToUpper(*s2))
       return(1);
     else
       { s1++; s2++; }
   }
}


/**
*** This routine shows the signals known to the kill command. To produce
*** a reasonable format it assumes 80 columns and four characters per
*** signal.
**/
static	void	show_signals(void)
{ int	i, j;

  for (i = 0, j = 0; known_signals[i].name != Null(char); i++)
   { fputs(known_signals[i].name, stderr);
     fputc(' ', stderr);
     if (++j == 16)
      { fputc('\n', stderr);
        j = 0;
      }
   }
  unless (j == 0) fputc('\n', stderr);
}

/* internal which command (looks at alias list) */

int b_which(
	    int argc,
	    char *argv[] )
{
  char *arg, *name = argv[0];
  BOOL rungeneralwhich = FALSE;
  char **newargv = nullargv();
  char **aliasargv;

  newargv = addword(newargv, argv[0]);

  if (argc < 2) 
	{
	  freeargv(newargv);
	  return error(ERR_TOOFEWARGS, name);
	}
  until ((arg = *++argv) == NULL)
  {
	if((aliasargv = findalias(arg)) != NULL)
		{
		fprintf(stdout,"%s:	 aliased to ", arg);
		putargv(aliasargv, TRUE);
		}
	else
		{	
		newargv = addword(newargv, arg);
		rungeneralwhich = TRUE;
		}
  }
  if(rungeneralwhich)
     return(waitforcmd(runsimplecmd(newargv, NULL, READ, WRITE, NOBUILTIN)));
  freeargv(newargv);
  return OK;
}
