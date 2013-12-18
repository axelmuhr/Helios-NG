/**
*
* Title:  Helios Debugger - Line editting.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/line.c,v 1.4 1993/03/19 16:51:19 nickc Exp $";
#endif

#include "tla.h"

PRIVATE BOOL nothing(void);
PRIVATE void putline(LINE *);
PRIVATE void update(LINE *);
PRIVATE void saveline(LINE *);
PRIVATE BOOL fetchnext(LINE *);
PRIVATE BOOL fetchprev(LINE *);
PRIVATE BOOL insertchar(LINE *, int);
PRIVATE BOOL leftword(LINE *);
PRIVATE BOOL rightword(LINE *);
PRIVATE BOOL undo(LINE *);
PRIVATE BOOL left(LINE *);
PRIVATE BOOL right(LINE *);
PRIVATE BOOL up(LINE *);
PRIVATE BOOL down(LINE *);
PRIVATE BOOL deletechar(LINE *);
PRIVATE BOOL erasechar(LINE *);
PRIVATE BOOL deleteword(LINE *);
PRIVATE BOOL eraseword(LINE *);
PRIVATE BOOL deletetoend(LINE *);
PRIVATE BOOL refresh(LINE *);
PRIVATE BOOL startofline(LINE *);
PRIVATE BOOL endofline(LINE *);

PRIVATE BOOL (*keyaction[32])() =
{
  nothing,
  startofline,
  left,
  nothing,
  nothing,
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
  refresh,
  nothing,
  nothing,
  undo,
  nothing,
  rightword,
  deleteword,
  eraseword,
  nothing,
  nothing,
  nothing,
  nothing,
  nothing,
  nothing
};

/**
*
* line = newline(display);
*
* Initialise line data structure.
*
**/
PUBLIC LINE *newline(DISPLAY *display)
{
  LINE *line = NEW(LINE);
  int slot;

  line->display = display;
  line->firstslot = line->lastslot = line->currentslot = 0;
  for (slot = 0; slot < SaveMax; slot++) line->vec[slot] = NULL;
  initkeymap(line->keymap);
  return line;
}

/**
*
* remline(line);
*
* Tidyup line data structures.
*
**/
PUBLIC void remline(LINE *line)
{
  int slot;

  for (slot = 0; slot < SaveMax; slot++)
    unless (line->vec[slot] == NULL) freemem(line->vec[slot]);
  freekeymap(line->keymap);
  freemem(line);
}

/**
*
* c = dgetkey(display);
*
* Get a key sequence.
*
**/
PUBLIC int dgetkey(DISPLAY *display)
{
  int c;

  unless ((c = dgetc(display)) == CSI) return c;
  switch (c = dgetc(display))
  {
    case 'A':
    return UpArrow;

    case 'B':
    return DownArrow;

    case 'C':
    return RightArrow;

    case 'D':
    return LeftArrow;

    case 'H':
    return Home;

#ifdef NO_LONGER_WORKS
    case 'S':
    (void)dgetc(display);
    return ShiftDownArrow;

    case 'T':
    (void)dgetc(display);
    return ShiftUpArrow;
#endif
    
    case '?':
    (void)dgetc(display);
    return Help;

    case '@':
    return VInsert;

#ifdef NO_LONGER_WORKS
    case ' ':
    if (dgetc(display) == '@') c = ShiftRightArrow;
    else c = ShiftLeftArrow;
    (void)dgetc(display);
    return c;
#endif
    
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      int val = 0;

      do
      {
        val = (val * 10) + (c - '0');
      } until ((c = dgetc(display)) == '~' OR c == 'z');
      if (c == '~') return FunctionKeys + val;
      return PageKeys + val;
    }

    default:
    until (iscst(c)) c = dgetc(display);
    return CSI;
  }
}

/**
*
* getinput(line, buffer, prompt);
*
* Get input for dialog command.
*
**/
PUBLIC char *getinput(LINE *line, char *buffer, char *prompt)
{
  DISPLAY *display = line->display;
  int c;
  int i = 0;

  dlock(display);
  unless (prompt == NULL)
  {
    until ((c = *prompt++) == '\0') xdputc(display, c);
  }
  deol(display);
  dunlock(display);
  until ((c = dgetkey(display)) == '\n' OR c == '\r')
  {
    if (c == Escape OR c == EOF)
    {
      dlock(display);
      xdputc(display, '\r');
      deol(display);
      dunlock(display);
      return NULL;
    }
    dlock(display);
    if (isprint(c))
    {
      xdputc(display, c);
      buffer[i++] = c;
    }
    else if (c == Backspace AND i > 0)
    {
      i--;
      xdputc(display, Backspace);
      deol(display);
    }
    else xdputc(display, Bell);
    dunlock(display);
  }
  buffer[i] = '\0';
  dlock(display);
  xdputc(display, '\r');
  deol(display);
  dunlock(display);
  return buffer;
}

/**
*
* text = getline(line);
*
* Get line of input.
*
**/
PUBLIC char *getline(LINE *line)
{
  DISPLAY *display = line->display;
  BOOL (*action)();
  BOOL virgin = TRUE;
  char *cmd;
  int c;

  line->length = line->index = 0;
  line->update = FALSE;
  forever
  {
    if ((c = dgetkey(display)) == EOF) return NULL;
    if (virgin)
    {
      virgin = FALSE;
      dlock(display);
      deol(display);
      dunlock(display);
    }
    if (line->length == 0 AND (cmd = getkey(line->keymap, c)) != NULL) return cmd;
    switch (c)
    {
      case UpArrow:
#ifdef NO_LONGER_WORKS
      case ShiftUpArrow:
#endif
      action = up;
      break;

      case DownArrow:
#ifdef NO_LONGER_WORKS      
      case ShiftDownArrow:
#endif
      action = down;
      break;

      case RightArrow:
#ifdef NO_LONGER_WORKS
      case ShiftRightArrow:
#endif
      action = right;
      break;

      case LeftArrow:
#ifdef NO_LONGER_WORKS
      case ShiftLeftArrow:
#endif
      action = left;
      break;

      case Home:
      action = startofline;
      break;

      case Help:
      case VInsert:
      action = nothing;
      break;

      case '\n':
      case '\r':
      line->index = line->length;
#ifdef OLDCODE
      if (line->index > 0 AND line->buffer[line->index - 1] == '\\')
      {
        action = insertchar;
        break;
      }
#endif
      dlock(display);
      xdputc(display, '\r');
      dunlock(display);
      line->buffer[line->length] = '\0';
      saveline(line);
      return line->buffer;

      default:
      if ((c & 0x80) == 0x80) action = nothing;
      else if (c == VDelete) action = deletechar;
      else if (iscntrl(c)) action = keyaction[c];
      else action = insertchar;
      break;
    }
    dlock(display);
    unless ((*action)(line, c)) xdputc(display, Bell);
    if (line->update) update(line);
    dunlock(display);
  }
}

PRIVATE BOOL nothing(void)
{
  return FALSE;
}

PRIVATE void putline(LINE *line)
{
  DISPLAY *display = line->display;
  int index = line->index;

  line->index = line->length = strlen(line->buffer);
  for (; index > 0; index--) xdputc(display, Backspace);
  for (; index < line->length; index++) xdputc(display, line->buffer[index]);
  deol(display);
  for (; index > line->index; index--) xdputc(display, Backspace);
}

PRIVATE BOOL refresh(LINE *line)
{
  DISPLAY *display = line->display;
  int index = line->index;

  for (; index > 0; index--) xdputc(display, Backspace);
  for (;index < line->length; index++) xdputc(display, line->buffer[index]);
  deol(display);
  for (;index > line->index; index--) xdputc(display, Backspace);
  return TRUE;
}

PRIVATE BOOL undo(LINE *line)
{
  line->buffer[0] = '\0';
  putline(line);
  return TRUE;
}

PRIVATE BOOL left(LINE *line)
{
  if (line->index == 0) return FALSE;
  xdputc(line->display, Backspace);
  line->index--;
  return TRUE;
}

PRIVATE BOOL right(LINE *line)
{
  if (line->index == line->length) return FALSE;
  xdputc(line->display, line->buffer[line->index]);
  line->index++;
  return TRUE;
}

PRIVATE BOOL startofline(LINE *line)
{
  while (left(line));
  return TRUE;
}

PRIVATE BOOL endofline(LINE *line)
{
  while (right(line));
  return TRUE;
}

PRIVATE BOOL up(LINE *line)
{
  unless (fetchprev(line)) return FALSE;
  putline(line);
  return TRUE;
}

PRIVATE BOOL down(LINE *line)
{
  unless (fetchnext(line)) return FALSE;
  putline(line);
  return TRUE;
}

PRIVATE BOOL deletechar(LINE *line)
{
  if (line->index == line->length) return FALSE;
  line->length--;
  memmove(line->buffer + line->index, line->buffer + line->index + 1, 
          line->length - line->index);
  line->update = TRUE;
  return TRUE;
}

PRIVATE BOOL erasechar(LINE *line)
{
  if (line->index == 0) return FALSE;
  line->index--;
  line->length--;
  memmove(line->buffer + line->index, line->buffer + line->index + 1,
          line->length - line->index);
  xdputc(line->display, Backspace);
  line->update = TRUE;
  return TRUE;
}

PRIVATE BOOL rightword(LINE *line)
{
  until (isspace(line->buffer[line->index])) unless (right(line)) return TRUE;
  while (isspace(line->buffer[line->index])) unless (right(line)) return TRUE;
  return TRUE;
}

PRIVATE BOOL leftword(LINE *line)
{
  do unless (left(line)) return TRUE; while (isspace(line->buffer[line->index]));
  do unless (left(line)) return TRUE; until (isspace(line->buffer[line->index]));
  (void)right(line);
  return TRUE;
}

PRIVATE BOOL deleteword(LINE *line)
{
  until (isspace(line->buffer[line->index])) unless (deletechar(line)) return TRUE;
  while (isspace(line->buffer[line->index])) unless (deletechar(line)) return TRUE;
  return TRUE;
}

PRIVATE BOOL eraseword(LINE *line)
{
  do unless (erasechar(line)) return TRUE; while (isspace(line->buffer[line->index]));
  do unless (erasechar(line)) return TRUE; until (isspace(line->buffer[line->index]));
  (void)right(line);
  return TRUE;
}

PRIVATE BOOL deletetoend(LINE *line)
{
  while (deletechar(line));
  return TRUE;
}

PRIVATE BOOL insertchar(LINE *line, int c)
{
  unless (isprint(c) OR c == '\n') return FALSE;
  if (line->length >= LineMax) return FALSE;
  memmove(line->buffer + line->index + 1, line->buffer + line->index,
          line->length - line->index);
  line->buffer[line->index++] = c;
  line->length++;
  if (c == '\n') xdputc(line->display, '\r');
  xdputc(line->display, c);
  line->update = TRUE;
  return TRUE;
}

PRIVATE void update(LINE *line)
{
  DISPLAY *display = line->display;
  int index = line->index;

  for (; index < line->length; index++) xdputc(display, line->buffer[index]);
  deol(display);
  for (; index > line->index; index--) xdputc(display, Backspace);
  line->update = FALSE;
}

PRIVATE void saveline(LINE *line)
{
  if (strlen(line->buffer) == 0) return;
  line->vec[line->lastslot] = strdup(line->buffer);
  nextslot(line->lastslot);
  if (line->firstslot == line->lastslot)
  {
    freemem(line->vec[line->firstslot]);
    line->vec[line->firstslot] = NULL;
    nextslot(line->firstslot);
  }
  line->currentslot = line->lastslot;
}

PRIVATE BOOL fetchprev(LINE *line)
{
  if (line->currentslot == line->firstslot) return FALSE;
  if (line->currentslot == line->lastslot)
  {
    line->buffer[line->length] = '\0';
    strcpy(line->savebuffer, line->buffer);
  }
  prevslot(line->currentslot);
  strcpy(line->buffer, line->vec[line->currentslot]);
  return TRUE;
}

PRIVATE BOOL fetchnext(LINE *line)
{
  if (line->currentslot == line->lastslot) return FALSE;
  nextslot(line->currentslot);
  if (line->currentslot == line->lastslot) strcpy(line->buffer, line->savebuffer);
  else strcpy(line->buffer, line->vec[line->currentslot]);
  return TRUE;
}
