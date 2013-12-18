/**
*
* Title:  Helios Shell - 
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/list.c,v 1.6 1994/04/06 11:20:11 nickc Exp $
*
**/
#include "shell.h"

LIST aliaslist;
LIST historylist;
LIST varlist;
LIST dirlist;

void fputargv(
	      FILE *file,
	      ARGV argv,
	      BOOL newline )
{
  char *arg = *argv++;

  until (arg == NULL)
  {
    fprintf(file, "%s", arg);
    unless ((arg = *argv++) == NULL) fputc(' ', file);
  }
  if (newline) fputc('\n', file);
  else fflush(file);
}

#ifdef __STDC__
void sputargv(char *buffer, ARGV argv, char filler)
#else
void sputargv(buffer, argv, filler)
char *buffer;
ARGV argv;
char filler;
#endif
{
  char *arg;
  int length = 0;

  until ((arg = *argv++) == NULL)
  {
    while (*arg) buffer[length++] = *arg++;
    if (*argv) buffer[length++] = filler;
  }
  buffer[length] = '\0';
}

int compar(
	   char **s1,
	   char **s2 )
{
  return strcmp(*s1, *s2);
}

typedef int (*cmpfn)(const void *, const void *);

void putsortedargv(char *argv[])
{
  char *duparg;
  int argc = lenargv(argv);
  int i, length, numcols, numrows, row, colwidth;
  int maxlength = 0;

  qsort((void *)argv, argc, sizeof(char *), (cmpfn)compar);
  if ( argc >= 2 )
  {
    for (i = 1; i < argc; i++)
    {
      unless (strcmp (argv[i-1], argv[i]))
      {
      	duparg = argv[i-1];
      	memmove (&argv[i-1], &argv[i], (argc - i) * sizeof (char *));
      	argv[--argc] = duparg;
      }
    }
  }
  for (i = 0; i < argc; i++)
  {
    if ((length = strlen(argv[i])) > maxlength) maxlength = length;
  }
  colwidth = maxlength + 1;
  numcols = (SCREEN_WIDTH - 1) / colwidth;
  numrows = (argc + numcols - 1) / numcols;
  for (row = 0; row < numrows; row++)
  {
    for (i = row; i < argc; i += numrows)
    {
      int j = 0;
      int c;

      until ((c = argv[i][j++]) == '\0') putchar(c);
      while (j++ <= colwidth) putchar(' ');
    }
    putchar('\n');
  }
}

ARGV nullargv()
{
  ARGV argv = (ARGV)newmemory(sizeof(char *) * ARGV_MAX);

  argv[0] = 0;
  return argv;
}

ARGV makeargv(char *arg)
{
  ARGV argv = nullargv();

#ifdef DEBUGGING
DEBUG("makeargv()");
#endif

  return addword(argv, arg);
}

ARGV nummakeargv(int number)
{
  char buffer[11];

  ignore sprintf(buffer, "%d", number);
  return makeargv(buffer);
}

ARGV envmakeargv(char *name)
{
  char *value;

  if ((value = getenv(name)) == NULL) return makeargv("");
  else
  { 
    ARGV argv = nullargv();
    int c;

    until ((c = *value++) == '\0')
    {
      if (c == ENVCHAR)
      {
        ignore endword();
        unixpath(wordbuffer);
        argv = addword(argv, wordbuffer);
      }
      else addchar(c);
    }
    ignore endword();
    unixpath(wordbuffer);
    argv = addword(argv, wordbuffer);
    return argv;
  }
}

ARGV buildargv(ARGV argv)
{
  char *arg;
  ARGV oldargv = argv;
  ARGV newargv = nullargv();

#ifdef DEBUGGING
  DEBUG("buildargv(%V)",argv);
#endif
  parencount = 0;
  until ((arg = *argv++) == NULL)
  {
    until ((arg = getword(arg)) == NULL)
    {
      unless (wordstate == NEUTRAL)
      {
        freeargv(oldargv);
        freeargv(newargv);
        switch (wordstate)
        {
          case INSQUOTE:
          error(ERR_SQUOTE, NULL);
          break;

          case INBQUOTE:
          error(ERR_BQUOTE, NULL);
          break;

          case INDQUOTE:
          error(ERR_DQUOTE, NULL);
          break;
        }
        recover();
      }
      newargv = addword(newargv, wordbuffer);
    }
  }
  freeargv(oldargv);
  return newargv;
}

ARGV dupargv(ARGV argv)
{
  char *arg;
  ARGV newargv = nullargv();

  until ((arg = *argv++) == NULL) newargv = addword(newargv, arg);
  return newargv;
}

ARGV addword(
	     ARGV argv,
	     char *arg )
{
  int length = lenargv(argv);
  int i;

#ifdef DEBUGGING
  DEBUG("addword '%s'",arg);
#endif
  if (((length + 1) % ARGV_MAX) == 0)
  {
    ARGV newargv = (ARGV)newmemory(sizeof(char *) * (length + 1 + ARGV_MAX));

    for (i = 0; i < length; i++) newargv[i] = argv[i];
    freememory((int *)argv);
    argv = newargv;
  }
  argv[length++] = strdup(arg);
  argv[length] = 0;
  return argv;
}

ARGV prefixword(
		ARGV argv,
		char *arg )
{
  int length = lenargv(argv);
  int i;

  if (((length + 1) % ARGV_MAX) == 0)
  {
    ARGV newargv = (ARGV)newmemory(sizeof(char *) * (length + ARGV_MAX));

    for (i = 0; i <= length; i++) newargv[i + 1] = argv[i];
    freememory((int *)argv);
    argv = newargv;
  }
  else
  {
    for (i = length; i >= 0; i--) argv[i + 1] = argv[i];
  }
  argv[0] = strdup(arg);
  return argv;
}

void set(
	 char *name,
	 ARGV argv )
{
  setsubnode(&varlist, name, argv);
  if (strequ(name, "path"))
  {
    char buffer[WORD_MAX + 1];

    sputargv(buffer, argv, ':');
    setenv("PATH", buffer);
    hash();
  }
  else if (strequ(name, "cdl"))  setenv("CDL", argv[0]);
  else if (strequ(name, "home")) setenv("HOME", argv[0]);
  else if (strequ(name, "term")) setenv("TERM", argv[0]);
  else if (strequ(name, "user")) setenv("USER", argv[0]);
}

BOOL setword(
	     char *name,
	     ARGV argv,
	     int index,
	     char *text )
{
  if (index <= 0 OR index > lenargv(argv)) return FALSE;
  freememory((int *)argv[index - 1]);
  argv[index - 1] = strdup(text);
  if (strequ(name, "path"))
  {
    char buffer[WORD_MAX + 1];

    sputargv(buffer, argv, ':');
    setenv("PATH", buffer);
    hash();
  }
  else if (strequ(name, "cdl"))  setenv("CDL", argv[0]);
  else if (strequ(name, "home")) setenv("HOME", argv[0]);
  else if (strequ(name, "term")) setenv("TERM", argv[0]);
  else if (strequ(name, "user")) setenv("USER", argv[0]);
  return TRUE;
}

void unset(char *pattern)
{
  if (match(pattern, "cdl")) delenv("CDL");
  patremsubnode(&varlist, pattern);
}

void freesubnode(SUBNODE *subnode)
{
  freeargv(subnode->argv);
  freememory((int *)subnode->name);
  freememory((int *)subnode);
}

void fputsublist(
		 FILE *file,
		 LIST *sublist,
		 int length,
		 BOOL nonames,
		 BOOL noparens,
		 BOOL reverse )
{
  SUBNODE *subnode;
  int i = 0;

  if (length == -1) length = lensublist(sublist);
  if (reverse)
  {
    for (subnode = (SUBNODE *)sublist->Tail; subnode->prev; subnode = subnode->prev)
    {
      if (++i > length) break;
#ifdef NOTABS
      unless (nonames)
      {
        int tablen = 8 - (strlen(subnode->name) % 8);

        fprintf(file, "%s", subnode->name);
        while (tablen-- > 0) fputc(' ', file);
      }
#else
      unless (nonames) fprintf(file, "%s\t", subnode->name);
#endif
      if (noparens OR lenargv(subnode->argv) == 1)
        fputargv(file, subnode->argv, FALSE);
      else
      {
        fputc('(', file);
        fputargv(file, subnode->argv, FALSE);
        fputc(')', file);
      }
      fputc('\n', file);
    }
  }
  else
  {
    length = lensublist(sublist) - length;
    for (subnode = (SUBNODE *)sublist->Head; subnode->next; subnode = subnode->next)
    {
      if (++i > length)
      {
#ifdef NOTABS
        unless (nonames)
        {
          int tablen = 8 - (strlen(subnode->name) % 8);

          fprintf(file, "%s", subnode->name);
          while (tablen-- > 0) fputc(' ', file);
        }
#else
        unless (nonames) fprintf(file, "%s\t", subnode->name);
#endif
        if (noparens OR lenargv(subnode->argv) == 1)
          fputargv(file, subnode->argv, FALSE);
        else
        {
          fputc('(', file);
          fputargv(file, subnode->argv, FALSE);
          fputc(')', file);
        }
        fputc('\n', file);
      }
    }
  }
}

int lenargv(ARGV argv)
{
  int length = 0;

  while (argv[length]) length++;
  return length;
}

void freeargv(ARGV argv)
{ 
  unless (argv == NULL)
  {
    char *arg;
    int i = 0;

    until ((arg = argv[i++]) == NULL) freememory((int *)arg);
    freememory((int *)argv);
  }
}

void addsubnode(
		LIST *sublist,
		char *name,
		ARGV argv )
{
  SUBNODE *subnode = new(SUBNODE);

  subnode->name = strdup(name);
  subnode->argv = argv;
#ifdef SYSDEB
  subnode->next = subnode->prev = subnode;
#endif
  
  AddTail(sublist, (NODE *)subnode);
}

void setsubnode(
		LIST *sublist,
		char *name,
		ARGV argv )
{
  SUBNODE *next;
  int result;

  for (next = (SUBNODE *)sublist->Head; next->next; next = next->next)
  {
    if ((result = strcmp(name, next->name)) == 0)
    {
      freeargv(next->argv);
      next->argv = argv;
      return;
    }
    else if (result < 0)
    {
      SUBNODE *subnode = new(SUBNODE);
       
      subnode->name = strdup(name);
      subnode->argv = argv;
#ifdef SYSDEB
      subnode->next = subnode->prev = subnode;
#endif
      PreInsert((Node *)next, (Node*)subnode);
      return;
    }
  }
  addsubnode(sublist, name, argv);
}

void remsubnode(LIST *sublist)
{
  SUBNODE *subnode;

  unless ((subnode = (SUBNODE *)RemHead(sublist)) == NULL) freesubnode(subnode);
}

void patremsubnode(
		   LIST *sublist,
		   char *pattern )
{
  SUBNODE *subnode;
  SUBNODE *next;

  for (subnode = (SUBNODE *)sublist->Head; subnode->next; subnode = next)
  {
    next = subnode->next;
    if (match(subnode->name, pattern))
    {
      Remove((NODE *)subnode);
      freesubnode(subnode);
    }
  }
}

char *getsubnode(
		 LIST *sublist,
		 char *name )
{
  ARGV argv;
  char *arg;

  if ((argv = findsubnode(sublist, name)) == NULL OR
      (arg = argv[0]) == NULL) return NULL;
  return arg;
}

ARGV findsubnode(
		 LIST *sublist,
		 char *name )
{
  SUBNODE *subnode;
  int result;

  for (subnode = (SUBNODE *)sublist->Head; subnode->next; subnode = subnode->next)
  {
    if ((result = strcmp(name, subnode->name)) == 0) return subnode->argv;
    if (result < 0) return NULL;
  }
  return NULL;
}

void addevent(ARGV argv)
{
  char buffer[NUMSTR_MAX + 1];
  char *history;
  int length = 1;

  ignore sprintf(buffer, "%6d", eventnumber++);
  addsubnode(&historylist, buffer, argv);
  unless ((history = getvar("history")) == NULL)
    length = atoi(history);		/* get value if history defined	*/
  if (length < 1) length = 1;		/* check for negative value	*/
  length = lensublist(&historylist) - length;	/* calc delete count	*/
  while (length-- > 0) remsubnode(&historylist);	
}

ARGV findevent(int number)
{
  char buffer[NUMSTR_MAX + 1];

  ignore sprintf(buffer, "%6d", number);
  return findsubnode(&historylist, buffer);
}

ARGV findhistory(char *event)
{
  SUBNODE *subnode;
  int length = strlen(event);

  for (subnode = (SUBNODE *)historylist.Tail; subnode->prev; subnode = subnode->prev)
  {
    char *arg;

    unless ((arg = subnode->argv[0]) == NULL)
    {
      if (strnequ(event, arg, length)) return subnode->argv;
    }
  }
  return NULL;
}

int lensublist(LIST *sublist)
{
  int length = 0;
  SUBNODE *subnode;

  for (subnode = (SUBNODE *)sublist->Head; subnode->next; subnode = subnode->next)
    length++;
  return length;
}

void adddir(char *path)
{
  DIRNODE *dirnode = new(DIRNODE);

  dirnode->name = strdup(path);
#ifdef SYSDEB
  dirnode->next = dirnode->prev = dirnode;
#endif
  AddHead(&dirlist, (NODE *)dirnode);
}

BOOL popdir(char *path)
{
  DIRNODE *dirnode;

  if ((dirnode = Head_( DIRNODE, dirlist )) == NULL) return FALSE;
  strcpy(path, dirnode->name);
  freedirnode(dirnode);
  return TRUE;
}

BOOL getdir(
	    int number,
	    char *path )
{
  DIRNODE *dirnode;

  for (dirnode = (DIRNODE *)dirlist.Head; dirnode->next; dirnode = dirnode->next)
  {
    if (number-- == 0)
    {
      strcpy(path, dirnode->name);
      freedirnode(dirnode);
      return TRUE;
    }
  }
  return FALSE;
}
 
void putdirnode(DIRNODE *dirnode)
{
  printf("%s ", dirnode->name);
}

void freedirnode(DIRNODE *dirnode)
{
  Remove((NODE *)dirnode);
  freememory((int *)dirnode->name);
  freememory((int *)dirnode);
}
 
