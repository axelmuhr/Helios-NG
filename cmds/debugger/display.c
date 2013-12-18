/**
*
* Title:  Helios Debugger - Display handling.
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
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/display.c,v 1.7 1993/03/17 17:39:51 nickc Exp $";
#endif

#include "tla.h"

PRIVATE void breakhandler(DISPLAY *);
PRIVATE void dgetsize(DISPLAY *);
PRIVATE void winfo(WINDOW *);
PRIVATE void highlight(WINDOW *);
PRIVATE BOOL inwindow(WINDOW *, LOCATION);
PRIVATE void vrefresh(DISPLAY *);
PRIVATE void cursorshow(WINDOW *);
PRIVATE void cursorhide(WINDOW *);
PRIVATE void cursormove(WINDOW *, int, int);

/*
-- crf : 15/08/81 - not happy with this function ...
*/
#ifdef PARSYTEC
PRIVATE void vwupdate (WATCHPOINT *, DISPLAY *);
#endif


/**
*
* display = dopen(debug, name);
*
* Create a new display environment.
*
**/
#ifndef BUG
/* PRIVATE BOOL stop_display; CR: this was a big fault MR E */
PRIVATE BOOL in_read = FALSE;
#endif

PUBLIC DISPLAY *dopen(DEBUG *debug, char *name, char *cserver)
{
  DISPLAY *display = NEW(DISPLAY);
  char windowname[PathMax + 1];

  
#ifndef BUG
  display->stop_display = FALSE;
#endif

  strcpy(windowname, cserver);
  strcat(windowname,"/");
  strcat(windowname, name);
  
#ifdef V1_1
  /* JMP 08/03/90
     my_fdopen has an increased number of filedescriptors available as well
     as having locking on iob table to allow multiple access
  */
  display->filein  = my_fdopen(open(windowname, O_CREAT | O_RDONLY), "r");
  display->fileout = my_fdopen(open(Heliosno(display->filein)->Name, O_WRONLY), "w");
#else
  display->filein  = fdopen(open(windowname, O_CREAT | O_RDONLY), "r");
  display->fileout = fdopen(open(Heliosno(display->filein)->Name, O_WRONLY), "w");
#endif
  
  setvbuf(display->filein, NULL, _IONBF, 0);
  
#ifdef SETVBUF_FIXED
  setvbuf(display->fileout, NULL, _IOFBF, BUFSIZ);
#else
#ifdef OLDCODE
  setvbuf(display->fileout, Malloc(BUFSIZ), _IOFBF, BUFSIZ);
#endif
#endif

  raw(display);
  
  /* ACE: Force the window to appear. */
  
  write(fileno(display->fileout), " ", 1);
  
  dgetsize(display);
  
  display->row    = display->height - 1;
  display->col    = 0;
  display->topwin = NULL;
  
  InitList(&display->windowlist);
  InitSemaphore(&display->lock, 1);
  
  display->varsize = 0;
/*
-- crf : 14/08/91 - keep track of last line in watch window
*/
  display->varline = 0;

  display->debug = debug;
  display->breakflag = FALSE;

  unless ((display->breakport = EnableEvents(Heliosno(display->filein), Event_Break)) == NULL)
    Fork( 20000, breakhandler, sizeof(DISPLAY *), display);
  
  return display;
}

/**
*
* breakhandler(dislay);
*
* Handle break events for display.
*
**/
PRIVATE void breakhandler(DISPLAY *display)
{
  MCB mcb;
  byte data[IOCDataMax];

  InitMCB(&mcb, 0, display->breakport, NullPort, 0);
  mcb.MsgHdr.DataSize = IOCDataMax;
  mcb.Data = data;
  mcb.Timeout = -1;
  while (GetMsg(&mcb) > Err_Null) display->breakflag = TRUE;
}

/**
*
* flag = testbreak(display);
*
* Return breakflag and clear it.
*
**/
PUBLIC BOOL testbreak(DISPLAY *display)
{
  BOOL breakflag = display->breakflag;

  display->breakflag = FALSE;
  return breakflag;
}

/**
*
* dclose(display);
*
* Close a display environment.
*
**/
PUBLIC void dclose(DISPLAY *display)
{
  
  char name[PathMax + 1];
  strcpy(name, Heliosno(display->filein)->Name);

#ifndef BUG
  /* JMP */
  /* the getc routine now has the responsibility of closing the window down */
  /* force the getc to get a EOF to initiate the action */

  if(in_read == TRUE)
  {
    /* There is a pending read in operation we must signal the dgetc() function
    to close the file descriptors and close the window down on completion */
    display->stop_display = TRUE;
    SendException(Heliosno(display->filein)->Reply, ReadRc_EOF);
  }
  else
  {
    /* as there is not a pending read we must have requested the close
    from either a quit or free command therefore we take responsibility
    for closing the window */
    display->stop_display = TRUE;
    fclose(display->filein);
    fclose(display->fileout);
  
    (void)WalkList(&display->windowlist, (WordFnPtr)wclose, 0);
    unless (display->varsize == 0) freemem(display->varvec);
    FreePort(display->breakport);
    freemem(display);
    Delete(cdobj(),name);
  }
      
#else
  fclose(display->filein);
  fclose(display->fileout);
  
  (void)WalkList(&display->windowlist, (WordFnPtr)wclose, 0);
  unless (display->varsize == 0) freemem(display->varvec);
  FreePort(display->breakport);
  freemem(display);
  Delete(cdobj(),name);
#endif
}

/**
*
* drefresh(display);
*
* Refresh whole of display.
*
**/
PRIVATE void wrefill(WINDOW *window)
{
  DISPLAY *display = window->display;

  debugf("wrefill()");
  dcursor(display, window->pos, 0);
  debugf("list(%s, %d, %d)", window->loc.module->name, window->loc.line, window->size - 1);
#ifdef OLDCODE
  list(display, getsource(window->loc.module), window->loc.line, window->size - 1);
#else
  {
    SOURCE *src ;
/*
-- crf : 26/09/91 - (related to) Bug 677
-- if source files cannot be located, tell the user about it
-- (a bit crude ...)
*/
    src = getsource(window->loc.module) ; 
    if (src == NULL)
    {
      dcursor (display, 5, 7) ;
      dprintf (display, "Unable to locate source files in current directory\n");
      dcursor (display, 7, 7) ;
      dprintf (display, "After quitting this session, either");
      dcursor (display, 8, 7) ;
      dprintf (display, "1. change the working directory, or") ;
      dcursor (display, 9, 7) ;
      dprintf (display, "2. set the environment variable %s ", DBGSRC_VARNAME);
      dprintf (display, "to the name of the") ;
      dcursor (display, 10, 7) ;
      dprintf (display, "   appropriate directory") ;
    }
    else
    {
      list(display, src, window->loc.line, window->size - 1);
      cursorshow(window);
      if (inwindow(window, window->progloc)) highlight(window);
    }
  }
#endif
}

PRIVATE void wrefresh(WINDOW *window)
{
  debugf("wrefresh()");
  wrefill(window);
  winfo(window);
}

PUBLIC void drefresh(DISPLAY *display)
{
/* -- crf : 07/08/91 - not used -
  int oldheight = display->height;
*/

  dgetsize(display);
  display->row = display->height - 1;
  /* ACE: Should change size and position of each window */
  unless (display->varsize == 0) vrefresh(display);
  (void)WalkList(&display->windowlist, (WordFnPtr)wrefresh, 0);
  dcursor(display, display->row, display->col);
  deol(display);
}

/**
*
* dgetsize(display);
*
* Discover the dimensions of the display.
*
**/
PRIVATE void dgetsize(DISPLAY *display)
{
  Attributes attr;

  GetAttributes(Heliosno(display->fileout), &attr);
  display->height = attr.Min;
  display->width = attr.Time;
}

/**
*
* dcursor(display, row, col);
*
* Position cursor on display.
*
**/
PUBLIC void dcursor(DISPLAY *display, int row, int col)
{
  /* ACE: I would like only to flush on unlocking display and inputting
          but I must flush before writing a escape sequence so that it
          is entirely in a single write.
  */
  fflush(display->fileout);
  dprintf(display, "\233%d;%dH", row + 1, col + 1);
}

/**
*
* dstart(display);
*
* Takeover the display.
*
**/
PUBLIC void dstart(DISPLAY *display)
{
  dlock(display);
  dclear(display);
  cooked(display);
}

/**
*
* dend(display, prompt);
*
* Release the display.
*
**/
PUBLIC void dend(DISPLAY *display, BOOL prompt)
{
  raw(display);
  if (prompt)
  {
    dprintf(display, "Press RETURN to continue...");
    fflush(display->fileout);
    (void)dgetc(display);
  }
  drefresh(display);
  dunlock(display);
}

/**
*
* dclear(display);
*
* Clear the whole display;
*
**/
PUBLIC void dclear(DISPLAY *display)
{
  dcursor(display, 0, 0);
  fflush(display->fileout);
  dprintf(display, "\233J");
  fflush(display->fileout);
}

/**
*
* dinverse(display);
*
* Put display into inverse video.
*
**/
PUBLIC void dinverse(DISPLAY *display)
{
  fflush(display->fileout);
  dprintf(display, "\2337m");
}

/**
*
* dnormal(display);
*
* Put display into normal video.
*
**/
PUBLIC void dnormal(DISPLAY *display)
{
  fflush(display->fileout);
  dprintf(display, "\2330m");
}

/**
*
* deol(DISPLAY *display);
*
* Delete to end of line.
*
**/
PUBLIC void deol(DISPLAY *display)
{
  fflush(display->fileout);

  /* IOdebug( "e ... %" );  */
  
  dprintf(display, "\233K");
  
  /* IOdebug( "l" );  */
}

/**
*
* dlock(display);
*
* Lock the display, so that no one else can move the cursor.
*
**/
PUBLIC void dlock(DISPLAY *display)
{
  Wait(&display->lock);
  if (TestSemaphore(&display->lock) > 0)
    IOdebug( "TLA: dlock: Not locked (%d)", TestSemaphore( &display->lock ));
}

/**
*
* dunlock(display);
*
* Unlock the display.
*
**/
PUBLIC void dunlock(DISPLAY *display)
{
  dcursor(display, display->row, display->col);
  fflush(display->fileout);
  Signal(&display->lock);
}

/**
*
* dprintf(display, format, ...);
*
* Formatted output to display.
*
**/
PUBLIC void dprintf(DISPLAY *display, char *format, ...)
{
  va_list args;

  if (TestSemaphore(&display->lock) > 0)
    IOdebug( "TLA: dprintf: Not locked (%d)", TestSemaphore( &display->lock ));
  va_start(args, format);
  vfprintf(display->fileout, format, args);
  va_end(args);

  return;
}

/**
*
* xdputc(display, c);
*
* Put a character to the display.
*
**/
PUBLIC void xdputc(DISPLAY *display, int c)
{
  if (TestSemaphore(&display->lock) > 0)
    IOdebug( "TLA: xdputc: Not locked (%d)", TestSemaphore( &display->lock ));
  if (c == '\r') display->col = 0;
  else if (c == 0x08) display->col--; /* ACE: '\b' gives wrong value ! */
  else unless (c == 0x07) display->col++; /* ACE: '\a' does not work ! */
  fputc(c, display->fileout);
}

/**
*
* dputc(display, c);
*
* Put a character to the display.
*
**/
PUBLIC void dputc(DISPLAY *display, int c)
{
  if (TestSemaphore(&display->lock) > 0)
    IOdebug( "TLA: dputc: Not locked (%d)", TestSemaphore( &display->lock ));
  fputc(c, display->fileout);
}

/**
*
* c = dgetc(display);
*
* Get a character from the display.
*
**/

PUBLIC int dgetc(DISPLAY *display)
{
#ifndef BUG

    /* JMP this fix allows the window manager to time out on a read which
    enables it to process any close commands it might have received */
    
    char buff[1];
    word  no_read;

    in_read = TRUE;

    if(display->stop_display == TRUE)
    {   /* window has closed do not attempt any reads */
    	in_read = FALSE;
        return(EOF);
    }
    
    do
    {
      /* check that a child process has not locked the read stream */
      no_read = Read(Heliosno(display->filein),buff,1,OneSec);
    }while(no_read == 0);

    if ( no_read == -1)
    {
      /* close down the window */
      char name[PathMax + 1];
      strcpy(name, Heliosno(display->filein)->Name);

      Delay(OneSec);
      fclose(display->filein);
      fclose(display->fileout);
  
      (void)WalkList(&display->windowlist, (WordFnPtr)wclose, 0);
      unless (display->varsize == 0) freemem(display->varvec);
      FreePort(display->breakport);
      freemem(display);
      Delete(cdobj(),name);
    }
    in_read = FALSE;
    return(buff[0]);
#else
  return(fgetc(display->filein));
#endif
}

/**
*
* raw(display);
*
* Put display into raw mode.
*
**/
PUBLIC void raw(DISPLAY *display)
{
  Attributes attr;
  Stream *console = Heliosno(display->fileout);

  GetAttributes(console, &attr);
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsolePause);
  AddAttribute(&attr, ConsoleRawInput);
  AddAttribute(&attr, ConsoleRawOutput);
  RemoveAttribute(&attr, ConsoleIgnoreBreak);
  AddAttribute(&attr, ConsoleBreakInterrupt);
  SetAttributes(console, &attr);
}

/**
*
* cooked(display);
*
* Put display into cooked mode.
*
**/
PUBLIC void cooked(DISPLAY *display)
{
  Attributes attr;
  Stream *console = Heliosno(display->fileout);

  /*
  * ACE: There is a problem with stream locking.
  */
  GetAttributes(console, &attr);
  AddAttribute(&attr, ConsoleEcho);
  AddAttribute(&attr, ConsolePause);
  RemoveAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsoleRawOutput);
  SetAttributes(console, &attr);
}

/**
*
* vrefresh(display);
*
* Refresh the variable watch window.
*
**/
PRIVATE void vrefresh(DISPLAY *display)
{
  int i;

#ifdef PARSYTEC  
  (void) WalkList(&display->debug->watchpointlist, (WordFnPtr) vwupdate, (word) display);

/*  for (i = 0; i < display->varsize; i++)
  {

-- crf : 24/07/91 - Bug 708
-- Test if varvec entry is NULL before displaying watchpoint (see note in 
-- "vinsert()" regarding setting entries to NULL)

-- if (display->varvec[i] != NULL)

    dcursor(display, i, 0);
    dprintf(display, "%d) ", i);
    putwatchpoint(display->varvec[i], display->debug);
    deol(display);

  }  CR: this is better done by Walk */
#endif

/*
-- crf : 15/08/91 - this works better
*/
  {
    int watch_line_no = 0 ;
    for (i = 0 ; i < display->varsize ; i++)
    {
      debugf("called vrefresh");
      dcursor(display, watch_line_no, 0);
      dprintf(display, "%d) ", i);
      putwatchpoint(display->varvec[i], display->debug);
      deol(display);
      watch_line_no += (1 + display->varvec[i]->num_elements) ;
    }
  }

#ifdef OLDCODE
  dcursor(display, display->varsize, 0);
#endif
/*
-- crf : 14/08/91 - note use of "varline"
*/
  dcursor(display, display->varline, 0);

  dinverse(display);
  for (i = 0; i < display->width; i++) dputc(display, '-');
  dnormal(display);
}

/**
-- crf : 24/07/91 - Bug 708
-- Additional parameter (incr) for creating variable size watch windows for
-- structures.
-- Include incr in all window resizing commands.
*
* vgrow (display, incr);
*
* Increase the size of the variable watch window.
*
**/
PUBLIC void vgrow(DISPLAY *display, int incr)
{
  int i;

#ifdef CRFOLD
  if (display->varsize == 0)
#endif
/*
-- crf : 14/08/91 - better to use "varline"
*/
  if (display->varline == 0)
  {
    WINDOW *window = (WINDOW *)display->windowlist.Head;

    unless (window->node.Next == NULL)
    {
      unless (window->size > 3) return;
      window->pos += (2 + incr) ;
      window->size -= (2 + incr);
      window->loc.line += (2 + incr) ;
    }
    display->varvec = (struct watchpoint **)newmem(sizeof(struct watchpoint *) * 80);
  }
  else
  {
    WINDOW *window = (WINDOW *)display->windowlist.Head;

    unless (window->node.Next == NULL)
    {
      unless (window->size > 2) return;
      window->pos += (1 + incr) ;
      window->size -= (1 + incr) ;
      window->loc.line += (1 + incr) ;
    }
  }
/*
-- crf : 14/08/91 - update "varline"
*/
  display->varline += (incr + 1) ;
  dcursor(display, display->varline, 0);
#ifdef CRFOLD
  dcursor(display, display->varsize + incr, 0);
#endif

  display->varsize ++ ;
  dinverse(display);
  for (i = 0; i < display->width; i++) dputc(display, '-');
  dnormal(display);
}

/**
-- crf : 24/07/91 - Bug 708
-- Additional parameter (decr) for deleting variable size watch windows (used
-- for deleting watchpointed structures).
-- Include decr in all window resizing commands.
*
* vshrink(display, decr);
*
* Decrease the size of the variable watch window.
*
**/
PUBLIC void vshrink(DISPLAY *display, int decr)
{
  int i;

#ifdef OLDCODE
  if (display->varsize == 0) return;
  if (display->varsize == (1 + decr))
#endif
/*
-- crf : 14/08/91 - better to use "varline"
*/
  if (display->varline == 0) return;
  if (display->varline == (1 + decr))
  {
    WINDOW *window = (WINDOW *)display->windowlist.Head;

    unless (window->node.Next == NULL)
    {
      window->pos -= (2 + decr) ;
      window->size += (2 + decr);
      dcursor(display, window->pos, 0);
      if (window->loc.line > (2 + decr))
      {
        window->loc.line -= (2 + decr) ;
        list(display, getsource(window->loc.module), window->loc.line, 2 + decr);
      }
      else
      {
      	window->loc.line = 1 ;
#ifdef OLDCODE
        list(display, getsource(window->loc.module), window->loc.line, window->size);
#endif
        list(display, getsource(window->loc.module), window->loc.line, 
             window->size - 1);
      }
    }
    freemem(display->varvec);
  }
  else
  {
    WINDOW *window = (WINDOW *)display->windowlist.Head;

    unless (window->node.Next == NULL)
    {
      /* ACE: Check size of window */
      window->pos -= (1 + decr) ;
      window->size += (1 + decr) ;
      dcursor(display, window->pos, 0);
      if (window->loc.line > (1 + decr))
      {
        window->loc.line -= (1 + decr) ;
        list(display, getsource(window->loc.module), window->loc.line, 1 + decr);
      }
      else
      {
        window->loc.line = 1 ;
#ifdef OLDCODE
        list(display, getsource(window->loc.module), window->loc.line, window->size);
#endif
        list(display, getsource(window->loc.module), window->loc.line, 
             window->size - 1);
      }
    }
  }

#ifdef CRFOLD
/* -- crf : adjust varsize by number of elements */
  display->varsize -= (1 + decr) ;
 
  unless (display->varsize == 0)
  {
    dcursor(display, display->varsize, 0);
    dinverse(display);
    for (i = 0; i < display->width; i++) dputc(display, '-');
    dnormal(display);
  }
#endif

  display->varsize -- ;
  display->varline -= (1 + decr) ;

  unless (display->varline == 0)
  {
    dcursor(display, display->varline, 0);
    dinverse(display);
    for (i = 0; i < display->width; i++) dputc(display, '-');
    dnormal(display);
  }
}

/**
*
* vinsert(display, watchpoint)
*
* Add item to variable window.
*
**/
PUBLIC void vinsert(DISPLAY *display, struct watchpoint *watchpoint)
{
#ifdef CRFOLD
  int incr ;
#endif
/*
-- crf : 14/08/91 - keep track of line at which expression should be
-- displayed
*/
  int watch_line_no ;

/*
-- crf : 24/07/91 - Bug 708
-- The creation of watch windows must take into account the number of elements
-- in a watchpointed structure. 
-- The number of elements is obtained from "num_watch_elements()", and this is
-- passed into "vgrow()". 
*/
  dlock(display);
  /* ACE: vgrow() may fail */

  /* crf : num_watch_elements() is a new routine in "monitor.c" */
#ifdef CRFOLD
  incr = num_watch_elements (watchpoint, display->debug) ;
  vgrow(display, incr);
#endif
/*
-- crf : 14/08/91 - get number of elements in expression
*/
  watchpoint->num_elements = num_watch_elements (watchpoint, display->debug) ;
  watch_line_no = display->varline ;
  vgrow(display, watchpoint->num_elements);

  display->varvec[display->varsize - 1] = watchpoint;

#ifdef CRFOLD
  dcursor(display, display->varsize - 1, 0);
#endif
  dcursor(display, watch_line_no, 0);

  dprintf(display, "%d) ", display->varsize - 1);
  putwatchpoint(watchpoint, display->debug);
  deol(display);
  dunlock(display);

/*
-- crf : 14/08/91 - Ignore the following ....
*/
#ifdef CRFOLD
/* 
-- crf : 
-- The watchpoint identity and the line number at which it is displayed are
-- both given by varsize. With variable size watch windows, the identity of
-- a watchpoint will no longer be the same as its line number. Instead of 
-- attempting to keep a separate track of identities and line numbers, I have
-- taken an easier solution. Each element of the structure is stored as a 
-- NULL entry in the watchpoint table (varvec) - i.e. each element has a 
-- watchpoint identity. This means that the identity and the line number will 
-- still correspond, but there will be gaps in the numbering that is 
-- displayed. For example, the numbering will look something like :
--   0) var1 {
--        a = ...,
--        b = ...}
--   3) var2 = ....
-- I don't think this is too serious, though it could be looked at again later
-- on.
-- The following piece of code sets the varvec entries corresponding to
-- the elements of a structure to NULL.
*/
  {
    int i ;
    for (i = 0 ; i < incr ; i ++)
    {
      display->varsize ++ ;
      display->varvec[display->varsize - 1] = NULL ;
    }
  }
#endif
}

/**
*
* vdelete(display, number);
*
* Delete item from variable window.
*
**/
PUBLIC void vdelete(DISPLAY *display, int number)
{
  int i;
/*
-- crf : 24/07/91 - Bug 708
-- The deletion of watch windows must take into account the number of elements
-- in a watchpointed structure. 
-- The number of elements is obtained from "num_watch_elements()", and this is
-- passed into "vshrink()". 
*/
#ifdef CRFOLD
  int decr ; 
#endif

/*
-- crf : 13/08/91 - Bug 711
-- Attempting to delete negative watchpoint ids hangs system
*/
#ifdef CRFOLD
  unless (number < display->varsize) return;
  unless (number >= 0) return;
#endif
  unless ((number < display->varsize) && (number >= 0))
  {
    cmderr (display->debug, "Invalid watchpoint number") ;  
    return;
  }

  debugf("vdelete entered for %s", display->varvec[number]->expr);

#ifdef CRFOLD
/* -- crf : test for NULL entry (i.e. an element of a structure) */
  if (display->varvec [number] == NULL) return ;
  decr = num_watch_elements (display->varvec [number], display->debug) ;
#endif

  remwatchpoint(display->debug, display->varvec[number]);

  dlock(display);
#ifdef CRFOLD
  vshrink(display, decr);
#endif
/*
-- crf : 14/08/91 - explicitly use num_elements
*/
  vshrink(display, display->varvec[number]->num_elements);

  {
    int watch_line_no = 0 ;
    for (i = 0 ; i < display->varsize ; i ++)
    {
      if (i >= number)
      {
        display->varvec[i] = display->varvec[i + 1];
        dcursor(display, watch_line_no, 0);
        dprintf(display, "%d) ", i);
        putwatchpoint(display->varvec[i], display->debug);
        deol(display);
      }
      watch_line_no += (1 + display->varvec[i]->num_elements);
    }
  dunlock(display);
  }

#ifdef CRFOLD
  for (i = number; i < display->varsize; i++)
  {
    display->varvec[i] = display->varvec[i + 1 + decr];

/* -- crf : test for NULL entry (i.e. an element of a structure) */
    if (display->varvec [i] != NULL)
    {
      dcursor(display, i, 0);
      dprintf(display, "%d) ", i);
      putwatchpoint(display->varvec[i], display->debug);
    }
    deol(display);
#endif
}

/**
*	update display if changed watchpoints exist
**/


PUBLIC void vupdate(DISPLAY *display, WATCHPOINT *watchpoint)
{
  int i;
/*
-- crf : 14/08/91 - keep track of line at which expression is to be
-- displayed
*/
  int watch_line_no = 0 ;

  for (i = 0; i < display->varsize; i++)
  {
    if (display->varvec[i] == watchpoint)
    {
      debugf("called vupdate");
      dlock(display);
#ifdef OLDCODE
      dcursor(display, i, 0);
#endif 
      dcursor(display, watch_line_no, 0);

      dprintf(display, "%d) ", i);
      putwatchpoint(watchpoint, display->debug);
      deol(display);
      dunlock(display);
      return;
    }
    watch_line_no += (1 + display->varvec[i]->num_elements) ;
  }
}

/**
*
* window = newwindow(pos, size);
*
* Support routine for wopen().
*
**/
PRIVATE WINDOW *newwindow(int pos, int size)
{
  WINDOW *window = NEW(WINDOW);

  window->pos = pos;
  window->size = size;
  window->loc.module = NULL;
  window->loc.line = 0;
  window->progloc.module = NULL;
  window->progloc.line = 0;
  window->cur.row = 0;
  window->cur.col = 0;
  window->cur.size = 1;
  window->traceout = NULL;
  return window;
}

/**
*
* window = wopen(display);
*
* Open a window on the display.
*
**/
PUBLIC WINDOW *wopen(DISPLAY *display)
{
  WINDOW *window;
  WINDOW *topwin;

  if ((topwin = display->topwin) == NULL)
  {
    display->topwin = window = newwindow(0, display->height - 1);
    AddHead(&display->windowlist, &window->node);
  }
  else
  {
    unless (topwin->size > 3) return NULL;
    window = newwindow(topwin->pos, topwin->size >> 1);
    topwin->pos += window->size;
    topwin->size -= window->size;
    topwin->loc.line += window->size;
    PreInsert(&topwin->node, &window->node);
#ifdef NEWCODE
    cursorinprison(topwin);
#endif
#ifdef OLDCODE
    dlock(display);
    wrefill(topwin);
    dunlock(display);
#endif
  }
  window->display = display;
  return window;
}

/**
*
* wclose(WINDOW *);
*
* Remove a window from the display.
*
**/
PUBLIC void wclose(WINDOW *window)
{
  DISPLAY *display = window->display;
  WINDOW *next;

  if (window->traceout != NULL)
  {
    fflush(window->traceout);
    Close(window->traceout);
  }
  unless ((next = (WINDOW *)window->node.Next)->node.Next == NULL)
  {
    next->pos -= window->size;
    next->size += window->size;
    if (next->loc.line > window->size) next->loc.line -= window->size;
    else next->loc.line = 1;
    dlock(display);
    wrefill(next);
    dunlock(display);
  }
  if (display->topwin == window) display->topwin = next;
  Remove(&window->node);
  freemem(window);
}

/**
*
* wclear(window);
*
* Clear a window.
*
**/
PUBLIC void wclear(WINDOW *window)
{
  int row;

  for (row = window->pos; row < window->pos + window->size; row++)
  {
    dcursor(window->display, row, 0);
    deol(window->display);
  }
}

/**
*
* wselect(window);
*
* High light window.
*
**/
PUBLIC void wselect(WINDOW *window)
{
  DISPLAY *display = window->display;
  WINDOW *topwin;

  if ((topwin = display->topwin) == window) return;
  display->topwin = window;
  dlock(display);
  unless (topwin == NULL) winfo(topwin);
  winfo(window);
  dunlock(display);
}

/**
*
* wgrow(window);
*
* Increase the size of a window.
*
**/
PUBLIC void wgrow(WINDOW *window)
{
  DISPLAY *display = window->display;
  WINDOW *other;

  unless ((other = (WINDOW *)window->node.Next)->node.Next == NULL)
  {
    unless (other->size > 2) return;
    window->size++;
    other->size--;
    other->pos++;
    other->loc.line++;
    dlock(display);
    dcursor(display, window->pos + window->size - 2, 0);
    list(display, getsource(window->loc.module), window->loc.line + window->size - 2, 1);
    winfo(window);
    dunlock(display);
    return;
  }
  unless ((other = (WINDOW *)window->node.Prev)->node.Prev == NULL)
    wshrink(other);
}

/**
*
* wshrink(window);
*
* Decrease the size of a window.
*
**/
PUBLIC void wshrink(WINDOW *window)
{
  DISPLAY *display = window->display;
  WINDOW *other;

  unless (window->size > 2) return;
  unless ((other = (WINDOW *)window->node.Next)->node.Next == NULL)
  {
    window->size--;
    other->size++;
    other->pos--;
    dlock(display);
    winfo(window);
    dcursor(display, other->pos, 0);
    if (other->loc.line > 1)
    {
      other->loc.line--;
      list(display, getsource(other->loc.module), other->loc.line, 1);
    }
    else 
      list(display, getsource(other->loc.module), other->loc.line, other->size - 1);
    dunlock(display);
    return;
  }
  unless ((other = (WINDOW *)window->node.Prev)->node.Prev == NULL)
    wgrow(other);
}

/**
*
* winfo(window);
*
* Display window information line.
*
**/
PRIVATE void winfo(WINDOW *window)
{
  DISPLAY *display = window->display;
  char *name = (window->loc.module == NULL) ? "<unknown source file>" :
                  basename(window->loc.module->name);
  int border = ((display->width - (strlen(name) + 2)) >> 1);
  int filler = (display->topwin == window) ? '=' : ' ';
  int i;

  debugf("winfo(), name = %s, border = %d, width = %d", name, border, display->width);
  dcursor(display, window->pos + window->size - 1, 0);
  dinverse(display);
  for (i = 0; i < border; i++) dputc(display, filler);
  dprintf(display, " %s ", name);
  for (i += strlen(name) + 2; i < display->width; i++) dputc(display, filler);
  dnormal(display);
}

/**
*
* highlight(window);
*
* Highlight a line in a window.
*
**/
PRIVATE void highlight(WINDOW *window)
{
  DISPLAY *display = window->display;

  unless (window->progloc.module == NULL)
  {
    dcursor(display, window->progloc.line - window->loc.line + window->pos, 0);
    dinverse(display);
    dprintf(display, "%4d:>", window->progloc.line);
    dnormal(display);
  }
}

/**
*
* lowlight(window);
*
* I don't know if I like this
*
**/
PUBLIC void lowlight(WINDOW *window)
{
  DISPLAY *display = window->display;

  if (inwindow(window, window->progloc))
  {
    dlock(display);
    dcursor(display, window->progloc.line - window->loc.line + window->pos, 0);
    dinverse(display);
    dprintf(display, "%4d:?", window->progloc.line);
    dnormal(display);
    dunlock(display);
  }
}

/**
*
* visible = inwindow(window, loc);
*
* Say whether location is visible in window.
*
**/
PRIVATE BOOL inwindow(WINDOW *window, LOCATION loc)
{
  return loc.module == window->loc.module AND
         loc.line >= window->loc.line AND
         loc.line < window->loc.line + window->size - 1;
}

/**
*
* wgoto(window, loc);
*
* Display the specified location.
*
**/
PUBLIC void wgoto(WINDOW *window, LOCATION loc)
{
  DISPLAY *display = window->display;

  if (loc.module == NULL)
  {
    debugf("wgoto(<no source>, %d)", loc.line);
    window->loc = window->progloc = loc;
    dlock(display);
    wclear(window);
    winfo(window);
    dunlock(display);
    return;
  }
  debugf("wgoto(%s, %d)", loc.module->name, loc.line);
  if (inwindow(window, loc))
  {
    dlock(display);
#ifdef OLDCODE
    unless (window->progloc.module == NULL OR window->progloc.line == loc.line)
#else
    if (window->progloc.line != loc.line AND inwindow(window, window->progloc))
#endif
    {
      dcursor(display, window->progloc.line - window->loc.line + window->pos, 0);
      dprintf(display, "%4d: ", window->progloc.line);
    }
    window->progloc = loc;
    highlight(window);
    dunlock(display);
  }
  else
  {
    BOOL samemodule = (window->loc.module == loc.module);

    window->progloc = loc;
    window->loc.module = loc.module;
    window->loc.line = loc.line > (window->size >> 1) ?
                       loc.line - (window->size >> 1) : 1;
    dlock(display);
    wrefill(window);
    unless (samemodule) winfo(window);
    dunlock(display);
  }
}

/**
*
* view(window, loc);
*
* Display the specified location.
*
**/
PUBLIC void view(WINDOW *window, LOCATION loc)
{
  DISPLAY *display = window->display;

  if (loc.module == NULL) return;
  if (inwindow(window, loc))
  {
    dlock(display);
    cursorhide(window);
    cursormove(window, loc.line - window->loc.line, 0);
    cursorshow(window);
    dunlock(display);
  }
  else
  {
    BOOL samemodule = (window->loc.module == loc.module);

    window->loc.module = loc.module;
    window->loc.line = loc.line > (window->size >> 1) ?
                       loc.line - (window->size >> 1) : 1;
    cursormove(window, loc.line - window->loc.line, 0);
    dlock(display);
    wrefill(window);
    unless (samemodule) winfo(window);
    dunlock(display);
  }
}

/**
*
* scrollup(window, size);
*
* Scroll contents of window up.
*
**/
PUBLIC void scrollup(WINDOW *window, int size)
{
  int lastline = getsource(window->loc.module)->lastline;
  int pagesize = window->size - 1;

  if (window->loc.line + pagesize <= lastline)
  {
    if (window->loc.line + pagesize + size <= lastline)
      window->loc.line += size;
    else
      window->loc.line = lastline - pagesize + 1;
    dlock(window->display);
    wrefill(window);
    dunlock(window->display);
  }
}

/**
*
* scrolldown(window, size);
*
* Scroll contents of window down.
*
**/
PUBLIC void scrolldown(WINDOW *window, int size)
{
  if (window->loc.line > 1)
  {
    if (window->loc.line > size) window->loc.line -= size;
    else window->loc.line = 1;
    dlock(window->display);
    wrefill(window);
    dunlock(window->display);
  }
}

/**
*
* pageup(window);
*
* Page-up a window.
*
**/
PUBLIC void pageup(WINDOW *window)
{
#ifdef OLDCODE
  scrollup(window, window->size - 1);
#endif
/*
-- crf : 26/07/91 - (minor) bug 689
-- "page -up" and "page -down" are the wrong way round
*/
  scrolldown(window, window->size - 1);
}

/**
*
* pagedown(window);
*
* Page-down a window.
*
**/
PUBLIC void pagedown(WINDOW *window)
{
#ifdef OLDCODE
  scrolldown(window, window->size - 1);
#endif
/*
-- crf : 26/07/91 - (minor) bug 689
-- "page -up" and "page -down" are the wrong way round
*/
  scrollup(window, window->size - 1);
}

/**
*
* pagefirst(window);
*
* Display first page.
*
**/
PUBLIC void pagefirst(WINDOW *window)
{
  scrolldown(window, getsource(window->loc.module)->lastline);
}

/**
*
* pagelast(window);
*
* Display last page.
*
**/
PUBLIC void pagelast(WINDOW *window)
{
  scrollup(window, getsource(window->loc.module)->lastline);
}

/**
*
* lrefresh(window, row, col, size, invert);
*
* Refresh part of a line.
*
**/
PRIVATE void lrefresh(WINDOW *window, int row, int col, int size, BOOL invert)
{
  DISPLAY *display = window->display;
  /* ACE: 80 might be too small */
  char buf[81];

  gettext(buf, getsource(window->loc.module), window->loc.line + row, col, size);
  dcursor(display, window->pos + row, col + DisplayBorder);
  if (invert)
  {
    dinverse(display);
    dprintf(display, "%s", buf);
    dnormal(display);
  }
  else dprintf(display, "%s", buf);
}

/**
*
* cursorshow(window);
*
* Display the cursor.
*
**/
PRIVATE void cursorshow(WINDOW *window)
{
  lrefresh(window, window->cur.row, window->cur.col, window->cur.size, TRUE);
}

/**
*
* cursorhide(window);
*
* Hide the cursor.
*
**/
PRIVATE void cursorhide(WINDOW *window)
{
  lrefresh(window, window->cur.row, window->cur.col, window->cur.size, FALSE);
}

/**
*
* cursormove(window, row, col);
*
* Move cursor to new position.
*
**/
PRIVATE void cursormove(WINDOW *window, int row, int col)
{
  unless (window->cur.row == row AND window->cur.col == col)
  {
    window->cur.row = row;
    window->cur.col = col;
    window->cur.size = 1;
  }
}

/**
*
* cursorup(window);
*
* Move the cursor up one line.
*
**/
PUBLIC void cursorup(WINDOW *window)
{
  int pagesize = window->size;
  if (window->cur.row == 0)/* scroll half page */
  {
    scrolldown(window, pagesize/2);
    dlock(window->display);
    cursorhide(window);
    window->cur.row = (pagesize-1)/2;
    cursorshow(window);
    dunlock(window->display);
  }
  else
  {
    dlock(window->display);
    cursorhide(window);
    window->cur.size = 1;
    window->cur.row--;
    cursorshow(window);
    dunlock(window->display);
  }
}

/**
*
* cursordown(window);
*
* Move the cursor down one line.
*
**/
PUBLIC void cursordown(WINDOW *window)
{
  int pagesize = window->size;
  if (window->cur.row == window->size - 2)/* scroll half page */
  {
    scrollup(window, pagesize/2);
    dlock(window->display);
    cursorhide(window);
    window->cur.row = (pagesize-1)/2;
    cursorshow(window);
    dunlock(window->display);
  }
  else
  {
    dlock(window->display);
    cursorhide(window);
    window->cur.size = 1;
    window->cur.row++;
    cursorshow(window);
    dunlock(window->display);
  }
}

/**
*
* cursorleft(window);
*
* Move the cursor left one column.
*
**/
PUBLIC void cursorleft(WINDOW *window)
{
  if (window->cur.col > 0)
  {
    dlock(window->display);
    cursorhide(window);
    window->cur.size = 1;
    window->cur.col--;
    cursorshow(window);
    dunlock(window->display);
  }
}

/**
*
* cursorright(window);
*
* Move the cursor right one column.
*
**/
PUBLIC void cursorright(WINDOW *window)
{
  if (DisplayBorder + window->cur.col < window->display->width - 1)
  {
    dlock(window->display);
    cursorhide(window);
    window->cur.size = 1;
    window->cur.col++;
    cursorshow(window);
    dunlock(window->display);
  }
}

/**
*
* cursorgrow(window);
*
* Increase the size of the cursor by one character.
*
**/
PUBLIC void cursorgrow(WINDOW *window)
{
  if (DisplayBorder + window->cur.col + window->cur.size < window->display->width)
  {
    dlock(window->display);
    lrefresh(window, window->cur.row, window->cur.col + window->cur.size, 1, TRUE);
    window->cur.size++;
    dunlock(window->display);
  }
}

/**
*
* cursorshrink(window);
*
* Decrease the size of the cursor by one character.
*
**/
PUBLIC void cursorshrink(WINDOW *window)
{
  if (window->cur.size > 1)
  {
    dlock(window->display);
    window->cur.size--;
    lrefresh(window, window->cur.row, window->cur.col + window->cur.size, 1, FALSE);
    dunlock(window->display);
  }
}

/**
*
* text = getcurtext(window, buf);
*
* Return the text under the cursor.
*
**/
PUBLIC char *getcurtext(WINDOW *window, char *buf)
{
  return getword(buf, getsource(window->loc.module), window->loc.line + window->cur.row, window->cur.col, window->cur.size);
}

#ifdef PARSYTEC
/******************************************************************
*
*
*	CR: this is a help routine to turn the arguments
*
*
******************************************************************/


PRIVATE void vwupdate (WATCHPOINT *watchpoint, DISPLAY *display)
{
  THREAD *thread = display->debug->thread;

  display->debug->ifwatchpoint = OLD_Watchpoint; /* CR: unfortunately I need */
  display->debug->tempwatchpoint = watchpoint->addr;/* them in locatestack */  
  display->debug->thread = watchpoint->thread; /* CR: necessary to have */
					       /*     the right evaluation */
  					       /*	  context  */
  
  dunlock(display);
  vupdate (display, watchpoint);
  dlock(display);
  
  display->debug->thread = thread;
  display->debug->ifwatchpoint = NEW_Watchpoint;
}
#endif
