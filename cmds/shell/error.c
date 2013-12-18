/**
*
* Title:  Helios Shell - Error Handling.
*
* Author: Andy England
*
* Date:   May 1988
*
*	  (c) Copyright 1988, Perihelion Software Ltd.
*
*	  All Rights Reserved.
*
* $Header: /users/nickc/RTNucleus/cmds/shell/RCS/error.c,v 1.11 1993/08/12 15:55:45 nickc Exp $
*
**/
#include "shell.h"

#define MAX_ERRMESS 64
char *errormessages[MAX_ERRMESS] =
{
  "Too few arguments",			/* ERR_TOOFEWARGS	0	*/
  "Too many arguments",			/* ERR_TOOMANYARGS	1	*/
  "Undefined variable",			/* ERR_VARIABLE 	2	*/
  "Badly formed number",		/* ERR_BADNUMBER	3	*/
  "Command not found",			/* ERR_NOTFOUND 	4	*/
  "Variable syntax",			/* ERR_VARSYNTAX	5	*/
  "Subscript out of range",		/* ERR_SUBSCRIPT	6	*/
  "Event not found",			/* ERR_EVENT		7	*/
  "Not in while/foreach",		/* ERR_NOTINLOOP	8	*/
  "Improper then",			/* ERR_THEN		9	*/
  "Empty if",				/* ERR_EMPTYIF		10	*/
  "Expression syntax",			/* ERR_EXPSYNTAX	11	*/
  "Words not ()'ed",			/* ERR_WORDLIST 	12	*/
  "Too many ('s",			/* ERR_LPAREN		13	*/
  "Too many )'s",			/* ERR_RPAREN		14	*/
  "Invalid variable",			/* ERR_INVALIDVAR	15	*/
  "Syntax error",			/* ERR_SYNTAX		16	*/
  "Too dangerous to alias that",	/* ERR_DANGEROUS	17	*/
  "endif not found",			/* ERR_NOENDIF		18	*/
  "else/endif not found",		/* ERR_NOELSE		19	*/
  "Word too long",			/* ERR_WORDTOOLONG	20	*/
  "Badly placed ()'s",			/* ERR_BADPARENS	21	*/
  "Missing name for redirect",		/* ERR_REDIRECT 	22	*/
  "Ambiguous input redirect",		/* ERR_INPUT		23	*/
  "Ambiguous output redirect",		/* ERR_OUTPUT		24	*/
  "No such limit",			/* ERR_LIMIT		25	*/
  "Improper or unknown scale factor",	/* ERR_SCALEFACTOR	26	*/
  "Invalid null command",		/* ERR_NULLCMD		27	*/
  "No more words",			/* ERR_NOMOREWORDS	28	*/
  "Improper mask",			/* ERR_MASK		29	*/
  "Alias loop",				/* ERR_ALIASLOOP	30	*/
  "Unmatched '",			/* ERR_SQUOTE		31	*/
  "Unmatched `",			/* ERR_BQUOTE		32	*/
  "Unmatched \"",			/* ERR_DQUOTE		33	*/
  "Use \"logout\" to logout",		/* ERR_USELOGOUT	34	*/
  "No file for $0",			/* ERR_NOFILENAME	35	*/
  "No match",				/* ERR_NOMATCH		36	*/
  "endsw not found",			/* ERR_NOENDSW		37	*/
  "Label not found",			/* ERR_NOLABEL		38	*/
  "auto-logout",			/* ERR_AUTOLOGOUT	39	*/
  "Not login shell",			/* ERR_NOTLOGIN 	40	*/
  "No home directory",			/* ERR_NOHOME		41	*/
  "end not found",			/* ERR_NOEND		42	*/
  "Use \"exit\" to leave shell",	/* ERR_USEEXIT		43	*/
  "Directory stack empty",		/* ERR_STACKEMPTY	44	*/
  "Bad directory",			/* ERR_BADDIR		45	*/
  "Directory stack not that deep",	/* ERR_NOTTHATDEEP	46	*/
  "No other directory",			/* ERR_NOOTHERDIR	47	*/
  "Unknown user",			/* ERR_UNKNOWNUSER	48	*/
  "So far unimplemented",		/* ERR_NOTINCLUDED	49	*/
  "<< terminator not found",		/* ERR_TERMINATOR	50	*/
  "No more memory",			/* ERR_NOMEMORY 	51	*/
  "Ambiguous",				/* ERR_AMBIGUOUS	52	*/
  "Modifier failed",			/* ERR_MODIFIER 	53	*/
  "Bad substitute",			/* ERR_BADSUB		54	*/
  "No prev lhs",			/* ERR_NOPREVLHS	55	*/
  "Bad Auxiliary",			/* ERR_BADAUX		56	*/
  "No current job",			/* ERR_NOCURJOB 	57	*/
  "No previous job",			/* ERR_NOPREVJOB	58	*/
  "No such job",			/* ERR_NOSUCHJOB	59	*/
  "Can't from terminal",		/* ERR_TERMINAL 	60	*/
  "Ambiguous error redirect",		/* ERR_DIAGNOSTIC	61	*/
  "Unknown option",			/* ERR_BADOPTION	62	*/
  "Unknown signal"			/* ERR_BADSIGNAL	63	*/
};

int syserr(char *name) 
{
  char *message;

#ifdef HELIOS
  set("error", nummakeargv(oserr));
#endif
  if (errno > 0 AND errno <= sys_nerr) message = sys_errlist[errno];
  else message = "Unknown error";
#ifdef DEBUGGING
  DEBUG("syserr %s: %s", name, message);
#endif
  if (name == NULL) fprintf(stderr, "%s.\n", message);
  else fprintf(stderr, "%s: %s.\n", name, message);
  return 1;
}

int error(
	  int code,
	  char *name )
{
  char *message = errormessages[code];

#ifdef DEBUGGING
  DEBUG("error %s: %s", name, message);
#endif
  if (name == NULL) fprintf(stderr, "%s.\n", message);
  else fprintf(stderr, "%s: %s.\n", name, message);
  return 1;
}

void recover()
{

  unless (getpid() == shellpid)
  {
#ifdef DEBUGGING
    DEBUG("In child, pid = %d, Shellpid = %d\n", getpid(), shellpid);
#endif
    _exit(1);
  }
#ifdef DEBUGGING
  DEBUG("In shell, pid = %d, Shellpid = %d\n", getpid(), shellpid);
#endif


  freecmd(globcmd);
  globcmd = NULL;
  tidyupfiles(); /* ACE: Loop lists should hang off files */
  tidyuploops();
  tidyupparse();
  resetinput();
  if(fdssaved)
	{
	restorefds(sfds);
	fdssaved = FALSE;
	}
  if(exitflag)
	{
	fast = TRUE;
	logout(1);
	}
  setmode(MODE_HISTORY);
  unsetmode(MODE_END);
  unsetmode(MODE_BREAK);
  clearerr(stdin);
  clearerr(stdout);
  fflush(stdout);
  clearerr(stderr);
  fflush(stderr);
  breakflag = FALSE;
  backgroundtask = FALSE;
#ifdef HELIOS
  ctrlcbegin();
#endif
  throw(1);
}

void bug(char *message)
{
  fprintf(stderr, "Bug: %s.\n", message);
  logout(1);
}
