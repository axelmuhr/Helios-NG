/**
*
* Title:  Helios Debugger - Source file displaying.
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
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/source.c,v 1.5 1993/03/17 17:39:35 nickc Exp $";
#endif

#include "tla.h"

PRIVATE LIST sourcelist;
PRIVATE Semaphore sourcelock;

/**
*
* initsource();
*
* Initialise the source list.
*
**/
PUBLIC void initsource(void)
{
  InitList(&sourcelist);
  InitSemaphore(&sourcelock, 1);
}

/**
*
* source = loadsource(debug, name);
*
* Load a source file.
*
**/
PUBLIC SOURCE *loadsource(DEBUG *debug, char *name)
{
  SOURCE *source;
  Stream *stream = NULL;
  char *filetext = NULL;
  char **linevec = NULL;

  debugf("loadsource(%s)", name);
  if ((source = findsource(name)) == NULL)
  {
    char **newlinevec;
    char *fileptr;
    int filesize;
    int line = 0;
    int linemax = 0;

#ifdef OLDCODE
    if ((stream = Open(debug->env.Objv[0], name, O_ReadOnly)) == NULL) return NULL;
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
#ifdef OLDCODE
    if ((stream = Open(debug->env.Objv[OV_Cdir], name, O_ReadOnly)) == NULL) 
      return NULL;
#else
/*
-- crf : 26/09/91 - Bug 677
-- Problem : if client is run from a directory other than that where the 
-- source files are located, an empty debug window is created. 
-- Solution : if the source and .dbg files are not present in the current
-- directory, the debugger looks for the directory defined by the environment
-- variable DBGSRC_VARNAME (#defined in tla.h).
-- Notes : I have changed the following sources :
--   1. loadsource() (source.c) - loads the program sources
--   2. loadinfo() (info.c) - loads the .dbg file
*/

    if ((stream = Open(debug->env.Objv[OV_Cdir], name, O_ReadOnly)) == NULL) 
      {
	char *		dbgsrc;
	Object *	path_obj ;

      
	if ((dbgsrc = getvar(debug->env.Envv, DBGSRC_VARNAME)) == NULL) 
	  return NULL;

	if ((path_obj = Locate(NULL, dbgsrc)) == NULL)
	  {
	    return NULL ;
	  }      

	if ((stream = Open(path_obj, name, O_ReadOnly)) == NULL)
	  {
	    return NULL ;
	  }      
      }    
#endif

    if ((source = NEW(SOURCE)) == NULL) goto err;
    filesize = (int)GetFileSize(stream);

    if ((filetext = (char *)newmem(filesize)) == NULL) goto err;
    unless (Read(stream, filetext, filesize, -1) == filesize) goto err;

    for (fileptr = filetext; fileptr - filetext < filesize;)
    {
      if (line >= linemax)
      {
        linemax += MAX_LINEVEC;
        if ((newlinevec = (char **)newmem(sizeof(char *) * linemax)) == NULL)
          goto err;
        unless (linevec == NULL)
        {
          memmove(newlinevec, linevec, sizeof(char *) * line);
          freemem(linevec);
        }
        linevec = newlinevec;
      }
      linevec[line++] = fileptr;
      until (*fileptr == '\n' OR *fileptr == '\r') fileptr++;
      if (*fileptr == '\r')
      {
        *fileptr++ = '\0';
        if (*fileptr == '\n') fileptr++;
      }
      else *fileptr++ = '\0';
    }

    if ((source->name = strdup(stream->Name)) == NULL) goto err;
    source->lastline = line;
    source->linevec = linevec;
    source->usage = 0;
    Close(stream);
    debugf("read in source, lines = %d", line);
    Wait(&sourcelock);
    AddTail(&sourcelist, &source->node);
    Signal(&sourcelock);
  }
  source->usage++;
  return source;

err:
  unless (stream == NULL) Close(stream);
  unless (source == NULL) freemem(source);
  unless (filetext == NULL) freemem(filetext);
  unless (linevec == NULL) freemem(linevec);
  return NULL;
}

/**
*
* unloadsource(source);
*
* Unload a source file.
*
**/
PUBLIC void unloadsource(SOURCE *source)
{
  if (--source->usage == 0)
  {
    Wait(&sourcelock);
    Remove(&source->node);
    Signal(&sourcelock);
    freemem(source->name);
    freemem(source->linevec[0]);
    freemem(source->linevec);
    freemem(source);
  }
}

/**
*
* found = sourcecmp(source, name)
*
* Support routine for findsource().
*
**/
PRIVATE BOOL cmpsource(SOURCE *source, char *name)
{
  return strequ(source->name, name);
}

/**
*
* source = findsource(name);
*
* Find a source file.
*
**/
PUBLIC SOURCE *findsource(char *name)
{
  SOURCE *source;

  Wait(&sourcelock);
  source = (SOURCE *)SearchList(&sourcelist, (WordFnPtr)cmpsource, (word)name);
  Signal(&sourcelock);
  return source;
}

/**
*
* list(display, source, line, count);
*
* Display a source file.
*
**/
PUBLIC void list(DISPLAY *display, SOURCE *source, int line, int count)
{
  while (count-- > 0)
    {
      if (line <= source->lastline)
	{
	  char text[81];


	  tabexp(text, source->linevec[line - 1], display->width - DisplayBorder);

	  dprintf(display, "%4d: %s", line, text);
	}

      deol(display);

      dprintf(display, "\r\n");

      line++;
    }

  return;
}

#ifndef OLDCODE
PRIVATE BOOL match(char *str, char *pat)
{
  int c;

  until ((c = *pat++) == '\0')
  {
    switch (c)
    {
      case '?':
      if (*str++ == '\0') return FALSE;
      continue;

      case '[':
      until ((c = *pat++) == ']' OR c == '\0')
      {
        if (*pat == '-')
        {
          pat++;
          if (*str >= c AND *str <= *pat++)
          {
            c = *str;
            break;
          }
        }
        else if (*str == c) break;
      }
      if (*str++ == c)
      {
        until ((c = *pat++) == ']' OR c == '\0');
        continue;
      }
      return FALSE;

      case '$':
      if (*str == '\0') return TRUE;
      return FALSE;

      case '\\':
      if ((c = *pat++) == '\0') return FALSE;
      unless (*str++ == c) return FALSE;
      continue;

      case '*':
      if (*pat == '\0') return TRUE;
      until (*str == '\0')
      {
        if (match(str++, pat)) return TRUE;
      }
      return FALSE;

      default:
      unless (*str++ == c) return FALSE;
      continue;
    }
  }
#ifdef OLDCODE
  if (*str == '\0') return TRUE;
  return FALSE;
#else
  return TRUE;
#endif
}

PRIVATE int matchline(char *line, char *pat)
{
  int i;

  if (*pat == '^') return match(line, pat + 1) ? 0 : -1;
  for (i = 0; line[i] != '\0'; i++)
  {
    if (match(line + i, pat)) return i;
  }
  return -1;
}
#else

PRIVATE int matchline(char *line, char *str)
{
  int len = strlen(str);
  int linelen = strlen(line);
  int i;

  for (i = 0; i < linelen; i++)
  {
    if (strnequ(str, line + i, len)) return i;
  }
  return -1;
}
#endif

/**
*
* line = search(source, str, start, backward, wrap);
*
*
* Search for a string in a source file.
*
**/
PUBLIC int search(SOURCE *source, char *str, int start, BOOL backward, BOOL wrap)
{
  int line;
  int col;

  if (backward)
  {
    if (wrap)
    {
      line = (start > 1) ? (start - 2) : (source->lastline - 1);
      do
      {
        unless ((col = matchline(source->linevec[line], str)) == -1)
          return line + 1;
      	if (line > 0) line--;
      	else line = source->lastline - 1;
      } until (line == start);
    }
    else
    {
      for (line = start - 2; line >= 0; line--)
      {
        unless ((col = matchline(source->linevec[line], str)) == -1)
          return line + 1;
      }
    }
  }
  else
  {
    if (wrap)
    {
      line = (start < source->lastline) ? start : 0;
      do
      {
        unless ((col = matchline(source->linevec[line], str)) == -1)
          return line + 1;
      	if (line < source->lastline) line++;
      	else line = 0;
      } until (line == start);
    }
    else
    {
      for (line = start; line < source->lastline; line++)
      {
        unless ((col = matchline(source->linevec[line], str)) == -1)
          return line + 1;
      }
    }
  }
  return 0;
}

/**
*
* text = getword(buf, source, line, col);
*
* Get text from a source file into a buffer expanding to word boundary.
*
**/
PRIVATE BOOL isnamech(int c)
{
  return isalnum(c) OR c == '_';
}

PUBLIC char *getword(char *buf, SOURCE *source, int line, int col, int size)
{
  char text[81];
  int len;

  buf[0] = ' ';
  buf[1] = '\0';
  if (line > source->lastline) return buf;
  tabexp(text, source->linevec[line - 1], 80);
  unless (col < (len = strlen(text))) return buf;
  if (isnamech(text[col]))
    while (col > 0 AND isnamech(text[col - 1]))
    {
      col--;
      size++;
    }
  if (col + size > len) size = len - col;
  else if (isnamech(text[col + size - 1]))
    while (col + size < len AND isnamech(text[col + size])) size++;
  memcpy(buf, text + col, size);
  buf[size] = '\0';
  return buf;
}

/**
*
* text = gettext(buf, source, line, col);
*
* Get text from a source file into a buffer.
*
**/
PUBLIC char *gettext(char *buf, SOURCE *source, int line, int col, int size)
{
  char text[81];
  int len;

  buf[0] = ' ';
  buf[1] = '\0';
  if (line > source->lastline) return buf;
  tabexp(text, source->linevec[line - 1], 80);
  unless (col < (len = strlen(text))) return buf;
  if (col + size > len) size = len - col;
  memcpy(buf, text + col, size);
  buf[size] = '\0';
  return buf;
}
