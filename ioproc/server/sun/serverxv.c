/*-----------------------------------------------------------------------
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

/* RcsId: $Id: serverxv.c,v 1.6 1993/07/22 11:58:12 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

#include <stdio.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/tty.h>
#include <xview/termsw.h>
#include <xview/notice.h>
#include <xview/window.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/types.h>

#include "sunlocal.h"

Icon icon;
static short icon_image[] = {
#include "helios1.ico"
};

mpr_static(helios,ICON_DEFAULT_WIDTH,ICON_DEFAULT_HEIGHT,1,icon_image);

static short icon_image2[] = {
#include "helios.ico"
};

mpr_static(helios2,ICON_DEFAULT_WIDTH,ICON_DEFAULT_HEIGHT,1,icon_image2);

char *debug_names[] = { "Resources", "Reconfigure", "Messages", "Search",
			"Open", "Close", "Name", "Read", "Boot", "Keyboard",
			"Init", "Write", "Quit", "Graphics", "Timeout",
			"Open Reply ", "File I/O", "Delete", "Directory" };

char *logger_names[] = { "Screen", "File", "Both" };

int loggerhits = 0;
int logreply = TRUE;
void    Sun_initialise_devices();
static  Notify_value xv_size_interposer();
void    handle_panel_event();
void    handle_logger_event();
void    handle_debug_event();
void    handle_reboot();
void    handle_debugger();
void    handle_status();
void    handle_exit();
void    handle_logger();
void    handle_logger_button();
void    panel_in();
void    handle_debug();
void    wait_for_user();
extern  void    exit();

unsigned char mess[10];
unsigned char in_buf[4096];
unsigned char out_buf[64];
static int win;
static int server_window;            /* is this the Server window ? */
static int parent;                   /* maintains pipes to/from parent */
static  Menu debug_menu;             /* handle to Debug menu */
static  Menu logmen;                 /* handle to logger menu */
static  Panel panel;                 /* panel in which all buttons are stored */
static  Panel_item log;              /* handle on logger choices panel */
static  Tty tty;
static  Frame frame;                 /* parent window to tty and panels */

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
  Menu_item *item;
  Menu  menu;
  int  debugger;
  int  f_ptr,how_many,no_to_check,cp_ptr;

  nice(10);  /* priority much lower than server */

  server_window = atol(*(++argv));    /* is this the server window ? */
  parent        = atol(*(++argv));    /* receive data on this pipe */
  debugger      = atol(*(++argv));    /* 0 :- do not show debugger panel */

  argc -= 5;

  xv_init(NULL);

                         /* create helios icon for window */
  if(server_window)
  	icon  = icon_create(ICON_IMAGE, &helios, NULL); 
  else
	{
	Rect image_rect, label_rect;
	char tmp[10];

	strncpy(tmp, name, 9);
	tmp[9] = 0;
	rect_construct(&image_rect, 0, 0, 64, 64);
	rect_construct(&label_rect, 3, 68, 58, 10);

  	icon  = icon_create(
		   XV_WIDTH, 64, XV_HEIGHT, 78,
		   XV_LABEL, tmp,
                   ICON_LABEL_RECT, &label_rect,
		   ICON_IMAGE, &helios2,
		   ICON_IMAGE_RECT, &image_rect, NULL); 
	}

  frame = xv_create(XV_NULL, FRAME,
                        FRAME_LABEL, name,
                        FRAME_ICON, icon,
                        WIN_ROWS,   server_window ? 17 : 32,
			WIN_COLUMNS, 69,
                        FRAME_NO_CONFIRM, FALSE,
                        FRAME_SHOW_LABEL, TRUE,
                        0);

  if (!server_window) goto ready;

/**
*** create the debug menu for debug. When adding to this list the defines must
*** be given the position of the menu entry in this list. All make WIN_ALL the 
*** last value in the defines list for debug menu entries
**/

  debug_menu = xv_create( NULL, MENU_TOGGLE_MENU,
			  MENU_NCOLS, 2, MENU_NROWS, 9,
			  NULL);
  logmen = xv_create( NULL, MENU_CHOICE_MENU, NULL);

  {	Menu_item mi;
	int i;

	for(i = 0 ; i < 19 ; i++)
		{
		mi = (Menu_item) xv_create(NULL, MENUITEM,
				MENU_STRING,	debug_names[i],
				MENU_NOTIFY_PROC,handle_debug_event,
				MENU_RELEASE,
				NULL);
		xv_set(debug_menu, MENU_APPEND_ITEM, mi, NULL);
		}
	for(i = 0 ; i < 3 ; i++)
		{
		mi = (Menu_item) xv_create(NULL, MENUITEM,
				MENU_STRING,	logger_names[i],
				MENU_NOTIFY_PROC,handle_logger,
				MENU_RELEASE,
				NULL);
		xv_set(logmen, MENU_APPEND_ITEM, mi, NULL);
		}
  }
  menu_set(debug_menu, XV_MARGIN, 5, 0);

                                       /* Add panel buttons to the window */
  panel = xv_create(frame, PANEL,
                        PANEL_LAYOUT, PANEL_HORIZONTAL,0);

  xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,		"Reboot",
                    PANEL_NOTIFY_PROC,  handle_reboot,0);

  if(debugger)
    xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,		"Debugger",
                    PANEL_NOTIFY_PROC,  handle_debugger,0);

  xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,		"Status",
                    PANEL_NOTIFY_PROC,  handle_status,0);

  xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,		"Exit",
                    PANEL_NOTIFY_PROC,  handle_exit,0);

  log = xv_create(panel, PANEL_BUTTON,
                    PANEL_LABEL_STRING,    "Logger - Screen",
		    PANEL_NOTIFY_PROC,	handle_logger_button,
                    PANEL_EVENT_PROC, handle_logger_event,0);

  xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,		"Debug",
                    PANEL_NOTIFY_PROC,  handle_debug,
                    PANEL_EVENT_PROC, handle_panel_event,0);
               
  window_fit_height(panel);


ready:
                                  /* create  tty window */
  tty = xv_create(frame, TERMSW,
                      WIN_ROWS,   server_window ? 14 : 32,
		      WIN_COLUMNS, 80,
                      TTY_ARGV, TTY_ARGV_DO_NOT_FORK,
                      0);
  xv_set(tty, TERMSW_MODE, TTYSW_MODE_TYPE, 0);
  win = (int) xv_get(tty, TTY_TTY_FD);

  window_fit_width(tty);
  window_fit(frame);
  xv_set(frame, XV_SHOW, TRUE, 0);

/**
*** add interposer to get new win size on resizing of win. Interposers insert
*** user code when specified window events happen
**/
  (void) notify_interpose_event_func(frame,xv_size_interposer, NOTIFY_SAFE);
  (void) notify_set_signal_func(frame, exit, SIGINT, NOTIFY_ASYNC);
  (void) notify_set_signal_func(frame, exit, SIGHUP, NOTIFY_ASYNC);
  (void) notify_set_signal_func(frame, exit, SIGQUIT, NOTIFY_ASYNC);
  (void) notify_set_signal_func(frame, exit, SIGPIPE, NOTIFY_ASYNC);

/** 
*** get the win size and return it to the creator so he can find out the 
*** initial size of the created window
**/

  send_window_size();

            /* initialise the window to the correct setup for Helios windows */
  Sun_initialise_devices();

  (void) notify_do_dispatch();            /* start the notifier running */

  maxmask= getdtablesize();               /* size of fd table for select */
  for(;;)
  { 
    FD_ZERO(&rdmask);
    FD_SET(parent,&rdmask);               /* from parent */
    FD_SET(win,&rdmask);                  /* from child block forever */
    if(select(maxmask,&rdmask,0,0,0) == 0) continue; 
    if(FD_ISSET(parent,&rdmask))
    { 

      no_to_check = how_many = read(parent,in_buf,500);
      if (how_many <= 0) exit(1);   /* broken pipe */
      for(f_ptr = 0; f_ptr <  no_to_check; f_ptr++)
        if ( in_buf[f_ptr] == FUNCTION_CODE )
          {
           if (in_buf[f_ptr + 1] == WINDOW_KILL)
	     {
	       if(server_window)
		wait_for_user();
               exit(1);
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
       exit(1);
    }  
  }
}

/**
*** this notifyer is called whenever the window changes size
*** It returns back to the calling process the new size
**/

static Notify_value xv_size_interposer(lframe, event, arg, type)
Frame                   lframe;
Event                   *event;
Notify_arg              arg;
Notify_event_type       type;
{
    if (event_action(event) == WIN_RESIZE)
	send_window_size();

    return(notify_next_event_func(lframe, event, arg, type));
}

/**
*** This special handler retrieves the selections on the Debug menu.
*** The events are then transmitted back to the parent.
**/

void handle_panel_event(item, event)
Panel_item item;
Event *event;
{
  if (event_id(event) == MS_RIGHT && event_is_down(event))
      menu_show(debug_menu,panel,event,0);
  else
    panel_default_handle_event(item,event);
}

void handle_logger_event(item, event)
Panel_item item;
Event *event;
{
  if (event_id(event) == MS_RIGHT && event_is_down(event))
      menu_show(logmen,panel,event,0);
  else
    panel_default_handle_event(item,event);
}


void handle_debug_event(menu, menu_item)
Menu menu;
Menu_item menu_item;
{
      int sel,invert;
      char buffer[20];

      strcpy(buffer, xv_get(menu_item, MENU_STRING));
	for(sel = 0 ; sel < 19 ; sel++)
		if(!strcmp(buffer, debug_names[sel]))
			break;

      
      mess[0] = FUNCTION_CODE;
      mess[1] = WINDOW_PANEL;
      mess[2] = ++sel;
      mess[3] = 0;
      	   if ( (sel != WIN_MEMORY) && (sel != WIN_RECONF) )
       	   {
         	   if ( xv_get(menu_item, MENU_SELECTED) )
            	     mess[3] = WIN_OFF;
         	   else
            	     mess[3] = WIN_ON;
       	   }
	   else
            	    xv_set(menu_item, MENU_SELECTED, FALSE,0);

      write(parent,mess,4);
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
  sleep(3);		/* wait for parent to shut down */
  exit(1);
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

void handle_logger_button(item, event)
Panel_item item;
Event *event;
{
	Menu_item menu_item;

	menu_item = xv_get(logmen, MENU_NTH_ITEM, ((++loggerhits)%3) + 1);
	handle_logger(logmen, menu_item);
}

void handle_logger(menu, menu_item)
Menu menu;
Menu_item menu_item;
{
  int value, saveval= 0;
  char buffer[10];
  Menu_item titem;

  strcpy(buffer, xv_get(menu_item, MENU_STRING));

  for(value = 0 ; value < 3 ; value++)
	{
	titem = (Menu_item) xv_get(menu, MENU_NTH_ITEM, value+1);
	xv_set(titem, MENU_SELECTED, FALSE, 0);
	if(!strcmp(buffer, logger_names[value]))
		saveval = value;
	}
  value = loggerhits = saveval;
  xv_set(menu_item, MENU_SELECTED, TRUE, 0);

  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  switch(value)
    {
      case(1) : mess[2] = WIN_LOG_FILE;
  		xv_set(log,PANEL_LABEL_STRING,"Logger - File",0);
                break;
      case(0) : mess[2] = WIN_LOG_SCREEN;
  		xv_set(log,PANEL_LABEL_STRING,"Logger - Screen",0);
                break;
      case(2) : mess[2] = WIN_LOG_BOTH;
  		xv_set(log,PANEL_LABEL_STRING,"Logger - Both",0);
                break;
    }
  mess[3] = 0;
  if(logreply)
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
  Menu_item menu_item;
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
      menu_item = xv_get(logmen, MENU_NTH_ITEM, log_value+1);
      logreply = FALSE;
      handle_logger(logmen, menu_item, FALSE);
      logreply = TRUE;
      return;
    }
  else
   {                                        /* debug item select */
     debug_item = menu_get(debug_menu, MENU_NTH_ITEM,(int)panel_event);
      if (panel_value)
          xv_set(debug_item, MENU_SELECTED, TRUE,0);
      else
          xv_set(debug_item, MENU_SELECTED, FALSE,0);    
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
    Panel       lpanel = (Panel)xv_get(panel, PANEL_PARENT_PANEL);

   (void) notice_prompt(lpanel, (Event *) NULL,
           NOTICE_NO_BEEPING, 1, 
           NOTICE_MESSAGE_STRINGS, "Server Exiting.", 0,
           NOTICE_BUTTON_YES, "OK",
           0 );

}

send_window_size()
{
	static int mastercols = 0;
	static int masterrows = 0;
	int cols = (int) xv_get(tty, WIN_COLUMNS);
	int rows = (int) xv_get(tty, WIN_ROWS);

	if(rows == masterrows && cols == mastercols)  /* nothing to do */
		return;

	masterrows = rows;
	mastercols = cols;

	mess[0] = FUNCTION_CODE;
	mess[1] = WINDOW_SIZE;
	mess[2] = (char)rows;
	mess[3] = (char)cols - 4;	/* account for scrollbar */
	write(parent, mess, 4);
}
