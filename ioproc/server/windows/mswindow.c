/*****************************************************************************/
/*****************************************************************************/
/**           Helios I/O Server Under Microsoft Windows                     **/
/**                                                                         **/
/**                 Copyright (c) 1990,   CSIR - MIKOMTEK                   **/
/**                     All Rights Reserved.                                **/
/**                                                                         **/
/**   mswindow.c                                                            **/
/**                                                                         **/
/**   This file contains the code to handle input from a window, via the    **/
/**   the keyboard, and output to a window. The file also contains code to  **/
/**   handle various Windows requests pertaining to an open Server window.  **/
/**                                                                         **/
/**   Author : S.A. Wilson               05/90                              **/
/**   Site   : CSIR - MIKOMTEK                                              **/
/**                                                                         **/
/*****************************************************************************/
/*****************************************************************************/


#include "helios.h"

/**
*** The interface to the window system, called by terminal.c
**/
        /* These variables exist within the ANSI emulator code */
extern int    Cur_x, Cur_y, LastRow, LastCol, Screen_mode;
extern byte   **map;
extern Window *current_window;
extern Screen *current_screen;
extern word   handle;

        /* Determining window sizes in the correct order is nasty.      */
        /* It relies on window_size() being called immediately after    */
        /* create_a_window().                                           */
bool    in_create;
PRIVATE int     x_size, y_size;

PRIVATE word get_config_size(char *name, char *item)
{
    char str[50];
    char *pos;

    strcpy(str, name);
    while ((pos=strchr(str, ' ')) ne NULL)
        *pos = '_';

    strcat(str, "_");
    strcat(str, item);

    return get_int_config(str);
}

PRIVATE int get_config_show(char *name)
{
    char str[50], *pos;
    char *show_type;
    bool servwin = !strcmp(name, "Helios Server");

    strcpy(str, name);
    while ((pos=strchr(str, ' ')) ne NULL)
        *pos = '_';

    strcat(str, "_show");
    show_type = get_config(str);
    if (show_type == NULL)
    {
        if (servwin)
            return global_nCmdShow;
        else
            return SW_SHOWNORMAL;
    }
    else if (!strcmp(show_type, "minimize"))
        return SW_SHOWMINIMIZED;
    else if (!strcmp(show_type, "maximize"))
        return SW_SHOWMAXIMIZED;
    else if (!strcmp(show_type, "noactivate"))
        return SW_SHOWNA;
    else if (!strcmp(show_type, "minnoactive"))
        return SW_SHOWMINNOACTIVE;
    else
        return SW_SHOWNORMAL;
}

word create_a_window(char *name)
{ int   xdim, ydim;
  int x, y, width, height;
  int   default_width, default_height, default_x, default_y;
  BOOL  bValidEntry = FALSE;
  HWND  hWnd;
  HDC   hDC;
  bool  servwin = !strcmp(name, "Helios Server");
  char  window_default[] = "window_default";

  xdim = GetSystemMetrics(SM_CXSCREEN);
  ydim = GetSystemMetrics(SM_CYSCREEN);

  /* first we find out if a size entry exists in the host.con file */
  if (!servwin)
  {
      default_x = get_config_size(window_default, "x");
      if (default_x < 0)
           default_x = 0;

      default_y = get_config_size(window_default, "y");
      if (default_y < 0)
           default_y = ydim / 4;

      default_width = get_config_size(window_default, "width");
      if (default_width < 1)
            default_width = xdim;
      else
      {
            default_width *= CHAR_WIDTH;
            default_width += GetSystemMetrics(SM_CXFRAME)*2;
      }

      default_height = get_config_size(window_default, "height");
      if (default_height < 1)
            default_height = ((ydim / 3) * 2);
      else
      {
            default_height *= CHAR_HEIGHT;
            default_height += GetSystemMetrics(SM_CYFRAME)*2 + GetSystemMetrics(SM_CYMENU)
                            + GetSystemMetrics(SM_CYCAPTION);
      }

      x = get_config_size(name, "x");
      if (x < 0)
           x = default_x;
      else
           bValidEntry = TRUE;

      y = get_config_size(name, "y");
      if (y < 0)
           y = default_y;
      else
           bValidEntry = TRUE;

      width = get_config_size(name, "width");
      if (width < 1)
           width = default_width;
      else
      {
           bValidEntry = TRUE;
           width *= CHAR_WIDTH;
           width += GetSystemMetrics(SM_CXFRAME)*2;
      }

      height = get_config_size(name, "height");
      if (height < 1)
           height = default_height;
      else
      {
           bValidEntry = TRUE;
           height *= CHAR_HEIGHT;
           height += GetSystemMetrics(SM_CYFRAME)*2 + GetSystemMetrics(SM_CYMENU)
                    + GetSystemMetrics(SM_CYCAPTION);
      }

      if (!bValidEntry)
      {
           if (get_config("cascade_windows") ne NULL)
           {
               x = CW_USEDEFAULT;
               y = CW_USEDEFAULT;
               width = default_width;
               height = default_height;
           }
      }
  }
  else
  {
       x = 0;
       y = 0;
       width = xdim;
       height = ydim;
  }

  hWnd = CreateWindow(servwin ? szAppName : szPopupClass,
        name,
        WS_OVERLAPPEDWINDOW,
        x,
        y,
        width,
        height,
        NULL, NULL, PROGINSTANCE, NULL);

  if (hWnd eq 0) return(NULL);

  /* set up show type */
  in_create = TRUE;
  ShowWindow(hWnd, get_config_show(name));

  UpdateWindow(hWnd);
  in_create = FALSE;

  hDC = GetDC(hWnd);
  SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
  SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
  ReleaseDC(hWnd, hDC);

        /* Set up a timer for the server window, to ensure that the     */
        /* I/O Server keeps waking up even if other applications are    */
        /* running. This is slightly optimistic, but better than        */
        /* nothing.                                                     */
  if (servwin)
  {
   TIMERPROC lpfnTimerProc = (TIMERPROC)MakeProcInstance(TimerProc, PROGINSTANCE);
   SetTimer(hWnd, 1, 50000, lpfnTimerProc);
  }

  return((word) hWnd);
}

void close_window(word window)
{
  DestroyWindow(LOWORD(window));
}

        /* Keyboard input is handled by explicit calls to add_key       */
        /* from the Windows callback functions, not by polling.         */
int read_char_from_keyboard(word window)
{ return(-1);
}

void window_size(word window, word *x, word *y)
{
        /* In the Windows I/O Server, this routine is only called       */
        /* immediately after a window is created.                       */
  *x = x_size;
  *y = y_size;
}

        /* These routines are callbacks invoked indirectly by Windows,  */
        /* via routines in winsrvr.c. They have to cope with resizing,  */
        /* initial pop-up, refresh, etc.                                */

        /* This routine is called indirectly by Windows during a call   */
        /* to CreateWindow().                                           */
void Set_Up_Window(HANDLE hWnd)
{ hWnd = hWnd;
}

        /* This routine is called when a window is resized, and when    */
        /* a window first pops up.                                      */
void Set_Window_Size(HANDLE hWnd)
{ RECT  rect;
  int   new_x, new_y;

  GetClientRect(hWnd, &rect);
  new_x = rect.right    / CHAR_WIDTH;
  new_y = rect.bottom   / CHAR_HEIGHT;
  if (new_y < 1) new_y = 1;
  if (new_x < 1) new_x = 1;

  if (in_create)        /* the window has just popped up */
   { if (IsIconic(hWnd))
      { /* this can only be the server window, so recalc the size */
        x_size = (GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXFRAME)*2) / CHAR_WIDTH;
        y_size = (GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYFRAME)*2
                        - GetSystemMetrics(SM_CYCAPTION)
                        - GetSystemMetrics(SM_CYMENU)) / CHAR_HEIGHT;
      }
     else
      { x_size = new_x;
        y_size = new_y;
      }
   }
  else
   { Window     *window = find_window(hWnd);
     if (window ne NULL)
      Resize_Ansi(window, window->handle, new_y, new_x);
   }
}

        /* This routine may be called as a result of UpdateWindow()     */
        /* or because something really has to be done.                  */
void Repaint_Screen(HWND hWnd, int top, int bott, int left, int right)
{ Window        *window;
  byte         **map;
  HDC            hDC;

       /* If this is a newly-created window, do nothing. */
  if (in_create) return;

  window = find_window(hWnd);

    /* Conceivably the repaint may come before the window has been put into */
    /* the list and the screen map set up.                                  */
  if (window eq NULL) return;

    /* Extract the screen map, and redraw all.                              */
  map = window->screen.map;

    /* Clip the dimensions, just in case */
  if (top < 0) top = 0;
  if (left < 0) left = 0;
  if (bott < top) return;
  if (right < left) return;
  if (bott > window->screen.Rows) bott = window->screen.Rows;
  if (right > window->screen.Cols) right = window->screen.Cols;

  hDC = GetDC(hWnd);
  for ( ; top < bott; top++)
   TextOut(hDC, left * CHAR_WIDTH, top * CHAR_HEIGHT,
             &(map[top][left]), right - left);
  ReleaseDC(hWnd, hDC);
}

/**
*** These routines are invoked by the ANSI emulator. Note that the windows
*** always use a fixed-size font.
***
***   1) clear_screen() and clear_eol() are straightforward, they can use the
***      rectangular operations to blat part of the screen
***   2) there is a Windows ScrollWindow() routine for linefeeds
***   3) refresh and redraw are just loops of text drawing, one line at a time.
***   4) ringing the bell, fiddling with modes and colours, etc. is trivial
***      (actually, mostly unimplemented)
*** The tricky bits involve text output. There are two problems. First,
*** it is necessary to reset the caret (text cursor) when output has finished.
*** This can be done easily in flush(), guaranteed to be called at the end of
*** any text output. Second, characters should be buffered so that the
*** graphics routines are not called for each and every character.
**/
static char text_buf[128];
static int  buf_contents = 0;
static int  start_x, start_y;
static void flush_textbuf(void);

void clear_screen(void)
{ RECT         rect;
  HDC          hDC;
  HBRUSH       hBrush;
  register int i, j;

  hide_caret(LOWORD(handle));
  buf_contents = 0;     /* discard any buffered data */

  for (i = 0; i <= LastRow; i++)
   for (j = 0; j <= LastCol; j++)
    map[i][j] = ' ';
  Cur_x = 0; Cur_y = 0;

  SetRect(&rect, 0, 0, current_screen->Cols * CHAR_WIDTH,
                current_screen->Rows * CHAR_HEIGHT);
  hDC = GetDC(LOWORD(handle));
  hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  FillRect(hDC, &rect, hBrush);
  DeleteObject(hBrush);
  ReleaseDC(LOWORD(handle), hDC);
}

void clear_eol(void)
{ HDC    hDC;
  HBRUSH hBrush;
  RECT   rect;

  hide_caret(LOWORD(handle));
  if (buf_contents > 0) flush_textbuf();

  hDC = GetDC(LOWORD(handle));
  SetRect(&rect, Cur_x * CHAR_WIDTH, Cur_y * CHAR_HEIGHT,
                 current_screen->Cols * CHAR_WIDTH, (Cur_y + 1) * CHAR_HEIGHT);
  hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  FillRect(hDC, &rect, hBrush);
  DeleteObject(hBrush);
  ReleaseDC(LOWORD(handle), hDC);
}

void linefeed(void)
{ register int  i;
  register BYTE *ptr;

  if (buf_contents > 0) flush_textbuf();

  if (Cur_y >= LastRow)
   {
     hide_caret(LOWORD(handle));
     ScrollWindow(LOWORD(handle), 0, -1 * CHAR_HEIGHT, NULL, NULL);
     ptr = map[0];
     for (i = 0; i < LastRow; i++)
      map[i] = map[i+1];
     for (i = 0; i <= LastCol; i++)
      ptr[i] = ' ';
     map[LastRow] = ptr;
     Cur_y = LastRow;
   }
  else
   Cur_y++;
}

void refresh_screen(int start_y, int start_x)
{ int   i;
  HDC   hDC;

  hide_caret(LOWORD(handle));
  if (buf_contents > 0) flush_textbuf();

  hDC = GetDC(LOWORD(handle));
  for (i = start_y; i <= LastRow; i++)
   TextOut(hDC, 0, i * CHAR_HEIGHT, map[i], current_screen->Cols);
  ReleaseDC(LOWORD(handle), hDC);
}

void redraw_screen(Window *window, word han)
{ HDC   hDC;
  int   i;
  int   cols = window->screen.Cols;
  int   rows = window->screen.Rows;
  byte **map = window->screen.map;

  hide_caret(LOWORD(handle));
  hDC = GetDC(LOWORD(handle));
  for (i = 0; i < rows; i++)
   TextOut(hDC, 0, i * CHAR_HEIGHT, map[i], cols);
  ReleaseDC(LOWORD(handle), hDC);
  show_caret(LOWORD(handle), window->screen.Cur_y, window->screen.Cur_x);
}

void ring_bell(void)
{ MessageBeep(0);
}

void set_inverse(int flag)
{ HDC   hDC;

  if (buf_contents > 0) flush_textbuf();

  hDC = GetDC(LOWORD(handle));
  if (flag)
   { SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
     SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
   }
  else
   { SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
     SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
   }
  ReleaseDC(LOWORD(handle), hDC);
}

void set_bold(int flag)
{ flag = flag;
}

void set_underline(int flag)
{ flag = flag;
}

void set_italic(int flag)
{ flag = flag;
}

void foreground(int colour)
{ colour = colour;
}

void background(int colour)
{ colour = colour;
}

void backspace(void)
{
  if (buf_contents > 0) flush_textbuf();
  if (Cur_x > 0) Cur_x--;
}

void carriage_return(void)
{
  if (buf_contents > 0) flush_textbuf();
  Cur_x = 0;
}

void move_to(int y, int x)
{ if (buf_contents > 0) flush_textbuf();
  Cur_y = y;
  Cur_x = x;
}


PRIVATE int mark_x, mark_y;

void set_mark(void)
{ if (buf_contents > 0) flush_textbuf();
  mark_x = Cur_x; mark_y = Cur_y;
}

void use_mark(void)
{
  move_to(mark_y, mark_x);
}

void send_ch(int ch)
{
  if (buf_contents >= 128)
   flush_textbuf();
  if (buf_contents eq 0)
   { start_x = Cur_x; start_y = Cur_y; }
  text_buf[buf_contents++] = (char) ch;
}

void flush(void)
{
  hide_caret(LOWORD(handle));   /* Force a reposition of the caret */
  if (buf_contents > 0) flush_textbuf();
  show_caret(LOWORD(handle), Cur_y, Cur_x);
}

static void flush_textbuf(void)
{ HDC  hDC;

  if (buf_contents eq 0) return;
  hide_caret(LOWORD(handle));
  hDC = GetDC(LOWORD(handle));
  TextOut(hDC, start_x * CHAR_WIDTH, start_y * CHAR_HEIGHT, text_buf, buf_contents);
  buf_contents = 0;
  ReleaseDC(LOWORD(handle), hDC);
}
/**
*** The find_window() routine is used mainly from inside Callback functions.
*** Given a window handle hWnd, this routine finds the associated Window
*** pointer, if any.
**/
Window *find_window(HANDLE hWnd)
{ Window        *window;

  if (hWnd eq LOWORD(Server_window.handle)) return(&Server_window);

  for (window = (Window *) Window_List.list.head;
       (window->node.node.next ne NULL) && (hWnd ne LOWORD(window->handle));
       window = (Window *) window->node.node.next);

  if ((window->node.node.next eq NULL) || (hWnd ne LOWORD(window->handle)))
   return(NULL);
  else
   return(window);
}

/**
*** This code deals with caret handling, the text cursor in normal
*** terminology. Windows has a single caret for the entire display,
*** in other words only one window can receive keyboard input. There are
*** callback functions to indicate when keyboard focus changes, and
*** create_a_caret() and delete_a_caret() will be called. The Server keeps
*** track of which window currently has the caret, if any.
***
*** Windows specifies that the caret should be hidden whenever text is
*** drawn in the window holding the caret, and re-shown at the end of the
*** output. Also, the caret position has to be moved explicitly, it does
*** not follow text output.
***
*** These routines are intended to be idiot-proof, so there is no need
*** for the rest of the system to keep track of which window is active
*** and how many times a caret has been hidden and hence must be redisplayed.
**/
static HWND caret_window  = -1;
static int  caret_visible = 0;
static int caret_width = -1, caret_height = -1;

void create_a_caret(HWND hWnd)
{
  if (caret_width eq -1)
   { char *caret_type = get_config("caret_type");
     caret_width = CHAR_WIDTH;
     caret_height = 2;
     if (caret_type ne NULL)
      { if (!strcmp(caret_type, "box"))
         { caret_width = CHAR_WIDTH; caret_height = CHAR_HEIGHT; }
        if (!strcmp(caret_type, "vline"))
         { caret_width = 2; caret_height = CHAR_HEIGHT; }
      }
    }
   caret_window = hWnd;
   CreateCaret(hWnd, NULL, caret_width, caret_height);
   { Window *window = find_window(hWnd);
     if (window ne NULL)
      SetCaretPos(window->screen.Cur_x * CHAR_WIDTH,
                ((window->screen.Cur_y + 1) * CHAR_HEIGHT) - caret_height);
   }
   ShowCaret(hWnd);
   caret_visible = 1;
}

void delete_a_caret(HWND hWnd)
{
  caret_visible = 0;
  HideCaret(hWnd);
  caret_window = -1;
  DestroyCaret();
}

void show_caret(HWND hWnd, int y, int x)
{
  if ((hWnd ne caret_window) || (caret_visible > 0)) return;
  SetCaretPos(x * CHAR_WIDTH, ((y + 1) * CHAR_HEIGHT) - caret_height);
  caret_visible = 1;
  ShowCaret(hWnd);
}

void hide_caret(HWND hWnd)
{
  if ((hWnd ne caret_window) || (!caret_visible)) return;
  caret_visible = 0;
  HideCaret(hWnd);
}

/***************************************************************************/
/*  err_out                                                                */
/*                                                                         */
/*  Parameters :-                                                          */
/*       text - the text of the error message to be displayed              */
/*                                                                         */
/*  Function :-                                                            */
/*     The function displays an error message in a message box.            */
/*                                                                         */
/*  Returns :-                                                             */
/*    0.                                                                   */
/***************************************************************************/

int err_out(text,...)
  char *text;
{ char  buf[128]; char *end;
  va_list args;

  va_start(args, text);
  vsprintf(buf, text, args);
  va_end(args);
  end = buf + strlen(buf) - 1;
  while (iscntrl(*end)) end--;

  if (IsIconic(LOWORD(Server_window.handle)))
    ShowWindow(LOWORD(Server_window.handle), SW_SHOWNORMAL);
  MessageBox(LOWORD(Server_window.handle), buf, "I/O Server Error!",
             MB_ICONEXCLAMATION | MB_OK);
  return(0);
}


/***************************************************************************/
/*  switch_to_next                                                         */
/*                                                                         */
/*  Parameters :-                                                          */
/*       window - A pointer to a window structure.                         */
/*                                                                         */
/*  Function :-                                                            */
/*     The function activates the next window in the list of open windows. */
/*     If the last window in the list has been activated, then the routine */
/*     starts at the beginning of the list again.                          */
/*                                                                         */
/*  Returns :-                                                             */
/*    Nothing.                                                             */
/***************************************************************************/

PRIVATE void switch_to_next(window)
   Window *window;
{

   if (window->handle eq Server_window.handle)
       window = (Window *) Window_List.list.head;
   else
      window = (Window *) window->node.node.next;
   
   if (window->node.node.next eq (Window *) NULL) window = &(Server_window);
   
   SetActiveWindow(LOWORD(window->handle));
   ShowWindow(LOWORD(window->handle), SW_SHOWNORMAL);
}


/***************************************************************************/
/*  switch_to_previous                                                     */
/*                                                                         */
/*  Parameters :-                                                          */
/*       window - A pointer to a window structure.                         */
/*                                                                         */
/*  Function :-                                                            */
/*     The function activates the previous window in the list of open      */
/*     windows.                                                            */
/*                                                                         */
/*  Returns :-                                                             */
/*    Nothing.                                                             */
/***************************************************************************/

PRIVATE void switch_to_previous(window)
   Window *window;
{

   if (window->handle eq Server_window.handle)
       window = (Window *) Window_List.list.tail;
   else
      window = (Window *) window->node.node.prev;
   
   if (window->node.node.prev eq (Window *) NULL) window = &(Server_window);
   
   SetActiveWindow(LOWORD(window->handle));
   ShowWindow(LOWORD(window->handle), SW_SHOWNORMAL);
}


/***************************************************************************/
/*  get_key_status                                                         */
/*                                                                         */
/*  Parameters :-                                                          */
/*       status  - The status of the key pressed.                          */
/*       shift   - returns if the shift key is pressed                     */
/*       alt     - returns if the alt key is pressed.                      */
/*       control - returns if the control key is pressd                    */
/*                                                                         */
/*  Function :-                                                            */
/*     The function determines if the alt, shift, or control keys are down */
/*                                                                         */
/*  Returns :-                                                             */
/*    Nothing.                                                             */
/***************************************************************************/

void get_key_status(status, shift, alt, control)
  LONG   status;
  short  far *shift, far *alt, far *control;
{

  if (0x20000000 & status) *alt = 1;   /* alt key down? */
    else
        *alt = 0;

  *shift = GetKeyState(VK_SHIFT);      /* shift key down? */
  if (*shift < 0) *shift = 1;
    else
        *shift = 0;

  *control = GetKeyState(VK_CONTROL);  /* control key down? */
  if (*control < 0) *control = 1;
    else
       *control = 0;
}


/***************************************************************************/
/*  toggle_debug_flags                                                     */
/*                                                                         */
/*  Parameters :-                                                          */
/*       key - the character key pressed.                                  */
/*                                                                         */
/*  Function :-                                                            */
/*     The function will toggle the debug flag represented by key, if key  */
/*     is pressed in conjunction with control and shift keys.              */
/*                                                                         */
/*  Returns :-                                                             */
/*    1 if the a debugging flag was toggled, -1 if not.                    */
/***************************************************************************/

int toggle_debug_flags(key)
  int key;
{
  int flagchar = key + 'a' - 1;    /* convert to flag */
  int i;

  if (flagchar eq 'a')       /* set all debugging flags? */
   { if (debugflags eq 0L)
      debugflags = All_Debug_Flags;
     else
      debugflags = 0L;
     return(-1);
   }
  for (i = 0; options_list[i].flagchar ne '\0'; i++)
   if (options_list[i].flagchar eq flagchar)   /* set the specified flag */
    { debugflags ^= options_list[i].flag;
      return(1);
    }
   return(-1);     /* unimplemented debugging flag */
}


/***************************************************************************/
/*  handle_functions                                                       */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       key    - the function key pressed                                 */
/*       status - the status of the key pressed. (See Ms-Windows)          */
/*                                                                         */
/*  Function :-                                                            */
/*     The function will perform the required operation on a function key  */
/*     used in conjunction with control and shift keys, eg Special_Exit,   */
/*     or the function required if used with the ALT key, eg.              */
/*     switch_to_next. If none of the above functions is performed, then   */
/*     the key is added to the circular keyboard buffer, to be passed to   */
/*     the transputer.                                                     */
/*                                                                         */
/*  Returns :-                                                             */
/*    -1                                                                   */
/***************************************************************************/

int handle_functions(hWnd, key, status)
  HWND hWnd;
  int  key;
  LONG status;
{
  Window *window;
  short  shift, control, alt;

  get_key_status(status, &shift, &alt, &control);

  if (shift && control) {
    switch (key) {
#if debugger_incorporated
       case VK_F7  : DebugMode = 1 - DebugMode; break;
#endif
       case VK_F8  : Special_Status = true; break;
       case VK_F9  : Special_Exit   = true; break;
       case VK_F10 : Special_Reboot = true; break;
    }
    return(-1);
  }


  window = find_window(hWnd);
  if (window eq (Window *) NULL) return(-1);


  if (alt) {
    switch (key) {
        case VK_F1 : switch_to_next(window);
                     return(-1);
        case VK_F2 : switch_to_previous(window);
                     return(-1);
    }
  }


/* None of the above, so add key to circular buffer */

  add_key(0x009B, window);
  key -= 111;

  if (alt) {
    if (shift) {
        add_key('3', window);
        add_key('0' + key - 1, window);
    }
    else {
        add_key('2', window);
        add_key('0' + key - 1, window);
    }
  }
  elif (shift) {
    add_key('1', window);
    add_key('0' + key - 1, window);
  }
  else
     add_key('0' + key - 1, window);

  add_key('~', window);
  return(-1);
}

/***************************************************************************/
/*  handle_arrows                                                          */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       key    - the arrow key pressed.                                   */
/*                                                                         */
/*  Function :-                                                            */
/*     The function will inserts the transputer code for each arrow key in */
/*     the circular buffer of the window identified by hWnd.               */
/*                                                                         */
/*  Returns :-                                                             */
/*    -1                                                                   */
/***************************************************************************/

int handle_arrows(hWnd, key)
  HWND hWnd;
  int  key;
{
  Window *window;

  window = find_window(hWnd);
  if (window eq (Window *) NULL) return(-1);

  add_key(0x009b, window);

  switch (key) {
    case VK_UP    : add_key('A', window);
                    break;
    case VK_DOWN  : add_key('B', window);
                    break;
    case VK_LEFT  : add_key('D', window);
                    break;
    case VK_RIGHT : add_key('C', window);
                    break;
  }
  return(-1);
}

/***************************************************************************/
/*  other_keys                                                             */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       key    - the character code of the key pressed.                   */
/*                                                                         */
/*  Function :-                                                            */
/*     The function will insert  the transputer code for the key in the    */
/*     circular buffer of the window idefitified by hWnd.                  */
/*                                                                         */
/*  Returns :-                                                             */
/*    -1                                                                   */
/***************************************************************************/

int other_keys(hWnd, key)
  HWND hWnd;
  int  key;
{
  Window *window;

  window = find_window(hWnd);
  if (window eq (Window *) NULL) return(-1);


  if (key eq VK_DELETE) add_key(0x7F, window);
    else  {
       add_key(0x009B, window);
       switch (key) {
          case VK_INSERT : add_key('@', window);   /* insert key */
                           break;
          case VK_HOME   : add_key('H', window);   /* home key */
                           break;                  /* page up */
          case VK_PRIOR  : add_key('3', window); add_key('z', window);
                           break;                  /* end key */
          case VK_END    : add_key('2', window); add_key('z', window);
                           break;                  /* page down */
          case VK_NEXT   : add_key('4', window); add_key('z', window);
                           break;
        }
    }
  return (-1);
}

/***************************************************************************/
/*  alt-key support.							   */
/***************************************************************************/

static void handle_alt(HWND hWnd, int key, LONG status)
{
  Window	*window;

  window = find_window(hWnd);
  if (window eq (Window *) NULL) return;

  add_key(tolower(key) | 0x80, window);
  status = status;
}

/***************************************************************************/
/*  non_char_keys                                                          */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       key    - the character code of the key pressed.                   */
/*       status - the status of the key pressed. (See Ms-Windows)          */
/*                                                                         */
/*  Function :-                                                            */
/*     The function handles all non character keys, eg. function keys,     */
/*     arrow keys, etc, by calling the appropriate routines.               */
/*                                                                         */
/*  Returns :-                                                             */
/*    1 if the key is used by Helios, 0 if not.                            */
/***************************************************************************/

int  non_char_keys(hWnd, key, status)
  HWND hWnd;
  int  key;
  LONG status;
{
  int helios_key = 0;

    /* Alt-letter combinations are not normally passed to the application. */
    /* Instead Windows uses them for pull-down menus etc. The I/O Server   */
    /* typically has three pull-down menus: File, Debug and Help, but      */
    /* other alt combinations can still be interpreted. This means not     */
    /* going through DefWindowProc...                                      */
  if (0x20000000 & status)     /* alt key down? */
   if ((key >= 'A') && (key <= 'Z'))
    if ((key != 'F') && (key != 'D') && (key != 'H'))
     { handle_alt(hWnd, key, status);
       return(1);
     }
     
  switch(key) {
    case VK_F1     :
    case VK_F2     :
    case VK_F3     :
    case VK_F4     :
    case VK_F5     :
    case VK_F6     :
    case VK_F7     :
    case VK_F8     :
    case VK_F9     :
    case VK_F10    : handle_functions(hWnd, key, status);
                     helios_key = 1;
                     break;
    case VK_UP     :
    case VK_DOWN   :
    case VK_LEFT   :
    case VK_RIGHT  : handle_arrows(hWnd, key);
                     helios_key = 1;
                     break;
    case VK_INSERT :
    case VK_HOME   :
    case VK_PRIOR  :
    case VK_DELETE :
    case VK_END    :
    case VK_NEXT   : other_keys(hWnd, key);
                     helios_key = 1;
                     break;
  }
  return(helios_key);
}


/***************************************************************************/
/*  process_keyboard                                                       */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       key    - the character code of the key pressed.                   */
/*       status - the status of the key pressed. (See Ms-Windows)          */
/*                                                                         */
/*  Function :-                                                            */
/*     The function handles all characters keys pressed. If the key is     */
/*     used in conjunction with control and shift, then it might be to     */
/*     toggle a debugging flag, otherwise the key is added to the circular */
/*     keyboard buffer associated with the window identified by hWnd.      */
/*                                                                         */
/*  Returns :-                                                             */
/*    Nothing.                                                             */
/***************************************************************************/

void process_keyboard(hWnd, key, status)
  HANDLE hWnd;
  int    key;
  LONG   status;
{
  Window *window;
  short  shift, control, alt;

  window = find_window(hWnd);
  if (window eq (Window *) NULL) return;

  get_key_status(status, &shift, &alt, &control);
  if (control && shift)
   toggle_debug_flags(key);
  else
   add_key(key, window);
}


/***************************************************************************/
/*  add_to_key_buffer                                                      */
/*                                                                         */
/*  Parameters :-                                                          */
/*       hWnd   - A handle identifing a window.                            */
/*       text   - A string of text to be added to a keyboard buffer        */
/*                                                                         */
/*  Function :-                                                            */
/*     The function will insert the text defined by the parameter "text"   */
/*     into the keyboard buffer of the window identified by hWnd.          */
/*     This routine is used to handle opening and closing shells via a     */
/*     menu option.                                                        */
/*                                                                         */
/*  Returns :-                                                             */
/*    Nothing.                                                             */
/***************************************************************************/


void add_to_key_buffer(hWnd, text)
   HWND  hWnd;
   LPSTR text;
{
   Window *window;
   int    num_chars, ct;
   char   line[80];

   window = find_window(hWnd);
   if (window eq (Window *) NULL) {
     if (!strcmp(text, "exit"))
       strcpy(line, "Unable to close window - Non Existent Window");
     MessageBox(hWnd, line, NULL, MB_ICONSTOP | MB_OK);
     return;
   }

   if ((window->node.account eq 0) && (!strcmp(text, "exit"))) {
      delete_window(window);
      return;
   }

   if ((window->node.account > 1) && (!strcmp(text, "exit"))) {
      strcpy(line,
            "Unable to close window - Window has active applications!");
      MessageBox(hWnd, line, NULL, MB_ICONSTOP | MB_OK);
      return;
   }

   num_chars = strlen(text);
   for (ct = 0; ct < num_chars; ct++)
     add_key((int) text[ct], window);

   add_key((int) '\n', window);
}


