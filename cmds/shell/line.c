/**
*
* Title:  Helios Shell - Line editing
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/line.c,v 1.8 1993/08/12 14:50:06 nickc Exp $
*
**/
#include "shell.h"

BOOL (*controlkeyaction[32])() =
{
  nothing,
  startofline,
  left,
  ctrlc,
  display,
  endofline,
  right,
  nothing,
  erasechar,
  nothing,
  nothing,
  deletetoend,
  leftword,
  nothing,
  down,
  nothing,
  up,
  nothing,
  retrieve,
  nothing,
  nothing,
  undo,
  nothing,
  rightword,
  deleteword,
  nothing,
  nothing,
  complete,
  nothing,
  nothing,
  nothing,
  nothing
};
static int lineindex;
static int linelength;
static int oldlinelength;
static char linebuffer[LINE_MAX + 1];
static char savebuffer[LINE_MAX + 1];
static SUBNODE *subnode;

char *getline(
	      char *buffer,
	      BOOL searching )
{
  BOOL (*action)();
  int c;
  int seconds;
  char *autologout;

#ifdef DEBUGGING
  DEBUG("getline()");
#endif
  unless (interactive)
  {
    char localbuf[LINE_MAX+1];
    char *bufptr;
    *buffer = '\0';
    if ((bufptr = fgets (buffer, LINE_MAX, inputfile)) == NULL) 
      return bufptr;

    /* XXX - NC - 29/3/93 bug fix 1029 */
    
    c = strlen(bufptr);
    if (c > 1 AND bufptr[c-1] == '\n' AND bufptr[c-2] == '\r')
      {
	bufptr[c-2]=' ';
      }
    else if (bufptr[c-1] == 0x1A)
      {
        bufptr[c-1]=' ';	/* XXX ignore ^Z at end of line/file */
      }
    else
      {
	while ((c = strlen(bufptr)) > 1 AND 
	       bufptr[c-1] == '\n' AND bufptr[c-2] == '\\' )
	  { 
	    word pos = ftell (inputfile);
	    unless (fgets (localbuf, LINE_MAX, inputfile)) return bufptr;
	    if ((c = strlen (bufptr) + strlen (localbuf)) >= LINE_MAX)
	      {
		fseek (inputfile, pos, 0);
		break;
	      }
	    strcat (bufptr, localbuf);
	  }
      }
    return bufptr;
  }
  if (fetchline(buffer)) return buffer;
  subnode = NULL;
  oldlinelength = linelength = lineindex = 0;
  if (searching) printf("? ");
  else putprompt();
  if ((autologout = getvar("autologout")) == NULL) seconds = 0;
  else seconds = 60 * atoi(autologout);
  raw();
  forever
  {
    fflush(stdout);
    if ((c = termgetc(inputfile)) == CTRL_D AND linelength == 0)
    {
      cooked();
      return NULL;
    }
    if (c == '\n' OR c == '\r')
    {
      lineindex = linelength;
      unless (lineindex > 0 AND linebuffer[lineindex - 1] == '\\')
      {
        unless (insertchar('\n')) putchar(BELL);
        linebuffer[linelength] = '\0';
        strcpy(buffer, linebuffer);
        storeline(buffer);
	cooked();
        return buffer;
      }
      c = '\n';
      action = insertchar;
    }
#ifdef HELIOS
    else if (c == CSI) action = handlecsi;
    else if ((c & 0x80) == 0x80) action = insertchar;
#endif
    else if (c == DELETE) action = deletechar;
    else if (iscntrl(c)) action = controlkeyaction[c];
    else action = insertchar;
    unless ((*action)(c)) putchar(BELL);
    update();
  }
}

BOOL nothing()
{
  return FALSE;
}

BOOL ctrlc()
{
  printf("^C\n\n");
  recover();
  return TRUE;
}

#ifdef HELIOS
BOOL handlecsi()
{
  int c;
  switch (termgetc(inputfile))
  {
    case 'A':
    return up();

    case 'B':
    return down();

    case 'C':
    return right();

    case 'D':
    return left();

    case 'H':
    return startofline();
    
    case '2':
    if (termgetc(inputfile) == 'z') return endofline();
    else return FALSE;

    case ' ':
    c = termgetc(inputfile);
    if ((c != '@' AND c != 'A') OR termgetc(inputfile) != '~') return FALSE;
    if (c == '@') return rightword();
    else return leftword();
    default:
    return FALSE;
  }
}
#endif

void replaceline()
{
  int index = lineindex;

  lineindex = linelength = strlen(linebuffer);
  for (; index > 0; index--) putchar(BACKSPACE);
  for (; index < linelength; index++) putchar(linebuffer[index]);
  for (; index < oldlinelength; index++) putchar(SPACE);
  for (; index > lineindex; index--) putchar(BACKSPACE);
  oldlinelength = linelength;
}

void putline()
{
  int index = 0;
  for (;index < linelength; index++) putchar(linebuffer[index]);
  for (;index > lineindex; index--) putchar(BACKSPACE);
}

BOOL complete()
{
  char buffer[WORD_MAX + 1];
  BOOL result;
  int length = getthisword(buffer);

  length = removetoken(buffer);

  if (incmd()) result = completecmd(buffer);
  else result = completefile(buffer);
  inserttext(buffer + length);
  return result;
}

BOOL display()
{
  char buffer[WORD_MAX + 1];

  printf("^D\n");
  ignore getthisword(buffer);
  if (incmd()) listcmds(buffer);
  else listfiles(buffer);
  putprompt();
  putline();
  return TRUE;
}

BOOL retrieve()
{
  char **argv;
  char buffer[LINE_MAX + 1];
  int length = getthisword(buffer);

  if ((argv = findhistory(buffer)) == NULL) return FALSE;
  sputargv(buffer, argv, SPACE);
  inserttext(buffer + length);
  return TRUE;
}

BOOL refresh()
{
  printf("^R\r\n");
  putline();
  return TRUE;
}

int getthisword(char *buffer)
{
  int index = lineindex;
  int length = 0;

  until (index == 0 OR isspace(linebuffer[index - 1])) index--;
  while (index < lineindex) buffer[length++] = linebuffer[index++];
  buffer[length] = '\0';
  return length;
}

BOOL incmd()
{
  int index = lineindex - 1;
  int c;

  until (index < 0 OR isspace(linebuffer[index]))
  {
    if (linebuffer[index--] == '/') return FALSE;
  }
  if (index < 0) return TRUE;
  while (index >= 0 AND isspace(linebuffer[index])) index--;
  if (index < 0) return TRUE;
  c = linebuffer[index];
  if (c == ';' OR c == '&' OR c == '|') return TRUE;
  return FALSE;
}

BOOL undo()
{
  linebuffer[0] = '\0';
  replaceline();
  return TRUE;
}

void inserttext(char *text)
{
  int c;
  int length = strlen(text);

  if (linelength + length <= LINE_MAX)
  {
    memmove(linebuffer + lineindex + length, linebuffer + lineindex,
            linelength - lineindex);
    until ((c = *text++) == '\0')
    {
      putchar(c);
      linebuffer[lineindex++] = c;
    }
    linelength += length;
  }
}

void putprompt()
{

  char *prompt;
  ARGV argv;
  jmp_buf savehome;

#ifdef DEBUGGING
  DEBUG("putprompt()");
#endif

  if ((prompt = getvar("prompt")) == NULL) return;
  /* ACE: if we get an error in fullsub() !!! */
  /* sub funcs should return NULL upon error */

  memcpy (savehome, home, sizeof (jmp_buf));
  if(catch())
  {
	memcpy(home, savehome, sizeof (jmp_buf));
	return;
  }
  argv = smallsub(makeargv(prompt));
  memcpy (home, savehome, sizeof (jmp_buf));

  unless ((prompt = argv[0]) == NULL)
  {
    int c;

    until ((c = *prompt++) == '\0')
    {
      switch (c)
      {
        case '\\':
        c = *prompt;
        if (c == '!' OR c == '?')
        {
          prompt++;
          putchar(c);
        }
        else putchar('\\');
        break;

        case '!':
        printf("%d", eventnumber);
        break;

        default:
        putchar(c);
        break;
      }
    }
    fflush(stdout);
  }
}

BOOL left()
{
  if (lineindex == 0) return FALSE;
  putchar(BACKSPACE);
  lineindex--;
  return TRUE;
}

BOOL right()
{
  if (lineindex == linelength) return FALSE;
  putchar(linebuffer[lineindex]);
  lineindex++;
  return TRUE;
}

BOOL startofline()
{
  while (left());
  return TRUE;
}

BOOL endofline()
{
  while (right());
  return TRUE;
}

BOOL up()
{
  if (subnode == NULL)
  {
    if (historylist.Tail->Prev == NULL) return FALSE;
    subnode = (SUBNODE *)historylist.Tail;
    linebuffer[linelength] = '\0';
    strcpy(savebuffer, linebuffer);
    sputargv(linebuffer, subnode->argv, SPACE);
  }
  else
  {
    if (subnode->prev->prev == NULL) return FALSE;
    subnode = subnode->prev;
    sputargv(linebuffer, subnode->argv, SPACE);
  }
  replaceline();
  return TRUE;
}

BOOL down()
{
  if (subnode == NULL) return FALSE;
  if (subnode->next->next == NULL)
  {
    subnode = NULL;
    strcpy(linebuffer, savebuffer);
  }
  else
  {
    subnode = subnode->next;
    sputargv(linebuffer, subnode->argv, SPACE);
  }
  replaceline();
  return TRUE;
}

BOOL deletechar()
{
  if (lineindex == linelength) return FALSE;
  linelength--;
  memmove(linebuffer + lineindex, linebuffer + lineindex + 1, 
          linelength - lineindex);
  return TRUE;
}

BOOL erasechar()
{
  if (lineindex == 0) return FALSE;
  lineindex--;
  linelength--;
  memmove(linebuffer + lineindex, linebuffer + lineindex + 1,
          linelength - lineindex);
  putchar(BACKSPACE);
  return TRUE;
}

BOOL rightword()
{
  until (isspace(linebuffer[lineindex])) unless (right()) return TRUE;
  while (isspace(linebuffer[lineindex])) unless (right()) return TRUE;
  return TRUE;
}

BOOL leftword()
{
  do unless (left()) return TRUE; while (isspace(linebuffer[lineindex]));
  do unless (left()) return TRUE; until (isspace(linebuffer[lineindex]));
  ignore right();
  return TRUE;
}

BOOL deleteword()
{
  until (isspace(linebuffer[lineindex])) unless (deletechar()) return TRUE;
  while (isspace(linebuffer[lineindex])) unless (deletechar()) return TRUE;
  return TRUE;
}

BOOL eraseword()
{
  do unless (erasechar()) return TRUE; while (isspace(linebuffer[lineindex]));
  do unless (erasechar()) return TRUE; until (isspace(linebuffer[lineindex]));
  ignore right();
  return TRUE;
}

BOOL deletetoend()
{
  while (deletechar());
  return TRUE;
}

BOOL insertchar(int c)
{
  if (linelength >= LINE_MAX) return FALSE;
  memmove(linebuffer + lineindex + 1, linebuffer + lineindex,
          linelength - lineindex);
  linebuffer[lineindex++] = c;
  linelength++;
  putchar(c);
  return TRUE;
}

void update()
{
  unless (linelength == oldlinelength)
  {
    int i = lineindex;

    for (; i < linelength; i++) putchar(linebuffer[i]);
    for (; i < oldlinelength; i++) putchar(SPACE);
    for (; i > lineindex; i--) putchar(BACKSPACE);
    oldlinelength = linelength;
  }
}

#define SAVE_MAX 100
#define nextslot(s) if (++(s) == SAVE_MAX) (s) = 0

char *linevector[SAVE_MAX];
int firstslot = 0;
int lastslot = 0;
int currentslot = 0;

void storeline(char *line)
{
  linevector[lastslot] = strdup(line);
  nextslot(lastslot);
  if (firstslot == lastslot)
  {
    freememory((int *)linevector[firstslot]);
    nextslot(firstslot);
  }
  currentslot = lastslot;
}

BOOL fetchline(char *line)
{
  if (currentslot == lastslot) return FALSE;
  strcpy(line, linevector[currentslot]);
  nextslot(currentslot);
  return TRUE;
}

void rewindinput()
{
  if (interactive) currentslot = firstslot;
  else fseek(inputfile, 0L, 0);
}

void resetinput()
{
  if (interactive) currentslot = lastslot;
}

long note()
{
  if (interactive) return currentslot;
  else return ftell(inputfile);
}

void point(long position)
{
  if (interactive) currentslot = (int) position;
  else fseek(inputfile, position, 0);
}

void freelinevector()
{
  until (firstslot == lastslot)
  {
    freememory((int *)linevector[firstslot]);
    nextslot(firstslot);
  }
}
