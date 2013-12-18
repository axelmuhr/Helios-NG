/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1989, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
-- serverwi.c                                                           --
--                                                                      --
--  Author:  BLV 8/6/89                                                 --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: serversv.c,v 1.2 1993/07/22 11:55:18 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/tty.h>
#include <suntool/panel.h>
#include <suntool/alert.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/types.h>

#include "sunlocal.h"

Icon icon;
static short icon_image[] = {
#include "helios1.ico"
};

mpr_static(helios,64,64,1,icon_image);

static short icon_image2[] = {
#include "helios.ico"
};

mpr_static(helios2,64,64,1,icon_image2);


void   Sun_initialise_devices();

void    Sun_initialise_devices();
static  Notify_value window_size_interposer();
void    handle_panel_event();
void    handle_panel_event1();
void    handle_reboot();
void    handle_debugger();
void    handle_status();
void    handle_exit();
void    handle_logger();
void    panel_in();
void    handle_debug();
void    wait_for_user();

unsigned char mess[10];
unsigned char in_buf[4096];
unsigned char out_buf[64];
static int win;
static int server_window;            /* is this the Server window ? */
static int parent;                   /* maintains pipes to/from parent */
static  Menu debug_menu;             /* handle to Debug menu */
static  Panel panel;                 /* panel in which all buttons are stored */
static  Panel_item log;              /* handle on logger choices panel */
static  Tty tty;
static  Frame frame;                 /* parent window to tty and panels */

/**
*** If the exit button has been selected it is safe to exit immediately.
*** Otherwise it is necessary to wait because the window may not
*** be visible yet.
**/
static  int exit_selected = 0;
/**
*** we expect six arguments for a server window . These are in order of
*** declaration program name, server window flag,label name, from pipe, to pipe,
*** debugger on/off.
**/

int main(argc, argv)
int argc;
char **argv;
{ 
  int maxmask;                       /* select max file descriptors */
  fd_set rdmask, wrtmask;            /* mask for select */
  char   *name = *(++argv);
  int   mess_size;                   /* size of messages. note no functions! */
  Menu_item item;
  Menu  menu;
  int  debugger;
  int  f_ptr,how_many,no_to_check,cp_ptr;

  nice(10);  /* priority much lower than server */

  server_window = atol(*(++argv));    /* is this the server window ? */
  parent        = atol(*(++argv));    /* receive data on this pipe */
  debugger      = atol(*(++argv));    /* 0 :- do not show debugger panel */

                         /* create helios icon for window */
  icon  = icon_create(ICON_IMAGE, 
             server_window ? &helios : &helios2, 0); 

  frame = window_create(NULL, FRAME,
                        FRAME_LABEL, name,
                        FRAME_ICON, icon,
                        WIN_ROWS,   server_window ? 14 : 32,
                        FRAME_NO_CONFIRM, TRUE,
                        0);

  menu = window_get(frame, WIN_MENU); /* turn off the Quit on the window menu */
  item = menu_find(menu,MENU_STRING, "Quit", 0);
  menu_set(item, MENU_INACTIVE, TRUE, 0);


  if (!server_window) goto ready;

/**
*** create the debug menu for debug. When adding to this list the defines must
*** be given the position of the menu entry in this list. All make WIN_ALL the 
*** last value in the defines list for debug menu entries
**/
  debug_menu = menu_create( MENU_NCOLS, 1, MENU_STRINGS,
                        "Resources", "Reconfigure",
                        "Messages", "Search", "Open", "Close",
                        "Name","Read","Boot","Keyboard","Init",
                        "Write","Quit","Graphics", "Timeout",
                        "Open Reply", "File I/O", "Delete",
                        "Directory", 0, 0);
  menu_set(debug_menu, MENU_MARGIN, 5, 0);

                                       /* Add panel buttons to the window */
  panel = window_create(frame, PANEL,
                        PANEL_LAYOUT, PANEL_HORIZONTAL,0);
  panel_create_item(panel, PANEL_BUTTON,
                    PANEL_LABEL_IMAGE,
                             panel_button_image(panel, "Reboot", 0, 0),
                   PANEL_NOTIFY_PROC,  handle_reboot,0);

  if(debugger)
    panel_create_item(panel, PANEL_BUTTON,
                    PANEL_LABEL_IMAGE, 
                             panel_button_image(panel, "Debugger", 0, 0),
                    PANEL_NOTIFY_PROC,  handle_debugger,0);

  panel_create_item(panel, PANEL_BUTTON,
                    PANEL_LABEL_IMAGE,
                            panel_button_image(panel, "Status", 0, 0),
                    PANEL_NOTIFY_PROC,  handle_status,0);

  panel_create_item(panel, PANEL_BUTTON,
                    PANEL_LABEL_IMAGE, panel_button_image(panel, "Exit", 0, 0),
                    PANEL_NOTIFY_PROC,  handle_exit,0);

  log = panel_create_item(panel, PANEL_CYCLE,
                    PANEL_LABEL_STRING,    "Logger",
                    PANEL_CHOICE_STRINGS,  "Screen", "File", "Both", 0,  
                    PANEL_NOTIFY_PROC,  handle_logger,0);

  panel_create_item(panel, PANEL_BUTTON,
                    PANEL_LABEL_IMAGE, panel_button_image(panel, "Debug", 0, 0),
                    PANEL_NOTIFY_PROC,  handle_debug,
                    PANEL_EVENT_PROC, handle_panel_event,0);
               
  window_fit_height(panel);


ready:
                                  /* create  tty window */
  tty = window_create(frame, TTY,
                      TTY_ARGV, TTY_ARGV_DO_NOT_FORK,
                      0);
  win = (int) window_get(tty, TTY_TTY_FD);
  window_set(frame, WIN_SHOW,  TRUE, 0);

/**
*** add interposer to get new win size on resizing of win. Interposers insert
*** user code when specified window events happen
**/
  (void) notify_interpose_event_func(tty,window_size_interposer, NOTIFY_SAFE);

/** 
*** get the win size and return it to the creator so he can find out the 
*** initial size of the created window
**/
  { int cols = (int) window_get(tty, WIN_COLUMNS);
    int rows = (int) window_get(tty, WIN_ROWS);
    mess[0] = FUNCTION_CODE;
    mess[1] = WINDOW_SIZE;
    mess[2] = (char)rows;
    mess[3] = (char)cols;
    write(parent,mess,4);
  }

            /* initialise the window to the correct setup for Helios windows */
  Sun_initialise_devices();

  (void) notify_do_dispatch();            /* start the notifier running */

  maxmask= getdtablesize();               /* size of fd table foe select */
  for(;;)
  { 
    FD_ZERO(&rdmask);
    FD_SET(parent,&rdmask);               /* from parent */
    FD_SET(win,&rdmask);                  /* from child block forever */
    if(select(maxmask,&rdmask,0,0,0) == 0) continue; 
    if(FD_ISSET(parent,&rdmask))
    { 

      no_to_check = how_many = read(parent,in_buf,500);
      if (how_many <= 0) _exit(1);   /* broken pipe */
      for(f_ptr = 0; f_ptr <  no_to_check; f_ptr++)
        if ( in_buf[f_ptr] == FUNCTION_CODE )
          {
           if (in_buf[f_ptr + 1] == WINDOW_KILL)
            {  if (!exit_selected && server_window)
                wait_for_user();
               window_destroy(frame);
               exit(0);
            }
           if (in_buf[f_ptr + 1] == WINDOW_PANEL)    /* update panel data */
             { 
               panel_in(in_buf[f_ptr+2],in_buf[f_ptr + 3]);
             }
           cp_ptr = f_ptr + 4; 
           while( cp_ptr < no_to_check)
             {
                in_buf[cp_ptr - 4] = in_buf[cp_ptr];
                cp_ptr++;
             }
           how_many -= 4; f_ptr--; no_to_check--;
          } 
      if (how_many > 0) write(win,in_buf,how_many);
    }
    if(FD_ISSET(win,&rdmask))    /* send data to parent process */
    { int x = read(win, out_buf, 64);   
      if (write(parent,out_buf,x) <= 0)
       _exit(1);
    }  
  }
}

/**
*** this notifyer is called whenever the window changes size
*** It returns back to the calling process the new size
**/

static Notify_value window_size_interposer(frame, event, arg, type)
Frame                   frame;
Event                   *event;
Notify_arg              arg;
Notify_event_type       type;
{
    Notify_value value;
    value = notify_next_event_func(frame, event, arg, type);
    if (event_action(event) == WIN_RESIZE)
     { int cols = (int) window_get(tty, WIN_COLUMNS);
       int rows = (int) window_get(tty, WIN_ROWS);
       mess[0] = FUNCTION_CODE;
       mess[1] = WINDOW_SIZE;
       mess[2] = (char)rows;
       mess[3] = (char)cols;
       write(parent, mess, 4);
     }
    return(value);
}

/**
*** This special handler retrieves the selections on the Debug menu.
*** The events are then transmitted back to the parent.
**/

void handle_panel_event(item, event)
Panel_item item;
Event *event;
{
  int sel,invert;
  Panel_item debug_item;
  if (event_action(event) == MS_RIGHT && event_is_down(event))
    {
      sel = (int)menu_show(debug_menu,panel,event,0);
      mess[0] = FUNCTION_CODE;
      mess[1] = WINDOW_PANEL;
      mess[2] = (char)sel;
      debug_item = menu_get(debug_menu, MENU_NTH_ITEM,sel);
      if ( (sel != WIN_MEMORY) && (sel != WIN_RECONF) )
       { invert = (int) menu_get(debug_item, MENU_BOXED,0);
         if (invert)
          {
            menu_set(debug_item, MENU_BOXED, FALSE,0);
            mess[3] = WIN_OFF;
          }
         else
          { 
            menu_set(debug_item, MENU_BOXED, TRUE,0);    
            mess[3] = WIN_ON;
          }
       }
      write(parent,mess,4);
   }
  else
    panel_default_handle_event(item,event);
}


/**
*** the following routines handle the other panel buttons
**/        
void handle_reboot(item, event)
Panel_item item;
Event      *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = WIN_REBOOT;
  mess[3] = 0;
  write(parent,mess,4);
}

void handle_debugger(item, event)
Panel_item item;
Event      *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = WIN_DEBUGGER;
  mess[3] = 0;
  write(parent,mess,4);
}

void handle_status(item, event)
Panel_item item;
Event      *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = WIN_STATUS;
  mess[3] = 0;
  write(parent,mess,4);
}

void handle_exit(item, event)
Panel_item item;
Event      *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = WIN_EXIT;
  mess[3] = 0;
  write(parent,mess,4);
  exit_selected = 1;
}

void handle_debug(item, event)
Panel_item item;
Event      *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = WIN_DEBUG;
  mess[3] = 0;
  write(parent,mess,4);
}

void handle_logger(item, value, event)
Panel_item item;
int        value;
int        *event;
{
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  switch(value)
    {
      case(1) : mess[2] = WIN_LOG_FILE;
                break;
      case(0) : mess[2] = WIN_LOG_SCREEN;
                break;
      case(2) : mess[2] = WIN_LOG_BOTH;
                break;
    }
  mess[3] = 0;
  write(parent,mess,4);
}

/**
*** this handles any updates to the panel displays from the parent process
**/
void panel_in(panel_event, panel_value)
char panel_event;
char panel_value;
{
  int log_value;
  Panel_item debug_item;
  if (panel_event > WIN_ALL)     /* check for logger value */
    {
      switch(panel_event)
        { 
          case(WIN_LOG_FILE)   : log_value = 1;
                                 break;
          case(WIN_LOG_SCREEN) : log_value = 0;
                                 break;
          case(WIN_LOG_BOTH)   : log_value = 2;
                                 break;
        }
      panel_set(log,PANEL_VALUE,log_value,0);
      panel_set(log,PANEL_DISPLAY_LEVEL,PANEL_CURRENT,0);
      return;
    }
  else
   {                                        /* debug item select */
     debug_item = menu_get(debug_menu, MENU_NTH_ITEM,(int)panel_event);
      if (panel_value)
          menu_set(debug_item, MENU_BOXED, TRUE,0);
      else
          menu_set(debug_item, MENU_BOXED, FALSE,0);    
   }
}


/**
*** Device initialisation and tidying routines.
**/

void Sun_initialise_devices()
{ 
  struct termios cur_term;
  ioctl(win, TCGETS, &cur_term);
  cur_term.c_iflag |= (IGNBRK | IGNPAR | INLCR); 
  cur_term.c_iflag &= ~(IXON + IXOFF);
  cur_term.c_lflag &= ~(ISIG + ICANON + ECHO + ECHOE + ECHOK + 
                        ECHONL);
  cur_term.c_oflag &= ~OPOST;
  cur_term.c_cc[VMIN]  = 0;
  cur_term.c_cc[VTIME] = 0;
  ioctl(win, TCSETS, &cur_term);
}

void wait_for_user()
{ 
  (void) alert_prompt(frame, (Event *) NULL,
           ALERT_NO_BEEPING, 1, 
           ALERT_MESSAGE_STRINGS, "Server Exiting.", 0,
           ALERT_BUTTON_YES, "OK",
           0 );

}

