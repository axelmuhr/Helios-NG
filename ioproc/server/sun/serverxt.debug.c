/*-----------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1993, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
-- serverxt.c                                                           --
--                                                                      --
--  Author:  BLV 21/7/93                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: serverxt.debug.c,v 1.1 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

/*{{{  header files and compile time options */
#ifdef SUN4
#undef SUN4
#define SUN4	1
#endif

#ifdef SOLARIS
#undef SOLARIS
#define SOLARIS 1
#endif

#if SOLARIS
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Paned.h>

#include <stdio.h>
#include <stdlib.h>

#include "sunlocal.h"

#ifndef EXIT_FAILURE
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#endif

#if defined (SUN4) || defined (RS6000)
#define fn(a) a
#else
#define fn(a) &a
#endif
/*}}}*/
/*{{{  data structures and statics */
#define ApplicationName	 "HeliosWindow"
#if 0
#define DefaultFont	 "-adobe-helvetica-bold-r-normal--*-100-*-*-*-*-*-*"
#else
#define DefaultFont	"fixed"
#endif

static	char		*Title			= "Helios";
static	int		 IOServerWindow		= FALSE;
static	int		 ShowDebugger		= FALSE;
static	int		 ChildFd		= 0;
static	int		 LoggerState		= WIN_LOG_SCREEN;
static	XtAppContext	 ThisContext;
static	Display		*ThisDisplay;
static	int		 ThisScreen;
static	Widget	 	 TopLevel;
static	Widget	 	 WholeWindow;
static	Widget	 	 ControlPanel;
static	Widget		 LoggerWidget;
static	Widget	 	 TextWidget;
static	XFontStruct	*TextFont;

static	Window		 TextWindow;
static	unsigned int	 TextHeight;
static	unsigned int	 TextWidth;
static	GC		 NormalGc;
static	GC		 InverseGc;
static	GC		 CursorGc;

static	int		 CharWidth;
static	int		 CharHeight;
static	int		 CharAscent;

static	int		 GotFocus	= FALSE;

static	Arg		 GlobalArgs[15];
static	int		 GlobalArgCount;
#define	reset_args()	 GlobalArgCount = 0
#define set_arg(a,b)	 XtSetArg(GlobalArgs[GlobalArgCount], a, b);	GlobalArgCount++
#define args		 GlobalArgs, GlobalArgCount

static	Pixmap		 TickBitmap;
static	Pixmap		 ClearBitmap;

typedef struct MenuEntry {
	String		Name;
	Widget		Widget;
	int		Value;
	int		Set;
} MenuEntry;

MenuEntry MenuOptions[] = {
	{ "Resources",		NULL,	WIN_MEMORY,	FALSE	},
	{ "Reconfigure",	NULL,	WIN_RECONF,	FALSE	},
	{ "Messages",		NULL,	WIN_MESSAGES,	FALSE	},
	{ "Search",		NULL,	WIN_SEARCH,	FALSE	},
	{ "Open",		NULL,	WIN_OPEN,	FALSE	},
	{ "Close",		NULL,	WIN_CLOSE,	FALSE	},
	{ "Name",		NULL,	IOWIN_NAME,	FALSE	},
	{ "Read",		NULL,	WIN_READ,	FALSE	},
	{ "Boot",		NULL,	WIN_BOOT,	FALSE	},
	{ "Keyboard",		NULL,	WIN_KEYBOARD,	FALSE	},
	{ "Init",		NULL,	WIN_INIT,	FALSE	},
	{ "Write",		NULL,	WIN_WRITE,	FALSE	},
	{ "Quit",		NULL,	WIN_QUIT,	FALSE	},
	{ "Timeout",		NULL,	WIN_TIMEOUT,	FALSE	},
	{ "Open Reply",		NULL,	WIN_OPENREPLY,	FALSE	},
	{ "File I/O",		NULL,	WIN_FILEIO,	FALSE	},
	{ "Delete",		NULL,	WIN_DELETE,	FALSE	},
	{ "Directory",		NULL,	WIN_DIRECTORY,	FALSE	},
	{ NULL,			NULL,	0,		FALSE	}
};

static	int	RebootEvent	= WIN_REBOOT;
static	int	DebuggerEvent	= WIN_DEBUGGER;
static	int	StatusEvent	= WIN_STATUS;
static	int	ExitEvent	= WIN_EXIT;
static	int	DebugEvent	= WIN_DEBUG;

/*{{{  bitmaps */

#define ioserv_width 40
#define ioserv_height 48
static unsigned char ioserv_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x80, 0xc1, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x01, 0x00, 0x40, 0x20, 0x14, 0x02, 0x00, 0x40, 0x10, 0x14,
   0x04, 0x00, 0x40, 0x08, 0x04, 0x08, 0x00, 0x40, 0xeb, 0x94, 0xeb, 0x00,
   0xc0, 0x14, 0x55, 0x14, 0x01, 0x40, 0x14, 0x55, 0x14, 0x00, 0x40, 0xf4,
   0x55, 0xf4, 0x00, 0x40, 0x14, 0x54, 0x04, 0x01, 0x40, 0x14, 0x55, 0x14,
   0x01, 0x40, 0xec, 0x94, 0xeb, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00,
   0x10, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00,
   0x01, 0x00, 0x00, 0x80, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xce,
   0x38, 0x77, 0xa2, 0x3b, 0x24, 0x45, 0x91, 0xa2, 0x48, 0x24, 0x05, 0x91,
   0xa2, 0x48, 0x24, 0x3d, 0x77, 0xa2, 0x3b, 0x24, 0x41, 0x51, 0x94, 0x28,
   0x24, 0x45, 0x91, 0x94, 0x48, 0xce, 0x38, 0x97, 0x88, 0x4b, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define tick_width 8
#define tick_height 8
static unsigned char tick_bits[] = {
   0x00, 0x80, 0xc0, 0xe2, 0x34, 0x18, 0x08, 0x00};

#define clear_width 8
#define clear_height 8
static char clear_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*}}}*/
/*}}}*/
/*{{{  font and text output handling */
/*{{{  text-specific variables */
/*
 * These variables deal with drawing and positioning the cursor.
 */
static	int		 CursorX	= 0;
static	int		 CursorY	= 0;
static	int		 CursorDrawn	= FALSE;

/*
 * Current display mode.
 */
static	int		 InverseVideo	= FALSE;

/*
 * The text code will perform some buffering while processing. There
 * are separate buffers for plain text and for the current escape
 * sequence, if any.
 */
static	char	TextBuffer[256];
static	int	TextBufferLength	= 0;

static	char	EscapeBuffer[32];
static	char	EscapeBufferLength	= 0;

/*}}}*/
/*{{{  get font details */
/*
 * Determine details about the font being used. Note the window will only
 * work properly with a fixed font.
 */
static void get_font_details()
{
	struct _resource {
		String		fontname;
	} data;

	static	XtResource	resources[] = {
	{ "ttyfont", "Ttyfont", XtRString, sizeof(String),
		 XtOffset(struct _resource *, fontname), 
		 XtRString, DefaultFont }
	};

	XtGetApplicationResources(TopLevel, &data, resources, 1, NULL, 0);

	TextFont	= XLoadQueryFont(ThisDisplay, data.fontname);
	if (TextFont == NULL)
		TextFont = XLoadQueryFont(ThisDisplay, "fixed");

	if (TextFont == NULL)
	{
		fputs("server window: failed to load a fixed font\n", stderr);
		exit(EXIT_FAILURE);
	}

	CharWidth	= TextFont->max_bounds.width;
	CharHeight	= TextFont->ascent + TextFont->descent;
	CharAscent	= TextFont->ascent;
}
/*}}}*/
/*{{{  place cursor */
/*
 * This routine draws the current text cursor. The form of the text cursor
 * depends on whether or not the program currently has the keyboard focus.
 */
#ifdef __STDC__
static void place_cursor(void)
#else
static void place_cursor()
#endif
{
	int cursor_x;

	if (CursorX >= TextWidth)
		cursor_x = TextWidth - 1;
	else
		cursor_x = CursorX;

	CursorDrawn = 1;

	if (GotFocus)
	{
		XFillRectangle(ThisDisplay, TextWindow, CursorGc,
			cursor_x * CharWidth, CursorY * CharHeight,
			CharWidth, CharHeight);
	}
	else
	{
		XDrawRectangle(ThisDisplay, TextWindow, CursorGc,
			cursor_x * CharWidth, CursorY * CharHeight,
			CharWidth, CharHeight);
	}
}
/*}}}*/
/*{{{  remove cursor */
/*
 * This routine is used to remove the cursor, typically before drawing any
 * text. In fact, since the cursor drawing uses XOR mode removing the cursor
 * involves much the same code as placing it.
 */
#ifdef __STDC__
static void remove_cursor(void)
#else
static void remove_cursor()
#endif
{
	int cursor_x;

	if (!CursorDrawn) return;

	if (CursorX >= TextWidth)
		cursor_x = TextWidth - 1;
	else
		cursor_x = CursorX;

	CursorDrawn = 0;
	if (GotFocus)
		XFillRectangle(ThisDisplay, TextWindow, CursorGc,
			cursor_x * CharWidth, CursorY * CharHeight,
			CharWidth, CharHeight);
	else
		XDrawRectangle(ThisDisplay, TextWindow, CursorGc,
			cursor_x * CharWidth, CursorY * CharHeight,
			CharWidth, CharHeight);
}
/*}}}*/
/*{{{  flush text */
/*
 * This routine draws the text currently in the TextBuffer at the
 * current cursor position, adjusting the latter as appropriate.
 */
#ifdef __STDC__
static void flush_text(void)
#else
static void flush_text()
#endif
{
	if (TextBufferLength <= 0) return;

	XDrawImageString(ThisDisplay, TextWindow,
		(InverseVideo) ? InverseGc : NormalGc,
		CursorX * CharWidth, (CursorY * CharHeight) + CharAscent,
		TextBuffer, TextBufferLength);

	CursorX += TextBufferLength;
	TextBufferLength = 0;
}
/*}}}*/
/*{{{  escape sequence handling */
/*{{{  bell */
#ifdef __STDC__
static void bell(void)
#else
static void bell()
#endif
{
	flush_text();
	XBell(ThisDisplay, 0);
}

/*}}}*/
/*{{{  backspace */
#ifdef __STDC__
static void backspace(void)
#else
static void backspace()
#endif
{
	flush_text();
	if (CursorX > 0) CursorX--;
}
/*}}}*/
/*{{{  line feed */
#ifdef __STDC__
static void linefeed(void)
#else
static void linefeed()
#endif
{
	flush_text();
	if (CursorY < (TextHeight - 1))
		CursorY++;
	else
	{
		XCopyArea(ThisDisplay, TextWindow, TextWindow, NormalGc,
			0, CharHeight,
			 TextWidth * CharWidth,	(TextHeight - 1) * CharHeight,
			0, 0);	
		XClearArea(ThisDisplay, TextWindow,
			0, (TextHeight - 1) * CharHeight,
			TextWidth * CharWidth, TextHeight,
			FALSE);
	}
}
/*}}}*/
/*{{{  clear screen */
#ifdef __STDC__
static void clear_screen(void)
#else
static void clear_screen()
#endif
{
	TextBufferLength	= 0;
	CursorX = CursorY	= 0;
	XClearWindow(ThisDisplay, TextWindow);
}
/*}}}*/
/*{{{  carriage return */
#ifdef __STDC__
static void carriage_return(void)
#else
static void carriage_return()
#endif
{
	flush_text();
	CursorX = 0;
}
/*}}}*/
/*{{{  move cursor */
#ifdef __STDC__
static void move_cursor(int x, int y)
#else
static void move_cursor(x, y) int x; int y;
#endif
{
	flush_text();
	CursorX = x; CursorY = y;
}

/*}}}*/
/*{{{  clear to end of line */
#ifdef __STDC__
static void clear_eol(void)
#else
static void clear_eol()
#endif
{
	flush_text();
	/* Note the +1 on the width, to clear junk at the end of each line	*/
	/* following a redraw.							*/
	XClearArea(ThisDisplay, TextWindow, CursorX * CharWidth, 
		CursorY * CharHeight,
		(TextWidth + 1 - CursorX) * CharWidth, CharHeight, FALSE);
}
/*}}}*/
/*{{{  inverse video */
#ifdef __STDC__
static void inverse_video(void)
#else
static void inverse_video()
#endif
{
	flush_text();
	InverseVideo = TRUE;
}
/*}}}*/
/*{{{  normal video */
#ifdef __STDC__
static void normal_video(void)
#else
static void normal_video()
#endif
{
	flush_text();
	InverseVideo = FALSE;
}
/*}}}*/
/*{{{  cursor home */
#ifdef __STDC__
static void cursor_home(void)
#else
static void cursor_home()
#endif
{
	flush_text();
	CursorX = CursorY = 0;
}
/*}}}*/
/*{{{  process escape sequence */
/*
 * This routine tries to analyse escape sequences sent by the I/O Server:
 *  \E[%i%d;%dH		move cursor
 *  \E[K		clear to end of line
 *  \E[7m		inverse video
 *  \E[m		normal video
 *  \E[H		cursor home
 *  \E[2J		clear screen
 */
#ifdef __STDC__
static void process_escape(void)
#else
static void process_escape()
#endif
{
	if ((EscapeBufferLength >= 3) && (EscapeBuffer[1] == '['))
	{
		switch (EscapeBuffer[EscapeBufferLength - 1])
		{
		case	'H'	:
			if (EscapeBufferLength == 0)
			{
				cursor_home();
				break;
			}
			else
			{
				int	x	=	0;
				int	y	=	0;

				sscanf(&(EscapeBuffer[2]), "%d;%d", &y, &x);

				move_cursor(x-1,y-1);
			}
			break;

		case	'K'	:
			clear_eol();
			break;

		case	'm'	:
			if (EscapeBuffer[2] == '7')
				inverse_video();
			else
				normal_video();
			break;

		case	'J'	:
			clear_screen();
			break;

		default:

			/* Ignore any unrecognised sequences	*/
			break;
		}
	}

	/* Reset the escape buffer at the end.	*/
	EscapeBufferLength = 0;
}
/*}}}*/
/*}}}*/
/*{{{  add char */
/*
 * This routine is given the output text one character at a time.
 * It must recognise the following special characters and escape
 * sequences:
 *   0x07		: bell
 *   0x08		: backspace
 *   0x0A		: line feed
 *   0x0C		: clear screen
 *   0x0D		: carriage return
 *   \E[%i%d;%dH	: move cursor
 *   \E[K		: clear to end of line
 *   \E[7m		: inverse video
 *   \E[m		: normal video
 *   \E[H		: cursor home
 *   \E[2J		: clear screen
 */
#ifdef __STDC__
static void add_ch(int ch)
#else
static void add_ch(ch) int ch;
#endif
{
	if (EscapeBufferLength > 0)
	{
		EscapeBuffer[EscapeBufferLength++] = ch;
		if (isalpha(ch))
			process_escape();
	}
	else
	{
		switch (ch)
		{
		case	0x07 :	bell();			break;
		case	0x08 :	backspace();		break;
		case	0x0A :	linefeed();		break;
		case	0x0C :	clear_screen();		break;
		case	0x0D :	carriage_return();	break;

		case	0x1B :
			EscapeBuffer[EscapeBufferLength++] = ch;
			break;

		default	:
			if (isprint(ch))
			{
				TextBuffer[TextBufferLength++]	= ch;
				if (TextBufferLength == 256)
					flush_text();
			}
		}
	}
}

/*}}}*/
/*}}}*/
/*{{{  control panel */
/*{{{  callbacks */
/*{{{  main callback */
/*
 * This routine deals with most of the callbacks using the callback
 * specific data.
 */
#ifdef __STDC__
static void main_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void main_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	char	message[4];

	message[0]	= FUNCTION_CODE;
	message[1]	= WINDOW_PANEL;
	message[2]	= (char) *((int *) clientdata);
	message[3]	= 0;

	write(ChildFd, message, 4);
}
/*}}}*/
/*{{{  reboot callback */
#ifdef __STDC__
static void reboot_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void reboot_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	main_callback(w, calldata, (XtPointer) &RebootEvent);
}
/*}}}*/
/*{{{  exit callback */
#ifdef __STDC__
static void exit_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void exit_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	main_callback(w, calldata, (XtPointer) &ExitEvent);
}

/*}}}*/
/*{{{  debugger callback */
#ifdef __STDC__
static void debugger_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void debugger_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	main_callback(w, calldata, (XtPointer) &DebuggerEvent);
}

/*}}}*/
/*{{{  status callback */
#ifdef __STDC__
static void status_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void status_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	main_callback(w, calldata, (XtPointer) &StatusEvent);
}

/*}}}*/
/*{{{  logger callback */
/*
 * This callback is used when the logger button is pressed. It has to
 * update the label on the button as well as send an event.
 */
#ifdef __STDC__
static void logger_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void logger_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	reset_args();

	switch(LoggerState)
	{
	case	WIN_LOG_SCREEN	:
		set_arg(XtNlabel,	"Logger - File");
		LoggerState	= WIN_LOG_FILE;
		main_callback(w, NULL, (XtPointer) &LoggerState);
		break;

	case	WIN_LOG_FILE	:
		set_arg(XtNlabel,	"Logger - Both");
		LoggerState	= WIN_LOG_BOTH;
		main_callback(w, NULL, (XtPointer) &LoggerState);
		break;

	case 	WIN_LOG_BOTH	:
	default			:
		set_arg(XtNlabel,	"Logger - Screen");
		LoggerState	= WIN_LOG_SCREEN;
		main_callback(w, NULL, (XtPointer) &LoggerState);
		break;
	}

	XtSetValues(w, args);
}

/*}}}*/
/*{{{  all callback */
/*
 * This callback function is invoked when the user presses the left
 * mouse button on the debug widget, to set or clear all debugging
 * options.
 */
#ifdef __STDC__
static void all_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void all_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	int	i;
	int	all_clear	= TRUE;

	for (i = 0; MenuOptions[i].Name != NULL; i++)
		if (MenuOptions[i].Set)
			{ all_clear = FALSE; break; }

	for (i = 0; MenuOptions[i].Name != NULL; i++)
	{
		if (all_clear)
		{
			MenuOptions[i].Set	= TRUE;
			reset_args();
			set_arg(XtNrightBitmap, TickBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
		else if (MenuOptions[i].Set)
		{
			MenuOptions[i].Set	= FALSE;
			reset_args();
			set_arg(XtNrightBitmap, ClearBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
	}
	main_callback(w, calldata, (XtPointer) &DebugEvent);
}		
/*}}}*/
/*{{{  menu callback */
/*
 * This callback is invoked for a single menu option.
 */
#ifdef __STDC__
static void menu_callback(Widget w, XtPointer calldata, XtPointer clientdata)
#else
static void menu_callback(w, calldata, clientdata) Widget w; XtPointer calldata, clientdata;
#endif
{
	int	i;

	for (i = 0; MenuOptions[i].Name != NULL; i++)
		if (MenuOptions[i].Widget == w)
			break;

	if (MenuOptions[i].Name == NULL)
	{
		return;
	}
	
	if ((MenuOptions[i].Value != WIN_MEMORY) && (MenuOptions[i].Value != WIN_RECONF))
	{
		if (MenuOptions[i].Set)
		{
			MenuOptions[i].Set	= FALSE;
			reset_args();
			set_arg(XtNrightBitmap, ClearBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
		else
		{
			MenuOptions[i].Set	= TRUE;
			reset_args();
			set_arg(XtNrightBitmap, TickBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
	}

	main_callback(w, calldata, (XtPointer) &(MenuOptions[i].Value));
}

/*}}}*/
/*{{{  resize and redraw */
/*
 * This routine deals with resizing and expose events. These are all handled
 * by sending a resize request to the I/O Server which responds by a
 * a screen redraw. Not particularly efficient, but it works.
 */
#ifdef __STDC__
static void resize_callback(void)
#else
static void resize_callback()
#endif
{ 
	Window		return_window;
  	int		x_return, y_return;
	unsigned int	border_width;
	unsigned int	depth;
	char		message[4];

	XGetGeometry(ThisDisplay, TextWindow, &return_window,
		 &x_return, &y_return, 	&TextWidth, &TextHeight,
		 &border_width, &depth);

	TextWidth	/= CharWidth;
	TextHeight	/= CharHeight;

	message[0]	= FUNCTION_CODE;
	message[1]	= WINDOW_SIZE;
	message[2]	= TextHeight;
	message[3]	= TextWidth;
	write(ChildFd, message, 4);
}
/*}}}*/
/*}}}*/
/*{{{  panel update */
/*
 * This code deals with panel update messages sent by the I/O Server, when
 * the window's idea of the current debugging options differ from the
 * I/O Server's idea.
 */
#ifdef __STDC__
static void panel_update(int event, int value)
#else
static void panel_update(event, value) int event; int value;
#endif
{
	int	i;

	reset_args();

	switch(event)
	{
	case	WIN_LOG_FILE :
		set_arg(XtNlabel,	"Logger - File");
		LoggerState	= WIN_LOG_FILE;
		XtSetValues(LoggerWidget, args);
		break;

	case	WIN_LOG_SCREEN :
		set_arg(XtNlabel,	"Logger - Screen");
		LoggerState	= WIN_LOG_SCREEN;
		XtSetValues(LoggerWidget, args);
		break;

	case	WIN_LOG_BOTH :
		set_arg(XtNlabel,	"Logger - Both");
		LoggerState	= WIN_LOG_BOTH;
		XtSetValues(LoggerWidget, args);
		break;

	default	:
		for (i = 0; MenuOptions[i].Name != NULL; i++)
			if (MenuOptions[i].Value == event)
				break;
		if (MenuOptions[i].Value != event) return;
		if (MenuOptions[i].Set)
		{
			MenuOptions[i].Set	= FALSE;
			set_arg(XtNrightBitmap, ClearBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
		else
		{
			MenuOptions[i].Set	= TRUE;
			set_arg(XtNrightBitmap, TickBitmap);
			XtSetValues(MenuOptions[i].Widget, args);
		}
	}				
	
}

/*}}}*/
/*{{{  setup the debug menu */
/*
 * This code sets up the debug menu. This involves creating a popup
 * shell, registering all the menu options, and setting up a new
 * translation table.
 */
#ifdef __STDC__
static void setup_debug_menu(Widget debug_widget)
#else
static void setup_debug_menu(debug_widget) Widget debug_widget;
#endif
{
	static char	*translation_table = "\
#augment\n\
<EnterWindow>:	highlight()\n\
<LeaveWindow>:	reset()\n\
<Btn1Down>:	set() notify()\n\
<Btn1Up>:	reset()\n\
<Btn2Down>:	reset() MenuPopup(\"DebugMenu\")\n\
<Btn3Down>:	reset() XawPositionSimpleMenu(\"DebugMenu\")  MenuPopup(\"DebugMenu\")\n\
";
	Widget		menu_widget;
	int		i;
	XtTranslations	tab;

	printf ("\t@setup_debug_menu ()");

#if 1
	XawSimpleMenuAddGlobalActions(ThisContext);

	printf ("\t\tsetup_debug_menu () <1>\n");
#endif
	reset_args();

	printf ("\t\tsetup_debug_menu () <2>\n");

	menu_widget = XtCreatePopupShell("DebugMenu", simpleMenuWidgetClass, 
		debug_widget, args);

	printf ("\t\tsetup_debug_menu () <3>\n");

	for (i = 0; MenuOptions[i].Name != NULL; i++)
	{
		printf ("\t\tsetup_debug_menu () <4.%d start>\n", i);

		reset_args();
		set_arg(XtNlabel,	MenuOptions[i].Name);
		set_arg(XtNrightMargin,	10);
		set_arg(XtNrightBitmap, ClearBitmap);
		MenuOptions[i].Widget	= XtCreateManagedWidget(MenuOptions[i].Name,
			smeBSBObjectClass, menu_widget, args);
		XtAddCallback(MenuOptions[i].Widget, XtNcallback, fn(menu_callback),
			(XtPointer) &(MenuOptions[i].Value));
	
		printf ("\t\tsetup_debug_menu () <4.%d end>\n", i);
	}

	printf ("\t\tsetup_debug_menu () <5>\n");

	tab	= XtParseTranslationTable(translation_table);

	printf ("\t\tsetup_debug_menu () <6>\n");

	XtAugmentTranslations(debug_widget, tab);

	printf ("\t\tsetup_debug_menu () <7>\n");

	XtAddCallback(debug_widget, XtNcallback, fn(all_callback), NULL);

	printf ("\t\tsetup_debug_menu () <8>\n");
}



/*}}}*/
/*{{{  create control panel */
/*
 * This routine is used to create the main control panel. This consists
 * of the reboot, exit, status and possibly the debugger buttons,
 * the logger button, and the debug button with its associated
 * pulldown menu.
 */
static void create_control_panel()
{
	Widget	reboot_widget;
	Widget	debugger_widget;
	Widget	status_widget;
	Widget	exit_widget;
	Widget	debug_widget;
	Widget	menu_widget;

	printf ("@create_control_panel ()\n");

	reset_args();
	set_arg(XtNleft,	XtChainLeft);
	set_arg(XtNright,	XtChainLeft);
	set_arg(XtNtop,		XtChainTop);
	set_arg(XtNbottom,	XtChainTop);
	set_arg(XtNshapeStyle,	XmuShapeRoundedRectangle);

	printf ("\tcreate_control_panel () <1>\n");

	reboot_widget = XtCreateManagedWidget("Reboot", commandWidgetClass, ControlPanel, args);
	printf ("\tcreate_control_panel () <2>\n");

	XtAddCallback(reboot_widget, XtNcallback, fn(reboot_callback), NULL);

	printf ("\tcreate_control_panel () <3>\n");

	if (ShowDebugger)
	{
		reset_args();
		set_arg(XtNleft,	XtChainLeft);
		set_arg(XtNright,	XtChainLeft);
		set_arg(XtNtop,		XtChainTop);
		set_arg(XtNbottom,	XtChainTop);
		set_arg(XtNfromHoriz, reboot_widget);
		set_arg(XtNshapeStyle,	XmuShapeRoundedRectangle);
		debugger_widget = XtCreateManagedWidget("Debugger", commandWidgetClass, ControlPanel, args);
		XtAddCallback(debugger_widget, XtNcallback, fn(debugger_callback), NULL);
	}

	printf ("\tcreate_control_panel () <4>\n");

	reset_args();
	if (ShowDebugger)
		{ set_arg(XtNfromHoriz, debugger_widget); }
	else
		{ set_arg(XtNfromHoriz, reboot_widget); }
	set_arg(XtNshapeStyle, XmuShapeRoundedRectangle);
	set_arg(XtNleft,	XtChainLeft);
	set_arg(XtNright,	XtChainLeft);
	set_arg(XtNtop,		XtChainTop);
	set_arg(XtNbottom,	XtChainTop);
	status_widget = XtCreateManagedWidget("Status", commandWidgetClass, ControlPanel, args);
	XtAddCallback(status_widget, XtNcallback, fn(status_callback), NULL);

	reset_args();
	set_arg(XtNfromHoriz,	status_widget);
	set_arg(XtNshapeStyle,	XmuShapeRoundedRectangle);
	set_arg(XtNleft,	XtChainLeft);
	set_arg(XtNright,	XtChainLeft);
	set_arg(XtNtop,		XtChainTop);
	set_arg(XtNbottom,	XtChainTop);
	exit_widget = XtCreateManagedWidget("Exit", commandWidgetClass, ControlPanel, args);
	XtAddCallback(exit_widget, XtNcallback, fn(exit_callback), NULL);

	printf ("\tcreate_control_panel () <5>\n");

	reset_args();
	set_arg(XtNfromHoriz,	exit_widget);
	set_arg(XtNlabel,	"Logger - Screen");
	set_arg(XtNshapeStyle,	XmuShapeRoundedRectangle);
	set_arg(XtNleft,	XtChainLeft);
	set_arg(XtNright,	XtChainLeft);
	set_arg(XtNtop,		XtChainTop);
	set_arg(XtNbottom,	XtChainTop);
	LoggerWidget	= XtCreateManagedWidget("logger", commandWidgetClass, ControlPanel, args);
	XtAddCallback(LoggerWidget,  XtNcallback, fn(logger_callback), NULL);

	printf ("\tcreate_control_panel () <6>\n");

	reset_args();
	set_arg(XtNleft,		XtChainRight);
	set_arg(XtNright,		XtChainRight);
	set_arg(XtNtop,			XtChainTop);
	set_arg(XtNbottom,		XtChainTop);
	set_arg(XtNhorizDistance,	(80 * CharWidth) - 2);
	set_arg(XtNshapeStyle,		XmuShapeRoundedRectangle);
	set_arg(XtNmenuName,		"DebugMenu");
	debug_widget	= XtCreateManagedWidget("Debug", commandWidgetClass, ControlPanel, args);
	printf ("\tcreate_control_panel () <7>\n");

	setup_debug_menu(debug_widget);

	printf ("\tcreate_control_panel () <8>\n");
}
 
/*}}}*/
/*}}}*/
/*{{{  actions in the tty window */
/*{{{  key press */
/*
 * This event handler copes with input handling. Fortunately X appears to
 * take care of auto-repeat etc. XRebindKeysym() will have been used to
 * establish bindings for cursor keys, function keys, etc. in accordance
 * with the current termcap entries.
 */
#ifdef __STDC__
static void key_press(Widget w, XKeyEvent *event, String *params, Cardinal *num_params)
#else
static void key_press(w, event, params, num_params) Widget w; XKeyEvent *event; String *params; Cardinal *num_params;
#endif
{
	char	buf[16];
	int	len;
	int	i;
	KeySym	sym;

	len = XLookupString(event, buf, 15, &sym, NULL);

	if (len == 0)
		return;

	write(ChildFd, buf, len);
}
/*}}}*/
/*{{{  focus in */
#ifdef __STDC__
static void focusin(Widget w, XFocusInEvent *event, String *params, Cardinal *num_params)
#else
static void focusin(w, event, params, num_params) Widget w; XFocusInEvent *event; String *params; Cardinal *num_params;
#endif
{
	if (GotFocus) return;
	remove_cursor();
	GotFocus	= TRUE;
	place_cursor();
}
/*}}}*/
/*{{{  focus out */
#ifdef __STDC__
static void focusout(Widget w, XFocusOutEvent *event, String *params, Cardinal *num_params)
#else
static void focusout(w, event, params, num_params) Widget w; XFocusOutEvent *event; String *params; Cardinal *num_params;
#endif
{
	if (!GotFocus) return;
	remove_cursor();
	GotFocus	= FALSE;
	place_cursor();
}
/*}}}*/
/*{{{  expose */
#ifdef __STDC__
static void expose(Widget w, XExposeEvent *event, String *params, Cardinal *num_params)
#else
static void expose(w, event, params, num_params) Widget w; XExposeEvent *event; String *params; Cardinal *num_params;
#endif
{
	if (event->count == 0)
		resize_callback();
}
/*}}}*/
/*{{{  configure */
#ifdef __STDC__
static void configure(Widget w, XConfigureEvent *event, String *params, Cardinal *num_params)
#else
static void configure(w, event, params, num_params) Widget w; XConfigureEvent *event; String *params; Cardinal *num_params;
#endif
{
	resize_callback();
}
/*}}}*/
/*{{{  setup actions */
/*
 * This routine installs a set of actions for the text window together
 * with a suitable translation table. 
 */

#define act_fn(f)	(XtActionProc)(f)
#ifdef __STDC__
static void setup_actions(Widget w)
#else
static void setup_actions(w) Widget w;
#endif
{
	static	XtActionsRec actions[] = {
		{ "keypress",	(XtActionProc)(fn(key_press)) },
		{ "focusin",	(XtActionProc)(fn(focusin))   },
		{ "focusout",	(XtActionProc)(fn(focusout))  },
		{ "expose",	(XtActionProc)(fn(expose))    },
		{ "configure",	(XtActionProc)(fn(configure)) },
		};
	static	char *translations = "\
#augment\n\
<KeyPress>:	keypress()\n\
<FocusIn>:	focusin()\n\
<FocusOut>:	focusout()\n\
<Expose>:	expose()\n\
<Configure>:	configure()\n\
";
	XtTranslations	my_translations;

	XtAppAddActions(ThisContext, actions, XtNumber(actions));
	my_translations = XtParseTranslationTable(translations);
	XtAugmentTranslations(w, my_translations);	
}

/*}}}*/
/*{{{  setup keyboard bindings */
/*
 * This routine defines keyboard bindings for function keys, cursor keys
 * etc. in accordance with the current termcap database.
 */
static struct {
	KeySym		 sym;
	char		*termcap_name;
} keys_table[] = {
	{ XK_F1,	"k1" },
	{ XK_F2,	"k2" },
	{ XK_F3,	"k3" },
	{ XK_F4,	"k4" },
	{ XK_F5,	"k5" },
	{ XK_F6,	"k6" },
	{ XK_F7,	"k7" },
	{ XK_F8,	"k8" },
	{ XK_F9,	"k9" },
	{ XK_F10,	"k;" },	/* function key F10	*/
	{ XK_Up,	"ku" },
	{ XK_Down,	"kd" },
	{ XK_Right,	"kr" },
	{ XK_Left,	"kl" },
	{ XK_End,	"@7" },	/* end			*/
	{ XK_Next,	"kN" },	/* PageDown		*/
	{ XK_Prior,	"kP" }, /* PageUp		*/
	{ XK_Home,	"kH" },	/* Home			*/
	{ -1,		NULL }
};

#if (SUN4 || SOLARIS)
#ifdef __STDC__

#ifdef __cplusplus
extern "C"
{
#endif
extern int	 tgetent(char *, char *);
extern char	*tgetstr(char *, char **);
#ifdef __cplusplus
}
#endif

#else
extern int	 tgetent();
extern char	*tgetstr();
#endif
#endif
	
#ifdef __STDC__
static void setup_keyboard_bindings(void)
#else
static void setup_keyboard_bindings()
#endif
{
	char		*term_type	= getenv("TERM");
	static char	 termcap_buffer[1024];
	static char	 termcap_data[1024];
	int		 res;
	int		 i;
	char		*termcap_str;
	char		*termcap_index;

	if (term_type == NULL)
		term_type = "xterm";

	res = tgetent(termcap_data, term_type);
	if (res == 0) res = tgetent(termcap_data, "sun");
	if (res == 0) res = tgetent(termcap_data, "vt100");
	if ((res == 0) || (res == -1))
	{
		fputs("serverxt: failed to get termcap keyboard details.\n", stderr);
		exit(EXIT_FAILURE);
	}
	termcap_index	= termcap_buffer;

	for (i = 0; keys_table[i].termcap_name != NULL; i++)
	{
		KeySym	modifier;
		termcap_str = tgetstr(keys_table[i].termcap_name, &termcap_index);
		if (termcap_str == NULL) continue;
#if SOLARIS
		XRebindKeysym(ThisDisplay, keys_table[i].sym,
			&modifier, 0, (const unsigned char *)termcap_str, strlen((const char *)termcap_str));
#else
		XRebindKeysym(ThisDisplay, keys_table[i].sym,
			&modifier, 0, termcap_str, strlen(termcap_str));
#endif
	}
}	

/*}}}*/
/*}}}*/
/*{{{  input from the I/O Server */

/*
 * This routine copes with input from the I/O Server.
 */
#ifdef __STDC__
static void from_ioserver(void)
#else
static void from_ioserver()
#endif
{
	unsigned char	buf[512];
	int		how_many;
	int		i;

	how_many = read(ChildFd, buf, 512);

	if (how_many <= 0)
	{
		exit(1);
	}

	remove_cursor();

	for (i = 0; i < how_many; i++)
	{
		if (buf[i] == FUNCTION_CODE)
		{
			if ((i + 3) >= how_many) break;
			if (buf[i+1] == WINDOW_KILL)
			{
				exit(1);
			}
			if (buf[i+1] == WINDOW_PANEL)
			{
				panel_update(buf[i+2], buf[i+3]);
			}
			i += 3;
		}
		else
		{
			add_ch(buf[i]);
		}
	}

	flush_text();			

	place_cursor();
}

/*}}}*/
/*{{{  main() */
/*
** main(). The X toolkit server window program should only ever be
** executed by the I/O Server so the arguments are pre-determined.
*/
#ifdef __STDC__
int main(int argc, char **argv)
#else
int main(argc, argv) int argc; char **argv;
#endif
{
	printf ("%s starting up ...\n", argv[0]);

	/* 1) Parse arguments						*/

	printf ("%s - <1>\n", argv[0]);

	if (argc != 5)
	{
		fputs("serverxt: this program can be run only by the I/O Server.\n", stderr);
		exit(EXIT_FAILURE);
	}

	Title		= argv[1];
	IOServerWindow	= (argv[2][0] == '1');
	ChildFd		= atoi(argv[3]);
	ShowDebugger	= (argv[4][0] == '1');

	/* 2) Initialise the toolkit library.				*/
	printf ("%s - <2>\n", argv[0]);

	argc = 1; argv[1] = NULL;
	TopLevel	= XtAppInitialize(&ThisContext, ApplicationName,
				NULL, 0, &argc, argv, NULL, NULL, 0);
	ThisDisplay	= XtDisplay(TopLevel);
	ThisScreen	= DefaultScreen(ThisDisplay);

	/* 3) Determine the font to use for the text window.		*/
	printf ("%s - <3>\n", argv[0]);

	get_font_details();

	/* 4) Create the top level window. Its size will be controlled	*/
	/*    by its contents.						*/
	printf ("%s - <4>\n", argv[0]);

	reset_args();
	WholeWindow	= XtCreateManagedWidget(ApplicationName, formWidgetClass , TopLevel, args);

	/* 5) If appropriate, create the buttons and menus.		*/
	printf ("%s - <5>\n", argv[0]);

	if (IOServerWindow)
	{
		printf ("%s - <5a>\n", argv[0]);
		reset_args();
		printf ("%s - <5b>\n", argv[0]);
		set_arg(XtNtop,			XtChainTop);
		set_arg(XtNbottom,		XtChainTop);
		set_arg(XtNleft,		XtChainLeft);
		set_arg(XtNright,		XtChainRight);
		set_arg(XtNwidth,		(CharWidth * 80) - 2);
		set_arg(XtNdefaultDistance, 	12);
		printf ("%s - <5c>\n", argv[0]);
    		ControlPanel = XtCreateManagedWidget("control", formWidgetClass, WholeWindow, args);
		printf ("%s - <5d>\n", argv[0]);
		create_control_panel();
		printf ("%s - <5e>\n", argv[0]);
	}

	/* 6) Create a simple widget which will be used for text. When the	*/
	/*    window is resized it is this window which grows.			*/
	printf ("%s - <6>\n", argv[0]);
	reset_args();
	set_arg(XtNborderWidth, 0);
	if (IOServerWindow)
	{
		set_arg(XtNvertDistance, 0);
		set_arg(XtNfromVert, ControlPanel);
	}
	else
		set_arg(XtNtop,		XtChainTop);

	set_arg(XtNleft,		XtChainLeft);
	set_arg(XtNright,		XtChainRight);
	set_arg(XtNbottom,		XtChainBottom);
	set_arg(XtNborderWidth,		0);
	if (IOServerWindow)
		{ set_arg(XtNheight, CharHeight * 15); }
	else
		{ set_arg(XtNheight, CharHeight * 25); }
	set_arg(XtNwidth, CharWidth * 80);
	TextWidget	= XtCreateManagedWidget("tty", simpleWidgetClass, WholeWindow, args);
	XtSetKeyboardFocus(WholeWindow, TextWidget);

	/* 7) Set up suitable actions for this window. This copes with keyboard	*/
	/*    input, resizing, redrawing, etc.					*/
	printf ("%s - <7>\n", argv[0]);
	setup_actions(TextWidget);

	/* 8) Get the window on the screen.					*/
	printf ("%s - <8>\n", argv[0]);

	XtRealizeWidget(TopLevel);

	/* 9) Now the widget is on the screen, set up the text handling. This	*/
	/*    involves some keyboard rebinding to define cursor keys etc., and	*/
	/*    creating suitable graphics context for normal text, inverse text,	*/
	/*    and drawing the text cursor.					*/
	printf ("%s - <9>\n", argv[0]);
	TextWindow	= XtWindow(TextWidget);

	setup_keyboard_bindings();
	{
		XGCValues	values;

		values.foreground	= BlackPixel(ThisDisplay, ThisScreen);
		values.background	= WhitePixel(ThisDisplay, ThisScreen);
		values.font		= TextFont->fid;
		NormalGc		= XCreateGC(ThisDisplay, TextWindow,
			(GCForeground | GCBackground | GCFont),
			&values);

		values.foreground	= WhitePixel(ThisDisplay, ThisScreen);
		values.background	= BlackPixel(ThisDisplay, ThisScreen);
		values.font		= TextFont->fid;
		InverseGc		= XCreateGC(ThisDisplay, TextWindow,
			(GCForeground | GCBackground | GCFont),
			&values);

		values.foreground	= BlackPixel(ThisDisplay, ThisScreen);
		values.background	= WhitePixel(ThisDisplay, ThisScreen);
		values.function		= GXxor;
		values.line_width	= 1;
		CursorGc		= XCreateGC(ThisDisplay, TextWindow,
			(GCFunction | GCForeground | GCBackground | GCLineWidth),
			&values);
	}
 
	/* 10) Once the window has been realized it can be given an icon. It is	*/
	/*     also necessary to negotiate with the Window Manager for the	*/
	/*     input focus.							*/
	printf ("%s - <10>\n", argv[0]);
	{
		XWMHints	hints;
		Pixmap		ioserv_icon;
		ioserv_icon = XCreateBitmapFromData(ThisDisplay,
#if SOLARIS
			 XtWindow(TopLevel), (const char *)(ioserv_bits), ioserv_width, ioserv_height);
#else
			 XtWindow(TopLevel), ioserv_bits, ioserv_width, ioserv_height);
#endif

		XSetStandardProperties(ThisDisplay, XtWindow(TopLevel), Title, Title,
					ioserv_icon, argv, argc, NULL);
		reset_args();
		set_arg(XtNiconPixmap, ioserv_icon);
		if (IOServerWindow)
		{	set_arg(XtNiconName, "Server");	}
		else
		{	set_arg(XtNiconName, Title);	}
		XtSetValues(TopLevel, args);

		hints.input	  	= TRUE;
		hints.icon_pixmap	= ioserv_icon;
		hints.flags	  	= InputHint | IconPixmapHint;
		XSetWMHints(ThisDisplay, XtWindow(TopLevel), &hints);
	}

	/* 11) Also create the icon for the menu tick.				*/
	printf ("%s - <11>\n", argv[0]);

	if (IOServerWindow)
	{
#if SOLARIS
		TickBitmap = XCreateBitmapFromData(ThisDisplay, XtWindow(TopLevel),
					(const char *)tick_bits, tick_width, tick_height);
#else
		TickBitmap = XCreateBitmapFromData(ThisDisplay, XtWindow(TopLevel),
					tick_bits, tick_width, tick_height);
#endif
		ClearBitmap = XCreateBitmapFromData(ThisDisplay, XtWindow(TopLevel),
				clear_bits, clear_width, clear_height);
	}

	/* 12) Allow for input from the I/O Server.				*/
	printf ("%s - <12>\n", argv[0]);
	XtAppAddInput(ThisContext, ChildFd, (XtPointer)XtInputReadMask, (XtInputCallbackProc)(fn(from_ioserver)), NULL);

	/* Finally, invoke the toolkit main loop.				*/

#if 1
	printf ("%s - entering loop\n", argv[0]);
	XtAppMainLoop(ThisContext);
#else
	for (;;)
	{
		XEvent		event;
		XtAppNextEvent(ThisContext, &event);
		XtDispatchEvent(&event);
	}
#endif

	printf ("%s stopping\n", argv[0]);
}

/*}}}*/

/*{{{  Patch for Solaris 1.x */
#if defined(SUN4)
/*
 * On Solaris 1.x the Xmu library appears to be fouled up. This sorts out
 * the mess.
 */

#include <X11/IntrinsicP.h>

/*
 * The following hack is used by XmuConvertStandardSelection to get the
 * following class records.  Without these, a runtime undefined symbol error 
 * occurs.
 */
extern WidgetClass applicationShellWidgetClass,wmShellWidgetClass;

WidgetClass get_applicationShellWidgetClass()
{
    return applicationShellWidgetClass;
}

WidgetClass get_wmShellWidgetClass()
{
    return wmShellWidgetClass;
}
#endif
/*}}}*/

