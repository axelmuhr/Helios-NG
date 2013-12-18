/***************************************************************************/
/***************************************************************************/
/**             Helios I/O Server under Microsoft Windows                 **/
/**                                                                       **/
/**             Copyright (C) 1993, Perihelion Software Ltd.		  **/
/**                           All Rights Reserved.                        **/
/**                                                                       **/
/**   winsrvr.c                                                           **/
/**                                                                       **/
/**     This file contains the main driver engine of the I/O Server.      **/
/**     The file initiates the startup procedure of the I/O Server, and   **/
/**     contains the most outer loop of the server. The file also         **/
/**     contains the I/O Server interface with Microsoft Windows.         **/
/**                                                                       **/
/**   Author : S.A. Wilson  05/90   (Code incorporates original code by   **/
/**                                 (BLV. of Perihelion Software Ltd.  )  **/
/**                                                                       **/
/**   Site   : CSIR - MIKOMTEK                                            **/
/**									  **/
/**   Rewritten: BLV, Mike Gunning, eliminating most of the hacks put in. **/
/**                                                                       **/
/***************************************************************************/
/***************************************************************************/

#define Windows_Module

#include "helios.h"
#include "sccs.h"
#include "windows\menus.h"

static char *argv[20];  /* for command line arguments */
int          argc;      /* for no. of cmd line args */
PRIVATE HANDLE  global_hInstance;
        int     global_nCmdShow;

long FAR PASCAL WndProc(HWND, unsigned, WORD, LONG);
long FAR PASCAL PopupProc(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL AboutDlgProc(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL DbgBoxProc(HWND, unsigned, WORD, LONG);

#include "windows\graph.h"      /* deferred message calls */

extern DirHeader Graph_List;
extern long FAR PASCAL GraphProc(HWND, unsigned, WORD, LONG);

void getcmdline (LPSTR lpszCmdLine, int *argc, char **argv);
extern int server_main(int argc, char **argv);

extern void  add_to_key_buffer(HWND, LPSTR);
PRIVATE void Update_Debug_Options(HWND);
PRIVATE void Check_Current_Boxes(HWND);
extern  void show_logger_name(HWND);
extern  void set_logger_name(HWND);

#define WM_TIDYDIALOG   (WM_USER+1)
PRIVATE char *configname = "host.con";
PRIVATE bool hid_program_manager = FALSE;
PRIVATE HWND hDbgDialog = NULL;
PRIVATE HWND hAboutDialog = NULL;
PRIVATE FARPROC lpfnDlgBox = NULL;
PRIVATE FARPROC lpfnAboutBox = NULL;


/**************************************************************************/
/*                                                                        */
/* The function WinMain provides the I/O server with the standard Windows */
/* application entry point.                                               */
/*                                                                        */
/* The function is the main driver routine for the I/O server, providing  */
/* the interface between the two message passing systems of Helios and    */
/* Microsoft Windows, via the MainLoop and PeekMessage calls.             */
/*                                                                        */
/* The function registers two window classes, the first being the main    */
/* server window, and the other class being the console window and other  */
/* shell windows. Once the two classes have been registered, the main     */
/* server window is opened.                                               */
/*                                                                        */
/* If the graphics server is incorporated, a seperate graphics window     */
/* class is registered. The same applies if the graphics printer server   */
/* is incorporated.                                                       */
/*                                                                        */
/* The function checks the version of Microsoft Windows, and determines   */
/* whether the server will be able to function under that particular      */
/* version. The server's environment is then set up, via a call to        */
/* server_main.                                                           */
/*                                                                        */
/* Once the server has been set up, the function enters the main outer    */
/* loop of the server, where it remains until the server is terminated.   */
/* Upon entry to the main loop, the server is initialised. (This must     */
/* only happen the first time the loop is entered or when the server is   */
/* rebooted) Once initialised, the server runs until completion,          */
/* servicing mesages from the transputer and from the Microsoft Windows.  */
/*                                                                        */
/**************************************************************************/


int PASCAL WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
   HANDLE     hInstance, hPrevInstance;
   LPSTR      lpszCmdLine;
   int        nCmdShow;
{
  int ret;

  global_hInstance      = hInstance;
  PROGINSTANCE          = hInstance;
  global_nCmdShow       = nCmdShow;
  Server_window.handle  = 0L;
  real_windows          = 1;

        /* Only one I/O Server please   */
  if (hPrevInstance)
   return(1);

  InitTable();   /* initialise the HashTable for the open windows */

  getcmdline(lpszCmdLine, &argc, argv);
  ret = server_main(argc, argv);

  if (hid_program_manager)
  {
      HWND hWnd = FindWindow(NULL, "Program Manager");
      if (hWnd ne NULL)
          ShowWindow(hWnd, SW_RESTORE);
  }

  if (IsWindow(hDbgDialog))
      DestroyWindow(hDbgDialog);
  if (IsWindow(hAboutDialog))
      DestroyWindow(hAboutDialog);
  if (lpfnDlgBox != NULL)
      FreeProcInstance(lpfnDlgBox);
  if (lpfnAboutBox != NULL)
      FreeProcInstance(lpfnAboutBox);

  return ret;
}

void initialise_Windows(void)
{ char  szCaption [] = "Helios I/O Server";     /* caption for main server */
  WNDCLASS     wndclass;
  HWND         hWnd;
  HDC          deskDC;
  int          colors,
               count = 0,     /* counter */
               CARRYON = 1,   /* flag used in the main loop */
               xdim, ydim;    /* dimensions of the graphics screen */
  char *hide_program_manager = get_config("hide_program_manager");

/* We attempt to access the Windows Desktop in order to determine the number */
/* of colors Windows is able to display. We then can use either the color    */
/* icons, or the monochrome icons. */

  hWnd = GetDesktopWindow();
  if (hWnd eq 0) colors = 2;   /* if we cant access desktop, use mono */
    else {
        deskDC = GetDC(hWnd);
        if (deskDC eq 0) colors = 2;
            else {
                colors = GetDeviceCaps(deskDC, NUMCOLORS);
                ReleaseDC(hWnd, deskDC);
            }
    }


  /* Register the main server window class */

    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE | CS_BYTEALIGNCLIENT;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = global_hInstance;
    if (colors > 7)
       wndclass.hIcon         = LoadIcon(global_hInstance, "mainicon");
    else
       wndclass.hIcon         = LoadIcon(global_hInstance, "monomain");
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = COLOR_WINDOW + 1;
    wndclass.lpszMenuName  = "MainMenu";
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass (&wndclass)) longjmp(exit_jmpbuf, 1);

/* Register the console and shell window class */

    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE | CS_BYTEALIGNCLIENT;
    wndclass.lpfnWndProc   = PopupProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = global_hInstance;
    if (colors > 7)
       wndclass.hIcon         = LoadIcon(global_hInstance, "shellicon");
    else
       wndclass.hIcon         = LoadIcon(global_hInstance, "monoshell");
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = COLOR_WINDOW + 1;
    wndclass.lpszMenuName  = "ShellMenu";
    wndclass.lpszClassName = szPopupClass;

    if (!RegisterClass (&wndclass)) longjmp(exit_jmpbuf, 1);

    graphics_registered = 1;

  if (hide_program_manager ne NULL)
  {
      HWND hWnd = FindWindow(NULL, "Program Manager");
      if (hWnd ne NULL)
      {
          ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
          hid_program_manager = TRUE;
      }
  }

  xdim = GetSystemMetrics(SM_CXSCREEN);   /* dimensions of graphics screen */
  ydim = GetSystemMetrics(SM_CYSCREEN);

}

void restart_windows(void)
{ MSG           msg;
/* We need to clear the message buffer of Windows. What remains in the  */
/* message buffer is the key up messages from the reboot sequence and a */
/* paint message.                                                       */

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void restore_windows(void)
{ MSG           msg;
/* We need to clear the message buffer of Windows. What remains in the  */
/* message buffer is the key up messages from the reboot sequence and a */
/* paint message.                                                       */

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void poll_windows(void)
{ MSG           msg;

   /* Now service Microsoft Windows messages */
  bExitMessageLoop = FALSE;
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
      GetMessage(&msg, NULL, 0, 0);
      if (IsWindow(hDbgDialog) || !IsDialogMessage(hDbgDialog, &msg))
      {
          TranslateMessage(&msg);

          DispatchMessage(&msg);
      }

      if (bExitMessageLoop)
         break;
  }

}


/***************************************************************************/
/*                                                                         */
/*  getcmdline                                                             */
/*                                                                         */
/*  Parameters :-                                                          */
/*       lpszCmdLine - The command line Windows gives to an application.   */
/*       argc        - Used to return the number of arguments in the       */
/*                     command line.                                       */
/*       argv        - Used to hold all the different arguments on the     */
/*                     command line.                                       */
/*                                                                         */
/*  Function :-                                                            */
/*       The function extracts the various command line arguments from the */
/*       command line string passed to the I/O server by Microsoft Windows */
/*                                                                         */
/*  Returns :-                                                             */
/*       The command line arguments in argv.                               */
/*       The number of command line arguments in argc.                     */
/***************************************************************************/

void getcmdline(lpszCmdLine, argc, argv)
  LPSTR          lpszCmdLine;
  int            *argc;
  char           **argv;
{
  int  count = 0;
  char temp[80];

  argv[0] = "winsrvr";
  *argc = 1;
  lpszCmdLine--;
  do  {
    lpszCmdLine++;
    if (*lpszCmdLine == '"') {    /* a string name as part of the command line */
      count = 0;
      do {
        temp[count] = *lpszCmdLine;  /* store the name in temp */
        lpszCmdLine++;
        count++;
      }
      while (*lpszCmdLine != '"');
    }
       /* Have we reached the end of an argument on the cmd line? */
    if ((*lpszCmdLine == ' ') || ((*lpszCmdLine == NULL) && (count > 0))) {
      temp[count] = '\0';
      argv[*argc] = malloc(count + 1);
      strcpy(argv[*argc], temp);
      (*argc)++;
      count = 0;
    }
    else {
      temp[count] = *lpszCmdLine;   /* store part of argument in temp */
      count++;
    }
   }
  while (*lpszCmdLine != NULL);
}

/***************************************************************************/
/*                                                                         */
/*  Function : Check_Current_Boxes                                         */
/*                                                                         */
/*  The function will place a check in the check boxes of the dialog box   */
/*  used for selecting debug options. The function checks to see what the  */
/*  current debug settings are, and places the check mark in the relevant  */
/*  box.                                                                   */
/*                                                                         */
/***************************************************************************/

PRIVATE void Check_Current_Boxes(hDlg)
   HWND hDlg;
{
   int ct;

   show_logger_name(hDlg);  /* displays the current logfile name */

   if (log_dest eq Log_to_both) {
      CheckDlgButton(hDlg, IDD_SCREEN, 1);  /* set current logfile options */
      CheckDlgButton(hDlg, IDD_TOFILE, 1);
   }
   else {
     if (log_dest eq Log_to_file)
       CheckDlgButton(hDlg, IDD_TOFILE, 1);
     else
       CheckDlgButton(hDlg, IDD_SCREEN, 1);
   }

	/* go through check boxes and set those flags selected */
#define check_btn(a,b) if (debugflags & a) CheckDlgButton(hDlg, b, 1)
    check_btn(Boot_Flag,	IDD_BOOT);
    check_btn(Com_Flag,		IDD_COMS);
    check_btn(Delete_Flag,	IDD_DEL);
    check_btn(Error_Flag,	IDD_ERR);
    check_btn(FileIO_Flag,	IDD_FILE);
    check_btn(Graphics_Flag,	IDD_GRAPHICS);
    check_btn(HardDisk_Flag,	IDD_RAWD);
    check_btn(Init_Flag,	IDD_INIT);
    check_btn(Directory_Flag,	IDD_DIR);
    check_btn(Keyboard_Flag,	IDD_KEYB);
    check_btn(Message_Flag,	IDD_MESS);
    check_btn(Name_Flag,	IDD_NAME);
    check_btn(Open_Flag,	IDD_OPEN);
    check_btn(Close_Flag,	IDD_CLOSE);
    check_btn(Quit_Flag,	IDD_EXIT);
    check_btn(Read_Flag,	IDD_READ);
    check_btn(Search_Flag,	IDD_SRCH);
    check_btn(Timeout_Flag,	IDD_TIME);
    check_btn(OpenReply_Flag,	IDD_OPRPLY);
    check_btn(Write_Flag,	IDD_WRITE);
    check_btn(DDE_Flag,		IDD_DDE);
#undef check_btn
}


/**************************************************************************/
/*                                                                        */
/*   Function : Update_Debug_Options                                      */
/*                                                                        */
/*   The function captures the user input on the dialog box used for      */
/*   changing debug settings in the server, and updates the dialog box    */
/*   display to reflect the users input.                                  */
/*                                                                        */
/**************************************************************************/

PRIVATE void Update_Debug_Options(hDlg)
   HWND hDlg;
{
   int ct, old_log = log_dest;

   set_logger_name(hDlg);   /* changes the logfile name if necessary */

   if ((IsDlgButtonChecked(hDlg, IDD_TOFILE)) &&
       (IsDlgButtonChecked(hDlg, IDD_SCREEN)))
     log_dest = Log_to_both;               /* has the log destination */
   else {                                  /* changed?                */
     if (IsDlgButtonChecked(hDlg, IDD_TOFILE)) log_dest = Log_to_file;
        else
     log_dest = Log_to_screen;
   }

/* Check all the check boxes and if they have been checked, set the */
/* corresponding debug flag */
#define check_btn(a,b) \
   if (IsDlgButtonChecked(hDlg, b)) \
    debugflags |= a; \
   else \
    debugflags &= ~a;

    check_btn(Boot_Flag,	IDD_BOOT);
    check_btn(Com_Flag,		IDD_COMS);
    check_btn(Delete_Flag,	IDD_DEL);
    check_btn(Error_Flag,	IDD_ERR);
    check_btn(FileIO_Flag,	IDD_FILE);
    check_btn(Graphics_Flag,	IDD_GRAPHICS);
    check_btn(HardDisk_Flag,	IDD_RAWD);
    check_btn(Init_Flag,	IDD_INIT);
    check_btn(Directory_Flag,	IDD_DIR);
    check_btn(Keyboard_Flag,	IDD_KEYB);
    check_btn(Message_Flag,	IDD_MESS);
    check_btn(Name_Flag,	IDD_NAME);
    check_btn(Open_Flag,	IDD_OPEN);
    check_btn(Close_Flag,	IDD_CLOSE);
    check_btn(Quit_Flag,	IDD_EXIT);
    check_btn(Read_Flag,	IDD_READ);
    check_btn(Search_Flag,	IDD_SRCH);
    check_btn(Timeout_Flag,	IDD_TIME);
    check_btn(OpenReply_Flag,	IDD_OPRPLY);
    check_btn(Write_Flag,	IDD_WRITE);
    check_btn(DDE_Flag,		IDD_DDE);

    check_btn(Memory_Flag,	IDD_MEMORY);
    check_btn(Reconfigure_Flag,	IDD_RECON);
#undef check_btn
}


/* WndProc is the standard Windows Procedure, associated with the main   */
/* server window, called by Microsoft Windows, when passing messages     */
/* pertaining to the main server window.                                 */

long FAR PASCAL WndProc( hWnd, iMessage, wParam, lParam)
  HWND         hWnd;
  unsigned     iMessage;
  WORD         wParam;
  LONG         lParam;
{
  PAINTSTRUCT ps;    /* used during paint messages */
  int         caption_height,   /* Height of the caption in pixels */
              screen_height,    /* Height of the screen in pixels */
              screen_width;     /* width of the screen in pixels */
  HDC         hDC;
  TEXTMETRIC  tm;
  HFONT       hFont;
  FARPROC     lpfnAboutProc;
  short       i;

  switch (iMessage) {
     case WM_TIDYDIALOG:
        if (lpfnDlgBox != NULL)
            FreeProcInstance(lpfnDlgBox);
        if (lpfnAboutBox != NULL)
            FreeProcInstance(lpfnAboutBox);
        break;

     case WM_CREATE:
        caption_height = GetSystemMetrics(SM_CYCAPTION);
        screen_height  = GetSystemMetrics(SM_CYSCREEN);
        screen_width   = GetSystemMetrics(SM_CXSCREEN);
        hFont          = GetStockObject(SYSTEM_FIXED_FONT);
        hDC = GetDC(hWnd);
        SelectObject(hDC, hFont);
        GetTextMetrics(hDC, &tm);
        ReleaseDC(hWnd, hDC);
   /* determine the max. no. of lines of text in a full screen window */
        CHAR_WIDTH  = tm.tmAveCharWidth;
        CHAR_HEIGHT = tm.tmHeight + tm.tmExternalLeading;
        MAXLINES = screen_height / CHAR_HEIGHT;
   /* determine the max. no. of chars per text line in a full screen window */
        MAXCOLS  = (screen_width) / CHAR_WIDTH;
        if (MAXCOLS > 255) MAXCOLS = 255;
        if (MAXCOLS < 81) MAXCOLS = 81;
        Caret_Offset = CHAR_HEIGHT - tm.tmInternalLeading;

        Server_window.handle = hWnd;
        Set_Up_Window(hWnd);
        SetErrorMode(SEM_NOOPENFILEERRORBOX);
        break;

     case WM_COMMAND:
        switch(wParam) {
            case IDM_QUIT    : Special_Exit = TRUE;
                               break;

            case IDM_REBOOT  : Special_Reboot = TRUE;
                               break;

            case IDM_STATUS  : Special_Status = TRUE;
                               break;

            case IDM_DBGSET  :
                  if (!IsWindow(hDbgDialog))
                  {
                      lpfnDlgBox = MakeProcInstance(DbgBoxProc, PROGINSTANCE);
                      hDbgDialog = CreateDialog(PROGINSTANCE, "DbgBox", NULL, lpfnDlgBox);
                      ShowWindow(hDbgDialog, SW_SHOWNORMAL);
                      UpdateWindow(hDbgDialog);
                  }
                  else
                      SetFocus(hDbgDialog);
                  break;

            case IDM_SETALL :
                  debugflags = All_Debug_Flags;
                  break;

            case IDM_RESETALL :
                  debugflags = 0L;
                  break;

            case IDM_ABOUT   :
                  if (!IsWindow(hAboutDialog))
                  {
                      lpfnAboutBox = MakeProcInstance(AboutDlgProc, PROGINSTANCE);
                      hAboutDialog = CreateDialog(PROGINSTANCE, "AboutBox", NULL, lpfnAboutBox);
                      ShowWindow(hAboutDialog, SW_SHOWNORMAL);
                      UpdateWindow(hAboutDialog);
                  }
                  else
                      SetFocus(hAboutDialog);
                  break;
        }
        break;

     case WM_PAINT:
       BeginPaint(hWnd, &ps);
       if (!(IsIconic(hWnd)))
         Repaint_Screen(hWnd, ps.rcPaint.top / CHAR_HEIGHT,
                              (ps.rcPaint.bottom / CHAR_HEIGHT) + 1,
                              ps.rcPaint.left / CHAR_WIDTH,
                              (ps.rcPaint.right / CHAR_WIDTH) + 1);
       EndPaint(hWnd, &ps);
       break;

     case WM_SIZE:
        {
        extern BOOL in_create;
        if ((wParam != SIZEICONIC) || (in_create))
         Set_Window_Size(hWnd);
        }
        break;


/* For keys pressed whem the main window is active, only interpret  */
/* non character keys, eg. function keys, alt, etc */

     case WM_KEYDOWN:
        if (!(non_char_keys(hWnd, wParam, lParam)))
           return(DefWindowProc(hWnd, iMessage, wParam, lParam));
        else
           break;

     case WM_SYSKEYDOWN:
        if (!(non_char_keys(hWnd,wParam,lParam)))
           return(DefWindowProc(hWnd, iMessage, wParam, lParam));
        else
           break;

     case WM_DESTROY:
       PostQuitMessage(0);
       break;

     case WM_CHAR:
        for (i = 0; i < (short) LOWORD(lParam); i++)
           process_keyboard(hWnd, wParam, lParam);
        break;

        /* Handling of alt-letter input					*/
     case WM_SYSCHAR:
	if ((wParam >= 'a') && (wParam <= 'z') && (lParam & 0x20000000))
	 if ((wParam != 'f') && (wParam != 'd') && (wParam != 'h'))
          return(0L);
      return DefWindowProc(hWnd, iMessage, wParam, lParam);
        
     case WM_SETFOCUS:
        create_a_caret(hWnd);   /* displays a cursor in the graphics screen */
        break;

     case WM_KILLFOCUS:
        delete_a_caret(hWnd);
        break;

    default:
      return DefWindowProc(hWnd, iMessage, wParam, lParam);
  }
  return 0l;
}


/* PopupProc is the standard Windows Procedure, associated with the console */
/* and shell windows, called by Microsoft Windows when passing messages     */
/* pertaining to the console or shell windows.                              */


long FAR PASCAL PopupProc( hWnd, iMessage, wParam, lParam)
  HWND         hWnd;
  unsigned     iMessage;
  WORD         wParam;
  LONG         lParam;
{
   PAINTSTRUCT ps;
   short       i;
   HFONT       hFont;
   HDC         hDC;

   switch (iMessage) {
     case WM_CREATE:
        hFont = GetStockObject(SYSTEM_FIXED_FONT);
        hDC   = GetDC(hWnd);
        SelectObject(hDC, hFont);
        ReleaseDC(hWnd, hDC);
        Set_Up_Window(hWnd);   /* See mswindow.c */
        break;

     case WM_COMMAND:
        switch(wParam) {
            case IDM_QUIT    : Special_Exit = TRUE;
                               break;

            case IDM_REBOOT  : Special_Reboot = TRUE;
                               break;

            case IDM_STATUS  : Special_Status = TRUE;
                               break;

            case IDM_DBGSET  :
                  if (!IsWindow(hDbgDialog))
                  {
                      lpfnDlgBox = MakeProcInstance(DbgBoxProc, PROGINSTANCE);
                      hDbgDialog = CreateDialog(PROGINSTANCE, "DbgBox", NULL, lpfnDlgBox);
                      ShowWindow(hDbgDialog, SW_SHOWNORMAL);
                      UpdateWindow(hDbgDialog);
                  }
                  else
                      SetFocus(hDbgDialog);
                  break;

            case IDM_SETALL   :
                  debugflags = (0xFFFFFFFFL & ~(Memory_Flag + Log_Flag));
                  break;

            case IDM_RESETALL :
                  debugflags = 0L;
                  break;

            default:
                  NotifyMenu(wParam);
                  break;
        }



     case WM_PAINT:
        BeginPaint(hWnd, &ps);
        if (!(IsIconic(hWnd)))        /* see mswindow.c */
          Repaint_Screen(hWnd, ps.rcPaint.top / CHAR_HEIGHT,
                               (ps.rcPaint.bottom / CHAR_HEIGHT) + 1,
                               ps.rcPaint.left / CHAR_WIDTH,
                               (ps.rcPaint.right / CHAR_WIDTH) + 1);
        EndPaint(hWnd, &ps);
        break;

     case WM_SIZE:
       if (wParam != SIZEICONIC)
         Set_Window_Size(hWnd);    /* see mswindow.c */
       break;

/* trap all keys hit in the console or shell windows. See mswindow.c */
     case WM_KEYDOWN:
        if (!(non_char_keys(hWnd, wParam, lParam)))
           return(DefWindowProc(hWnd, iMessage, wParam, lParam));
        else
           break;

     case WM_SYSKEYDOWN:
        if (!(non_char_keys(hWnd,wParam,lParam)))
           return(DefWindowProc(hWnd, iMessage, wParam, lParam));
        else
           break;

     case WM_CHAR:
        for (i = 0; i < (short) LOWORD(lParam); i++)
           process_keyboard(hWnd, wParam, lParam);
        break;

        /* Handling of alt-letter input					*/
     case WM_SYSCHAR:
	if ((wParam >= 'a') && (wParam <= 'z') && (lParam & 0x20000000))
	 if ((wParam != 'f') && (wParam != 'd') && (wParam != 'h'))
          return(0L);
      return DefWindowProc(hWnd, iMessage, wParam, lParam);
        
     case WM_SETFOCUS:
        create_a_caret(hWnd);   /* displays a cursor in the graphics screen */
        break;

     case WM_KILLFOCUS:
        delete_a_caret(hWnd);
        break;

     default:
       return DefWindowProc(hWnd, iMessage, wParam, lParam);
   }
   return (0l);
}

void DrawFocus(HDC hDC, int x, int y, int cx, int cy, BOOL bFocus)
{
   RECT rect;
   rect.left = x;
   rect.top = y;
   rect.right = cx;
   rect.bottom = cy;
   InvertRect(hDC, &rect);
   if (bFocus)
        DrawFocusRect(hDC, &rect);
}

/*************************************************************************/
/*                                                                       */
/*   Function : AboutDlgProc                                             */
/*                                                                       */
/*   The function handles the servers about message box.                 */
/*                                                                       */
/*************************************************************************/


BOOL FAR PASCAL AboutDlgProc(hDlg, iMessage, wParam, lParam)
    HWND     hDlg;
    unsigned iMessage;
    WORD     wParam;
    LONG     lParam;
{
    switch(iMessage) {
        case WM_INITDIALOG:
             SetDlgItemText(hDlg, IDD_VERSION, SccsId1);
             break;

        case WM_COMMAND:
             switch(wParam) {
                case IDOK :
                      DestroyWindow(hDlg);
                      hAboutDialog = NULL;

                      PostMessage(LOWORD(Server_window.handle), WM_TIDYDIALOG, 0, 0);
                      break;

                default   : return FALSE;
             }
             break;

        default : return FALSE;
    }
    return TRUE;
}


/***********************************************************************/
/*                                                                     */
/*   Function : DbgBoxProc                                             */
/*                                                                     */
/*   A standard Windows dialog function to handle the dialog box used  */
/*   for modifying the debug settings of the server.                   */
/*                                                                     */
/***********************************************************************/


BOOL FAR PASCAL DbgBoxProc(hDlg, iMessage, wParam, lParam)
    HWND     hDlg;
    unsigned iMessage;
    WORD     wParam;
    LONG     lParam;
{
    switch(iMessage) {
        case WM_INITDIALOG:
             SetDlgItemText(hDlg, IDD_FNAME, "   ");
             Check_Current_Boxes(hDlg);
             break;

        case WM_COMMAND:
             switch(wParam) {
                case IDOK :
                      Update_Debug_Options(hDlg);
                      /* fall through */

                case IDCANCEL :
                      DestroyWindow(hDlg);
                      hDbgDialog = NULL;

                      PostMessage(LOWORD(Server_window.handle), WM_TIDYDIALOG, 0, 0);
                      break;

                case  IDD_BOOT    :
                case  IDD_COMS    :
                case  IDD_DEL     :
                case  IDD_ERR     :
                case  IDD_FILE    :
                case  IDD_GRAPHICS:
                case  IDD_RAWD    :
                case  IDD_INIT    :
                case  IDD_DIR     :
                case  IDD_KEYB    :
                case  IDD_MESS    :
                case  IDD_NAME    :
                case  IDD_OPEN    :
                case  IDD_CLOSE   :
                case  IDD_EXIT    :
                case  IDD_READ    :
                case  IDD_SRCH    :
                case  IDD_TIME    :
                case  IDD_OPRPLY  :
                case  IDD_WRITE   :
                case  IDD_MEMORY  :
                case  IDD_RECON   :
                case  IDD_DDE     :
                case  IDD_SCREEN  :
                case  IDD_TOFILE  :

                case IDD_FNAME    :
                         break;

                default   : return FALSE;
             }
             break;

        default : return FALSE;
    }
    return TRUE;
}


#if use_own_memory_management
void initialise_memory()
{       /* Do not allow the default data segment to move !!! */
  LockData(0);
}

char *get_mem(uint size)
{ HANDLE        handle  = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size + 4);
  char          *result;

  if (handle eq 0) return(NULL);
  result = GlobalLock(handle);
  *((HANDLE *) result) = handle;
  return(&(result[4]));
}

void free_mem(char *a_ptr)
{ HANDLE        handle;
  a_ptr = a_ptr - 4;
  handle = *((HANDLE *) a_ptr);
  GlobalUnlock(handle);
  GlobalFree(handle);
}

void memory_map()
{
}

#endif
