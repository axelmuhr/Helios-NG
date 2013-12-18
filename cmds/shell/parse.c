/**
*
* Title:  Helios Shell - Parsing.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/parse.c,v 1.7 1994/02/07 11:53:11 nickc Exp $
*
**/
#include "shell.h"

void parseerror(
		int code,
		char *name )
{
  unless (parsingerror)
  {
    parsingerror = TRUE;
    ignore error(code, name);
  }
}

EXPR *newexpr(
	      TOKEN op,
	      EXPR *left,
	      EXPR *right )
{
  EXPR *expr = new(EXPR);

  expr->op = op;
  expr->left = left;
  expr->right = right;
  return expr;
}

CMD *newsimple(
	       ARGV argv,
	       IOINFO *ioinfo )
{
  CMD *cmd = new(CMD);

  cmd->op = T_SIMPLE;
  cmd->This = (CMD *)argv;
  cmd->next = NULL;
  cmd->ioinfo = ioinfo;
  return cmd;
}

CMD *newcmd(
	    TOKEN op,
	    CMD *This,
	    CMD *next )
{
  CMD *cmd = new(CMD);

  cmd->op = op;
  cmd->This = This;
  cmd->next = next;
  cmd->ioinfo = NULL;
  return cmd;
}

IOINFO *newioinfo(
		  OPENINFO * input,
		  OPENINFO * output,
		  OPENINFO * diag )
{
  IOINFO *ioinfo = new(IOINFO);

  ioinfo->input = input;
  ioinfo->output = output;
  ioinfo->diag = diag;
  return ioinfo;
}

OPENINFO *newopeninfo(
		      TOKEN op,
		      char *name,
		      long flags )
{
  OPENINFO *openinfo = new(OPENINFO);

  openinfo->op = op;
  openinfo->name = strdup(name);
  openinfo->flags = flags;
  return openinfo;
}

struct
{
  char *symbol;
  TOKEN token;
} exprtokentable[25] =
{
  "||", T_OR,
  "&&", T_AND,
  "|",  T_BITOR,
  "^",  T_BITXOR,
  "&",  T_BITAND,
  "==", T_EQ,
  "!=", T_NE,
  "=~", T_MATCH,
  "!~", T_NOMATCH,
  "<=", T_LE,
  ">=", T_GE,
  "<",  T_LT,
  ">",  T_GT,
  "<<", T_LSHIFT,
  ">>", T_RSHIFT,
  "+",  T_PLUS,
  "-",  T_MINUS,
  "*",  T_TIMES,
  "/",  T_DIVIDE,
  "%",  T_REM,
  "!",  T_NOT,
  "~",  T_ONECOMP,
  "(",  T_LPAREN,
  ")",  T_RPAREN,
  NULL
};

#define TOKENCOUNT 14

struct
{
  char *symbol;
  TOKEN token;
} tokentable[TOKENCOUNT] =
{
  ";",   T_SEMICOLON,
  "&",   T_AMPERSAND,
  "&&",  T_AND,
  "|",   T_PIPE,
  "||",  T_OR,
  "<",   T_READ,
  ">",   T_WRITE,
  "<<",  T_SHELLREAD,
  ">>",  T_APPEND,
  "(",   T_LPAREN,
  ")",   T_RPAREN,
  ">2",  T_WRITEDIAG,			/* CFL	additions for		*/
  ">>2", T_APPENDDIAG,			/*	stderr redirection	*/
  NULL
  
};

#ifdef CDL
struct
{
  char *symbol;
  TOKEN token;
} cdltokentable[12] =
{
  "|||", T_FARM,
  "|<",  T_REVPIPE,
  "|>",  T_PIPE,
  "<|",  T_READFIFO,
  ">|",  T_WRITEFIFO,
  "<>",  T_SUBORDINATE,
  "^",   T_PAR,
  "^^",  T_PAR,
  ",",   T_COMMA,
  "[",   T_LBRACKET,
  "]",   T_RBRACKET,
  NULL
};
#endif

int istoken(char *buffer)
{
	int i = TOKENCOUNT-1, len;
	char *s;

	while(--i)
		{
		s = tokentable[i].symbol;
		if(! strncmp(s, buffer, (len = strlen(s))))
			return len;
		}

	return 0;
}

int removetoken(char *buffer)
{
	int len;
	if((len = istoken(buffer)) != 0)
		strcpy(buffer, buffer+len);

	return strlen(buffer);
}


LIST expansionlist;

void newexpansion(
		  char *name,
		  ARGV argv )
{
  EXPANSION *expansion = new(EXPANSION);

#ifdef DEBUGGING
  DEBUG("newexpansion('%s')", name);
#endif
  expansion->wordlist = wordlist;
  expansion->lastword = lastword;
  expansion->name = name;
  expansion->argv = wordlist = buildargv(argv);
  lastword = FALSE;
#ifdef SYSDEB
  expansion->next = expansion->prev = expansion;
#endif
  AddHead(&expansionlist, (NODE *)expansion);
  settoken();
}

void oldexpansion()
{
  EXPANSION *expansion = (EXPANSION *)RemHead(&expansionlist);

#ifdef DEBUGGING
  DEBUG("oldexpansion()");
#endif
  wordlist = expansion->wordlist;
  lastword = expansion->lastword;
  freeargv(expansion->argv);
  freememory((int *)expansion);
}

BOOL processing(char *name)
{
  EXPANSION *expansion = (EXPANSION *)expansionlist.Head;

#ifdef DEBUGGING
  DEBUG("processing('%s')", name);
#endif
  if (expansion->next == NULL) return FALSE;
  return (BOOL) strequ(name, expansion->name);
}

BOOL expanding(char *name)
{
  EXPANSION *expansion;

#ifdef DEBUGGING
  DEBUG("expanding('%s')", name);
#endif
  for (expansion = (EXPANSION *)expansionlist.Head; expansion->next;
       expansion = expansion->next)
  {
    if (strequ(name, expansion->name)) return TRUE;
  }
  return FALSE;
}

void tidyupparse()
{
#ifdef DEBUGGING
  DEBUG("tidyupparse()");
#endif
  until (expansionlist.Head->Next == NULL) oldexpansion();
  freeargv(globargv);
  globargv = NULL;
  ignore endword();
  parencount = 0;
}

void readexprtoken()
{
  ++wordlist;
  setexprtoken();
}

void readtoken()
{
  forever
  {
    if (lastword)
    {
      lastword = FALSE;
      break;
    }
    unless (*++wordlist == NULL) break;
    if (expansionlist.Head->Next == NULL) break;
    oldexpansion();
  }
  settoken();
}

void setexprtoken()
{
  int i;
  char *nextword;

  if ((currentword = *wordlist) == NULL)
  {
    token = T_NEWLINE;
    return;
  }
  for (i = 0; exprtokentable[i].symbol; i++)
  {
    if (strequ(currentword, exprtokentable[i].symbol))
    {
      switch (token = exprtokentable[i].token)
      {
        case T_LT:
        if ((nextword = wordlist[1]) != NULL AND strequ(nextword, "="))
        {
          wordlist++;
          token = T_LE;
        }
        break;

        case T_GT:
        if ((nextword = wordlist[1]) != NULL AND strequ(nextword, "="))
        {
          wordlist++;
          token = T_GE;
        }
        break;
      }
      return;
    }
  }
  token = T_WORD;
}

void settoken()
{
  int i;

  if ((currentword = *wordlist) == NULL)
  {
    token = T_NEWLINE;
    return;
  }
  for (i = 0; tokentable[i].symbol; i++)
  {
    if (strequ(currentword, tokentable[i].symbol))
    {
      switch (token = tokentable[i].token)
      {
        case T_LPAREN:
        parencount++;
        break;

        case T_RPAREN:
        parencount--;
        break;
      }
      return;
    }
  }
#ifdef CDL
  if (usingcdl)
  {
    for (i = 0; cdltokentable[i].symbol; i++)
    {
      if (strequ(currentword, cdltokentable[i].symbol))
      {
        token = cdltokentable[i].token;
        return;
      }
    }
  }
#endif
  token = T_WORD;
}

void initexprparse(ARGV argv)
{
#ifdef DEBUGGING
  DEBUG("initexprparse()");
#endif
  parsingerror = FALSE;
  wordlist = argv;
  setexprtoken();
}

BOOL initparse(ARGV argv)
{
#ifdef DEBUGGING
  DEBUG("initparse()");
#endif
  if (argv[0] == NULL)
  {
    freeargv(argv);
    return FALSE;
  }
  globargv = argv;
  if (mode & MODE_HISTORY) addevent(dupargv(argv));
  if (historyused OR findvar("verbose")) putargv(argv, TRUE);
  unless (parencount == 0)
  {
    if (parencount > 0) error(ERR_LPAREN, NULL);
    else error(ERR_RPAREN, NULL);
    recover();
  }
  InitList(&expansionlist);
  parencount = 0;
  parsingerror = FALSE;
  wordlist = argv;
  settoken();
  return TRUE;
}

void unreadtoken()
{
  lastword = TRUE;
}

BOOL isconstructor(TOKEN token)
{
#ifdef CDL
  if (usingcdl)
    return token == T_PAR OR token == T_PIPE OR token == T_REVPIPE OR
           token == T_SUBORDINATE OR token == T_FARM;
#endif
  return (BOOL)(token == T_PIPE);
}

BOOL isdelimitor(TOKEN token)
{
  return (BOOL)
    (token == T_SEMICOLON OR token == T_AMPERSAND OR
     token == T_AND OR token == T_OR);
}

BOOL isterminator(TOKEN token)
{
  return (BOOL)(token == T_SEMICOLON OR token == T_AMPERSAND);
}

BOOL isredirection(TOKEN token)
{
  return (BOOL)
    (token == T_READ OR token == T_WRITE OR
#ifdef CDL
         token == T_READFIFO OR token == T_WRITEFIFO OR
#endif
	 token == T_WRITEDIAG OR token == T_APPENDDIAG OR	/* CFL	*/
	 token == T_APPEND OR token == T_SHELLREAD );
}

BOOL isinput(TOKEN token)
{
  return (BOOL)
    ( token == T_READ OR
#ifdef CDL
     token == T_READFIFO OR
#endif
     token == T_SHELLREAD );
}

BOOL isdiag( TOKEN token)	/* CFL	addition for		*/
				/*	stderr redirection	*/
{	
  return (BOOL)
    (token == T_WRITEDIAG OR
     token == T_APPENDDIAG );
}

int getpri(TOKEN delimitor)
{
  switch (delimitor)
  {
    case T_AMPERSAND:
    return 1;

    case T_SEMICOLON:
    return 2;

    case T_AND:
    return 3;

    case T_OR:
    return 3;

    default:
    bug("Unexpected delimitor");
    return 0;
  }
}

CMD *readcmd()
{
  CMD *cmd;

#ifdef DEBUGGING
  DEBUG("readcmd()");
#endif
  if (token == T_LPAREN)
  {
    readtoken();
    cmd = newcmd(T_LIST, readcmdlist(0), NULL);
    unless (checkfor(T_RPAREN))
    {
      parseerror(ERR_BADPARENS, NULL);
      return cmd;
    }
  }
  else if ((cmd = readsimplecmd()) == NULL) return NULL;
#ifdef CDL
  if (usingcdl AND token == T_LPAREN)
  {
    readtoken();
    cmd->next = readauxiliaries();
    unless (checkfor(T_RPAREN)) parseerror(ERR_BADPARENS, NULL);
  }
#endif
  return cmd;
}

CMD *readcmdlist(int rightpri)
{
  CMD *cmd;

#ifdef DEBUGGING
  DEBUG("readcmdlist()");
#endif
  cmd = readpipeline();
  forever
  {
    if (isdelimitor(token))
    {
      TOKEN delimitor = token;
      int leftpri = getpri(delimitor);

      if (leftpri < rightpri) return cmd;
      readtoken();
      if (token == T_NEWLINE AND isterminator(delimitor))
        return newcmd(delimitor, cmd, NULL);
      cmd = newcmd(delimitor, cmd, readcmdlist(leftpri));
    }
    else return cmd;
  }
}

CMD *readpipeline()
{
  CMD *cmd;

#ifdef DEBUGGING
  DEBUG("readpipeline()");
#endif
  cmd = readcmd();
  while (isredirection(token))
  {
    if ((cmd->ioinfo = readredirection(cmd->ioinfo)) == NULL) return NULL;
  }
  if (isconstructor(token))
  {
    CMD *next;
    TOKEN constructor = token;

    readtoken();
    if (cmd == NULL OR (next = readpipeline()) == NULL)
    {
      parseerror(ERR_NULLCMD, NULL);
      return cmd;
    }
    cmd = newcmd(constructor, cmd, next);
  }
  return cmd;
}

#ifdef CDL
CMD *readauxiliaries()
{
  CMD *cmd;

#ifdef DEBUGGING
  DEBUG("readauxiliaries()");
#endif
  cmd = readauxiliary();
  while (token == T_COMMA)
  {
    readtoken();
    cmd = newcmd(T_COMMA, cmd, readauxiliaries());
  }
  return cmd;
}

CMD *readauxiliary()
{
  CMD *cmd = NULL;

#ifdef DEBUGGING
  DEBUG("readauxiliary()");
#endif
  if (isconstructor(token))
  {
    CONSTRUCTOR constructor = token;

    readtoken();
    cmd = newcmd(constructor, NULL, readpipeline());
  }
  else parseerror(ERR_BADAUX, NULL);
  return cmd;
}
#endif

IOINFO *readredirection(IOINFO *ioinfo)
{
  TOKEN op = token;
  int flags = 0;
  BOOL diag = isdiag(op);
  BOOL input = isinput(op);
readtoken();
  if (ioinfo == NULL) ioinfo = newioinfo(NULL, NULL, NULL);
  else
  {
    if (input)
    {
      unless (ioinfo->input == NULL)
      {
        parseerror(ERR_INPUT, NULL);
        return ioinfo;
      }
    }
    else if (diag)
    {
      if (ioinfo->diag != NULL)
      {
        parseerror(ERR_DIAGNOSTIC, NULL);
        return ioinfo;
      }
    }
    else unless (ioinfo->output == NULL)
    {
      parseerror(ERR_OUTPUT, NULL);
      return ioinfo;
    }
  }
  unless (input)
  {
    if (token == T_AMPERSAND)
    {
      if (diag) 
      {
        parseerror(ERR_DIAGNOSTIC, NULL);
        return ioinfo;
      }
      else flags = FLAG_STDERR;
      readtoken();
    }
    if (strequ(currentword, "!"))
    {
      flags = FLAG_CLOBBER;
      readtoken();
    }
  }
  unless (token == T_WORD)
  {
    parseerror(ERR_REDIRECT, NULL);
    return ioinfo;
  }
  if (input) ioinfo->input = newopeninfo(op, currentword, 0);
  else if (diag) ioinfo->diag = newopeninfo (op, currentword, flags);
  else ioinfo->output = newopeninfo(op, currentword, flags);
  readtoken();
  return ioinfo;
}

CMD *readsimplecmd()
{
  IOINFO *ioinfo = NULL;
  ARGV argv;
  ARGV subargv;
  char *aliasname;

#ifdef DEBUGGING
  DEBUG("readsimplecmd()");
#endif
  forever
  {
    unless (token == T_WORD) return NULL;
    unless ((subargv = findalias(aliasname = currentword)) == NULL)
    {
      if (processing(aliasname)) subargv = NULL;
      else
      {
        if (expanding(aliasname))
        {
          parseerror(ERR_ALIASLOOP, NULL);
          return NULL;
        }
        subargv = historysub(dupargv(subargv));
        unless (historyused)
        {
          newexpansion(aliasname, subargv);
          continue;
        }
      }
    }
    if (isspecial(currentword))
    {
      parencount = 0;
      argv = nullargv();
      do
      {
        if (parencount == 0 AND isredirection(token))
        {
          if ((ioinfo = readredirection(ioinfo)) == NULL)
          {
            freeargv(argv);
            return NULL;
          }
        }
        else
        {
          argv = addword(argv, currentword);
         readtoken();
        }
      } until (token == T_NEWLINE OR parencount < 0 OR
               (parencount == 0 AND token != T_WORD AND token != T_RPAREN));
    }
    else
    {
      argv = nullargv();
      while (token == T_WORD OR isredirection(token))
      {
        if (token == T_WORD)
        {
          argv = addword(argv, currentword);
          readtoken();
        }
        else
        {
          if ((ioinfo = readredirection(ioinfo)) == NULL)
          {
            freeargv(argv);
            return NULL;
          }
        }
      }
    }
    unless (subargv == NULL)
    {
      unreadtoken();
      newexpansion(aliasname, subargv);
      freeargv(argv);
      continue;
    }
    return newsimple(argv, ioinfo);
  }
}

BOOL checkfor(TOKEN expectedtoken)
{
#ifdef DEBUGGING
  DEBUG("checkfor()");
#endif
  if (token == expectedtoken)
  {
    readtoken();
    return TRUE;
  }
  return FALSE;
}

void freeexpr(EXPR *expr)
{
  if (expr == NULL) return;
  if (expr->op == T_WORD) freeargv((ARGV)expr->left);
  else
  {
    freeexpr(expr->left);
    freeexpr(expr->right);
  }
  freememory((int *)expr);
}

void freecmd(CMD *cmd)
{
  if (cmd == NULL) return;
  if (cmd->op == T_SIMPLE) freeargv((ARGV)cmd->This);
  else freecmd(cmd->This);
  freecmd(cmd->next);
  freeioinfo(cmd->ioinfo);
  freememory((int *)cmd);
}

void freeioinfo(IOINFO *ioinfo)
{
  OPENINFO *openinfo;

  if (ioinfo == NULL) return;
  unless ((openinfo = ioinfo->input) == NULL)
  {
    freememory((int *)openinfo->name);
    freememory((int *)openinfo);
  }
  unless ((openinfo = ioinfo->output) == NULL)
  {
    freememory((int *)openinfo->name);
    freememory((int *)openinfo);
  }
  freememory((int *)ioinfo);
}

CMD *dupcmd(CMD *cmd)
{
  CMD *dup, *This;

  if (cmd == NULL) return NULL;
  if (cmd->op == T_SIMPLE)
    This = (CMD *)dupargv((ARGV)cmd->This);
  else This = dupcmd(cmd->This);
  dup = newcmd(cmd->op, This, dupcmd(cmd->next));
  dup->ioinfo = dupioinfo(cmd->ioinfo);
  return dup;
}

IOINFO *dupioinfo(IOINFO *ioinfo)
{
  OPENINFO *input, *output, *diag;

  if (ioinfo == NULL) return NULL;
  unless ((input = ioinfo->input) == NULL)
  {
    input = newopeninfo(input->op, input->name, input->flags);
  }
  unless ((output = ioinfo->output) == NULL)
  {
    output = newopeninfo(output->op, output->name, output->flags);
  }
  unless ((diag = ioinfo->diag) == NULL)
  {
    diag = newopeninfo(diag->op, diag->name, diag->flags);
  }
  return newioinfo(input, output, diag);
}

char *strend(char *s)
{
  return s + strlen(s);
}

void sputioinfo(
		char *buffer,
		IOINFO *ioinfo )
{
  OPENINFO *openinfo;

  if (ioinfo == NULL) return;
  unless ((openinfo = ioinfo->input) == NULL)
  {
    MODE mode = openinfo->op;

    if (mode == T_READ) strcat(buffer, " < ");
#ifdef CDL
    else if (mode == T_READFIFO) strcat(buffer, " <| ");
#endif
    else strcat(buffer, " << ");
    strcat(buffer, openinfo->name);
    strcat(buffer, " ");
  }
  unless ((openinfo = ioinfo->output) == NULL)
  {
    MODE mode = openinfo->op;

    if (mode == T_WRITE) strcat(buffer, " > ");
#ifdef CDL
    else if (mode == T_WRITEFIFO) strcat(buffer, " >| ");
#endif
    else ignore strcat(buffer, " >> ");
    if (openinfo->flags & FLAG_STDERR) strcat(buffer, " & ");
    if (openinfo->flags & FLAG_CLOBBER) strcat(buffer, " ! ");
    strcat(buffer, openinfo->name);
    strcat(buffer, " ");
  }
  unless ((openinfo = ioinfo->diag) == NULL)
  {
    MODE mode = openinfo->op;

    if (mode == T_WRITEDIAG) strcat(buffer, " >2 ");
    else ignore strcat(buffer, " >>2 ");
    if (openinfo->flags & FLAG_CLOBBER) strcat(buffer, " ! ");
    strcat(buffer, openinfo->name);
    strcat(buffer, " ");
  }
}

void sputcmd(
	     char *buffer,
	     CMD *cmd )
{
  for (; cmd != NULL; cmd = cmd->next)
  {
    switch (cmd->op)
    {
      case T_SIMPLE:
      sputargv(strend(buffer), (ARGV)cmd->This, ' ');
      sputioinfo(strend(buffer), (IOINFO *)cmd->next);
      return;

      case T_SEMICOLON:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, "; ");
      break;

      case T_AMPERSAND:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " & ");
      break;

      case T_OR:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " || ");
      break;

      case T_AND:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " && ");
      break;

      case T_PIPE:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " | ");
      break;

#ifdef CDL
      case T_PAR:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " ^^ ");
      break;

      case T_REVPIPE:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " |< ");
      break;

      case T_SUBORDINATE:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " <> ");
      break;

      case T_FARM:
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " ||| ");
      break;
#endif
      case T_LIST:
      strcat(buffer, "( ");
      sputcmd(strend(buffer), cmd->This);
      strcat(buffer, " )");
      sputioinfo(strend(buffer), (IOINFO *)cmd->next);
      return;
    }
  }
}

void putcmd(CMD *cmd)
{
  for (; cmd != NULL; cmd = cmd->next)
  {
    switch (cmd->op)
    {
      case T_SIMPLE:
      putargv((ARGV)cmd->This, FALSE);
      putioinfo((IOINFO *)cmd->next);
      return;

      case T_SEMICOLON:
      putcmd(cmd->This);
      printf("; ");
      break;

      case T_AMPERSAND:
      putcmd(cmd->This);
      printf(" & ");
      break;

      case T_OR:
      putcmd(cmd->This);
      printf(" || ");
      break;

      case T_AND:
      putcmd(cmd->This);
      printf(" && ");
      break;

      case T_PIPE:
      putcmd(cmd->This);
      printf(" | ");
      break;

#ifdef CDL
      case T_PAR:
      putcmd(cmd->This);
      printf(" ^^ ");
      break;

      case T_REVPIPE:
      putcmd(cmd->This);
      printf(" |< ");
      break;

      case T_SUBORDINATE:
      putcmd(cmd->This);
      printf(" <> ");
      break;

      case T_FARM:
      putcmd(cmd->This);
      printf(" ||| ");
      break;
#endif
      case T_LIST:
      printf("( ");
      putcmd(cmd->This);
      printf(" )");
      putioinfo((IOINFO *)cmd->next);
      return;
    }
  }
}

void putioinfo(IOINFO *ioinfo)
{
  OPENINFO *openinfo;

  if (ioinfo == NULL) return;
  unless ((openinfo = ioinfo->input) == NULL)
  {
    MODE mode = openinfo->op;

    if (mode == T_READ) printf(" < ");
#ifdef CDL
    else if (mode == T_READFIFO) printf(" <| ");
#endif
    else printf(" << ");
    printf("%s ", openinfo->name);
  }
  unless ((openinfo = ioinfo->output) == NULL)
  {
    MODE mode = openinfo->op;

   if (mode == T_WRITE) printf(" > "); 
#ifdef CDL
    else if (mode == T_WRITEFIFO) printf(" >| ");
#endif
    else printf(" >> ");
    if (openinfo->flags & FLAG_STDERR) printf(" & ");
    if (openinfo->flags & FLAG_CLOBBER) printf(" ! ");
    printf("%s ", openinfo->name);
  }
  unless ((openinfo = ioinfo->diag) == NULL)
  {
    if (openinfo->op == T_WRITEDIAG) printf(" >2 ");
    else printf(" >>2 ");
    if (openinfo->flags & FLAG_CLOBBER) printf(" ! ");
    printf("%s ", openinfo->name);
  }
}

int evaluate(EXPR *expr)
{
  char buffer1[11], buffer2[11];

  switch (expr->op)
  {
    case T_WORD:
    return atoi(((ARGV)expr->left)[0]);

    case T_OR:
    return evaluate(expr->left) OR evaluate(expr->right);

    case T_AND:
    return evaluate(expr->left) AND evaluate(expr->right);

    case T_BITOR:
    return evaluate(expr->left) | evaluate(expr->right);

    case T_BITXOR:
    return evaluate(expr->left) ^ evaluate(expr->right);

    case T_BITAND:
    return evaluate(expr->left) & evaluate(expr->right);

    case T_EQ:
    return strequ(streval(buffer1, expr->left), streval(buffer2, expr->right));

    case T_NE:
    return !strequ(streval(buffer1, expr->left), streval(buffer2, expr->right));

    case T_MATCH:
    return match(streval(buffer1, expr->left), streval(buffer2, expr->right));

    case T_NOMATCH:
    return !match(streval(buffer1, expr->left), streval(buffer2, expr->right));

    case T_LE:
    return evaluate(expr->left) <= evaluate(expr->right);

    case T_GE:
    return evaluate(expr->left) >= evaluate(expr->right);

    case T_LT:
    return evaluate(expr->left) < evaluate(expr->right);

    case T_GT:
    return evaluate(expr->left) > evaluate(expr->right);

    case T_LSHIFT:
    return evaluate(expr->left) << evaluate(expr->right);

    case T_RSHIFT:
    return evaluate(expr->left) >> evaluate(expr->right);

    case T_PLUS:
    return evaluate(expr->left) + evaluate(expr->right);

    case T_MINUS:
    return evaluate(expr->left) - evaluate(expr->right);

    case T_TIMES:
    return evaluate(expr->left) * evaluate(expr->right);

    case T_DIVIDE:
    return evaluate(expr->left) / evaluate(expr->right);

    case T_REM:
    return evaluate(expr->left) % evaluate(expr->right);

    case T_NOT:
    return !evaluate(expr->left);

    case T_ONECOMP:
    return ~evaluate(expr->left);
  }
  return 0;
}

char *streval(
	      char *buffer,
	      EXPR *expr )
{
  if (expr->op == T_WORD) return ((ARGV)expr->left)[0];
  ignore sprintf(buffer, "%d", evaluate(expr));
  return buffer;
}

BOOL isexproperator(TOKEN token)
{
  return (BOOL)
    (token == T_OR OR token == T_AND OR token == T_BITOR OR
     token == T_BITXOR OR token == T_BITAND OR token == T_EQ OR
     token == T_NE OR token == T_MATCH OR token == T_NOMATCH OR
     token == T_LE OR token == T_GE OR token == T_LT OR
     token == T_GT OR token == T_LSHIFT OR token == T_RSHIFT OR
     token == T_PLUS OR token == T_MINUS OR token == T_TIMES OR
     token == T_DIVIDE OR token == T_REM OR token == T_NOT OR
     token == T_ONECOMP );
}

int getexprpri(TOKEN op)
{
  switch (op)
  {
    case T_OR:
    return 1;

    case T_AND:
    return 2;

    case T_BITOR:
    return 3;

    case T_BITXOR:
    return 4;

    case T_BITAND:
    return 5;

    case T_EQ:
    case T_NE:
    case T_MATCH:
    case T_NOMATCH:
    return 6;

    case T_LE:
    case T_GE:
    case T_LT:
    case T_GT:
    case T_LSHIFT:
    case T_RSHIFT:
    return 7;

    case T_PLUS:
    case T_MINUS:
    return 8;

    case T_TIMES:
    case T_DIVIDE:
    case T_REM:
    return 9;

    case T_NOT:
    case T_ONECOMP:
    return 10;

    default:
    bug("Unexpected operator");
    return 0;
  }
}

EXPR *readexpr(int rightpri)
{
  EXPR *expr = readprefixexpr();

  forever
  {
    if (isexproperator(token))
    {
      TOKEN op = token;
      int leftpri = getexprpri(op);

      if (leftpri <= rightpri) return expr;
      readexprtoken();
      expr = newexpr(op, expr, readexpr(leftpri));
    }
    else return expr;
  }
}

EXPR *readprefixexpr()
{
  EXPR *expr;

  if (token == T_LPAREN)
  {
    readexprtoken();
    expr = readexpr(0);
    if (token != T_RPAREN) printf(") expected\n");
    readexprtoken();
  }
  else if (isexproperator(token) OR token == T_RPAREN)
  {
    expr = newexpr(T_WORD, (EXPR *)nullargv(), NULL);
  }
  else
  {
    expr = newexpr(T_WORD, (EXPR *)makeargv(currentword), NULL);
    readexprtoken();
  }
  return expr;
}

BOOL match(
	   char *str,
	   char *pattern )
{
  char c;

  until ((c = *pattern++) == '\0')
  {
    switch (c)
    {
      default:
      if (*str++ == c) continue;
      return FALSE;

      case '?':
      if (*str++) continue;
      return FALSE;

      case '[':
      until ((c = *pattern++) == ']' OR c == '\0')
      {
        if (*pattern == '-')
        {
          pattern++;
          if (*str >= c AND *str <= *pattern++)
          {
            c = *str;
            break;
          }
        }
        else if (*str == c) break;
      }
      if (*str++ == c)
      {
        until ((c = *pattern++) == ']' OR c == '\0');
	if(c != '\0')
            continue;
      }
      return FALSE; 

      case '*':
      if (*pattern == '\0') return TRUE;
      while (*str)
      {
        if (match(str, pattern)) return TRUE;
        str++;
      }
      return FALSE;
    }
  }
  if (*str) return FALSE;
  return TRUE;
}
