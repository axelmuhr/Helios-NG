/**
*
* Title:  Helios Debugger - Menu supprt.
*
* Author: Andy England
*
* Date:   April 1989
*
*         (c) Copyright 1989 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/menu.c,v 1.4 1992/10/27 14:19:30 nickc Exp $";
#endif

#include "tla.h"

/* -- crf : 08/08/91 - for displaying menu boxes */
#define inv_char(c) dinverse(display) ; \
                    dputc(display, c) ; \
                    dnormal(display)
/**
*
* _menu(debug, cmdc, cmdv, labv);
*
* Handle a menu interaction.
*
**/
PUBLIC void _menu(DEBUG *debug, char *title, int cmdc, char *cmdv[], char *labv[])
{
  DISPLAY *display = debug->display;
  int top, left, length;
  int width = 0;
  int choice = 0;
  int i;

  if (cmdc == 0) return;

  for (i = 0; i < cmdc; i++)
  {
#ifndef OLDCODE
    char buf[WordMax + 1];

    labv[i] = strdup(expandcmd(debug->interp, buf, labv[i], WordMax));
#endif
    if ((length = strlen(labv[i])) > width) width = length;
  }
  unless (title == NULL)
    if ((length = strlen(title)) > width) width = length;
  width += 4;

  top = ((display->height - cmdc - 2) >> 1);
  left = ((display->width - width) >> 1);
  dlock(display);

#ifdef OLDCODE
  dcursor(display, top - 1, left - 1);
  if (title == NULL)
    for (i = 0; i < width; i++) dputc(display, '-');
  else
  {
    /* ACE: should check title is not too large */
    int border = ((width - strlen(title) - 2) >> 1);

    for (i = 0; i < border; i++) dputc(display, '-');
    dprintf(display, " %s ", title);
    i += strlen(title) + 2;
    while (i++ < width) dputc(display, '-');
  }
  for (i = 0; i < cmdc; i++)
  {
    dcursor(display, top + i, left - 1);
    if (i == 0)
    {
      dputc(display, '|');
      dinverse(display);
      dprintf(display, " %-*s ", width - 4, labv[i]);
      dnormal(display);
      dputc(display, '|');
    }
    else dprintf(display, "| %-*s |", width - 4, labv[i]);
  }
  dcursor(display, top + cmdc, left - 1);
  for (i = 0; i < width; i++) dputc(display, '-');
#endif

/*
-- crf : 08/08/91 - Use inverse video to display menu boxes
*/

  dinverse (display) ;
  dcursor(display, top - 2, left - 2);
  if (title == NULL)
    for (i = 0; i < width + 2; i++) dputc(display, ' ');
  else
  {
    /* ACE: should check title is not too large */
    int border = ((width - strlen(title) - 2) >> 1);

    for (i = 0; i < border + 1; i++) dputc(display, ' ');
    dprintf(display, " %s ", title);
    i += strlen(title) + 2;
    while (i++ < width + 2) dputc(display, ' ');
  }
  dnormal (display) ;

  dcursor(display, top - 1, left - 2);
  inv_char (' ') ;
  for (i = 0 ; i < width ; i ++)
    dputc(display, ' ');
  inv_char (' ') ;

  for (i = 0; i < cmdc; i++)
  {
    dcursor(display, top + i, left - 2);
    inv_char (' ') ;
    if (i == 0)
    {
      dputc(display, ' ');
      dinverse(display);
      dprintf(display, " %-*s ", width - 4, labv[i]);
      dnormal(display);
      dputc(display, ' ');
    }
    else dprintf(display, "  %-*s  ", width - 4, labv[i]);
    inv_char (' ') ;
  }
  dcursor(display, top + cmdc, left - 2);
  inv_char (' ') ;
  for (i = 0; i < width; i++) dputc(display, ' ');
  inv_char (' ') ;

  dcursor(display, top + cmdc + 1, left - 2);
  dinverse (display) ;
  for (i = 0; i < width + 2; i++) dputc(display, ' ');
  dnormal (display) ;

/*
-- crf : end of menu box modifications
*/

  dcursor(display, display->row, display->col);

  forever
  {
    int c;

    fflush(display->fileout);
    switch (c = dgetc(display))
    {
      case CSI:
      switch (c = dgetc(display))
      {
      	case 'A':
      	if (choice > 0)
      	{
      	  dcursor(display, choice + top, left);
          dprintf(display, " %-*s ", width - 4, labv[choice]);
      	  dcursor(display, --choice + top, left);
      	  dinverse(display);
          dprintf(display, " %-*s ", width - 4, labv[choice]);
          dnormal(display);
          dcursor(display, display->row, display->col);
      	}
        else dputc(display, Bell);
      	break;

      	case 'B':
      	if (choice < cmdc - 1)
      	{
      	  dcursor(display, choice + top, left);
          dprintf(display, " %-*s ", width - 4, labv[choice]);
      	  dcursor(display, ++choice + top, left);
      	  dinverse(display);
          dprintf(display, " %-*s ", width - 4, labv[choice]);
          dnormal(display);
          dcursor(display, display->row, display->col);
      	}
        else dputc(display, Bell);
      	break;

        default:
        until (iscst(c)) c = dgetc(display);
        dputc(display, Bell);
        break;
      }
      break;

      case '\r':
      case '\n':
      drefresh(display);
      dunlock(display);
#ifndef OLDCODE
      for (i = 0; i < cmdc; i++) freemem(labv[i]);
#endif
      _do(debug, cmdv[choice]);
      return;

      case Escape:
      drefresh(display);
      dunlock(display);
#ifndef OLDCODE
      for (i = 0; i < cmdc; i++) freemem(labv[i]);
#endif
      return;

      default:
      dputc(display, Bell);
      break;
    }
  }
}
