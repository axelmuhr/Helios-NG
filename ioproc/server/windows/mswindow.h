/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  mswindow.h                                                          --
--                                                                      --
--  Author:  BLV 21/5/92                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1987, Perihelion Software Ltd.                         */


typedef struct DeviceInfo { ObjInfo obj;
			  } DeviceInfo;

extern void Repaint_Screen(HWND, int, int, int, int);
extern void new_msmouse(SHORT, SHORT, word, int);
extern void get_key_status(LONG, short far *, short far *, short far *);
extern int  MsMouse_Active;
extern int  MsEvent_Active;
extern void do_msevent(void);
extern void do_msmouse(void);
extern void new_msevent(SHORT, SHORT, word, word);
extern int  non_char_keys(HWND, int, LONG);
extern void process_keyboard(HWND, int, LONG);
extern void create_a_caret(HWND);
extern void delete_a_caret(HWND);
extern void show_caret(HWND, int, int);
extern void hide_caret(HWND);

extern void Set_Up_Window(HANDLE);
extern void Set_Window_Size(HWND);
extern Window *find_window(HWND);
extern int  err_out(char *text, ...);

#if MSWINDOWS
#ifdef Windows_Module
char   *szAppName = "Helios Server", *szPopupClass = "Shells";
char   *Print_Name = "Printer";
int    MAXLINES, MAXCOLS, CHAR_WIDTH, CHAR_HEIGHT, Caret_Offset;
int    graphics_registered, print_registered;
HANDLE PROGINSTANCE;
#else
extern int    MAXCOLS, MAXLINES, CHAR_WIDTH, CHAR_HEIGHT, Caret_Offset;
extern int    graphics_registered, print_registered;
extern HANDLE PROGINSTANCE;
extern int    global_nCmdShow;
extern char   *szAppName, *szPopupClass, *Print_Name;
#endif

#ifdef Graphics_Module
BOOL   SkipTimer = TRUE;
void   CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
FARPROC lpfnDdeCallback;
#else
extern BOOL   SkipTimer;
extern void   CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
extern FARPROC lpfnDdeCallback;
#endif

#ifdef DDE_Module
DWORD CALLBACK DdeCallback(UINT, UINT, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
#else
extern DWORD CALLBACK DdeCallback(UINT, UINT, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
#endif

#endif

extern void redraw_screen(Window *, word);
extern void fn( clear_screen,      (void));
extern void fn( clear_eol,         (void));
extern void fn( move_to,           (int, int));
extern void fn( refresh_screen,    (int, int));
extern void fn( backspace,         (void));
extern void fn( carriage_return,   (void));
extern void fn( linefeed,          (void));
extern void fn( set_mark,          (void));
extern void fn( use_mark,          (void));
extern void fn( foreground,        (int));
extern void fn( background,        (int));
extern void fn( set_bold,          (int));
extern void fn( set_italic,        (int));
extern void fn( set_underline,     (int));
extern void fn( set_inverse,       (int));
extern void fn( ring_bell,         (void));

extern void delete_window(Window *);

