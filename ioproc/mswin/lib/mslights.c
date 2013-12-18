/**************************************************************
 * mslights.c                                                 *
 * Perihelion Software Ltd                                    *
 * Charlton Rd. Shepton Mallet, Somerset UK BA4 5QE           *
 *                                                            *
 * This source code is copyright free and may be adapted or   *
 * distributed without notice to the authors.                 *
 * Any improvements would be welcome by the authors.          *
 *                                                            *
 * This is a conversion of xlights.c which works in the X     *
 * windows environment, so that it runs in the MS-windows     *
 * I/O server environment (graphics environment v2.0).        *
 **************************************************************/
#ifdef __C40
#pragma no_stack_checks
#pragma fast_fp
#pragma little_data
#endif

#include <stdio.h>
#include <syslib.h>
#include <root.h>
#ifdef __TRAN
#include <process.h>
#else
#include <thread.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <queue.h>
#include <rmlib.h>
#include <nonansi.h>
#include <gsp.h>
#include <windows.h>

#include "mslights.h"

int	panel_xstart;
int	panel_xend;
int    	panel_height;
int    	panel_width;
int	bar_length;
int	bar_x;

int  	nprocs	= 0;			/* number of processors tracked */
int  	maxw	= 0;			/* maximum string size of processor labels */
int 	MaxProcLoad = 1000;		/* maximum processor load to date */
int 	MaxLinkLoad = 20000;		/* maximum link load to date */

HFONT	hFont;
HBRUSH	brGrey;
HPEN    penwhite;
HPEN    pengrey;
HPEN	penLoad;
HPEN	penLink;
HPEN	penMem;
HPEN	penBG;
HWND	hMainWnd;

word	grey       = RGB(128, 128, 128);
word	ltgrey     = RGB(200, 200, 200);
word    white	   = RGB(255, 255, 255);
word	crLoadBar  = RGB(255,   0,   0);
word	crLinkBar  = RGB(  0, 255,   0);
word	crMemBar   = RGB(  0,   0, 255);

Semaphore 	one_access;		/* exclusion semaphore to graphics device */
Proc_Info       *info = NULL;
static	byte	DataBuffer[IOCDataMax]; 

/* This routine return the top left position of the load display rectangle */
int    panel_y(int i) 
{ 
   return ((i)*( panel_height + border ) + border); 
}

/* This routine is called for every processor in the network.   */
static  void    NetworkCount(RmProcessor Processor, ...)
{
  if (RmGetProcessorPurpose(Processor) == RmP_Helios)
     nprocs++;
}


void draw_bars(int proc_offset)
{
   int l;
   int _new;
   int linkload = 0;
   NewProcStats  *NewInfo, *OldInfo;                	/* place for new stat info      */
   Proc_Info *ii;
   HDC  hDC = GetDC(hMainWnd);

   ii = &(info[proc_offset]);

   ServerInfo(ii->Proc, DataBuffer);			/* get processor stats */
   NewInfo = (NewProcStats *)DataBuffer;
   OldInfo = (NewProcStats *)ii->OldInfo;

   /* draw processor load */
   if (NewInfo->Load > MaxProcLoad)
      MaxProcLoad = (int) NewInfo->Load;
   _new = newbar((int)NewInfo->Load,MaxProcLoad);

   if( _new != ii->LoadBar )
   {
      bar(hDC, _new,ii->LoadBar, penLoad, bar_x, panel_y(proc_offset)+loadbar_y);
      ii->LoadBar = _new;
   }

   /* draw link load */
   for( l = 0; l < NewInfo->NLinks; l++ )
      linkload += (int)(NewInfo->Link[l].In - OldInfo->Link[l].In + NewInfo->Link[l].Out - OldInfo->Link[l].Out);
   linkload = linkload / (int) NewInfo->NLinks;	/* average load on all links */
   if (linkload > MaxLinkLoad) 
      MaxLinkLoad = linkload;
   _new = newbar( linkload, MaxLinkLoad);
   if( _new != ii->LinkBar )
   {
      bar(hDC, _new, ii->LinkBar, penLink, bar_x, panel_y(proc_offset)+linkbar_y);
      ii->LinkBar = _new;
   }

   /* draw memory load */   
   _new = newbar( (int)(NewInfo->MemMax-NewInfo->MemFree), (int)NewInfo->MemMax);
   if( _new != ii->MemBar )
   {
      bar(hDC, _new, ii->MemBar, penMem, bar_x, panel_y(proc_offset)+membar_y);
      ii->MemBar = _new;
   }
   *OldInfo = *NewInfo;

   ReleaseDC(hMainWnd, hDC);
}

void bar( HDC hDC, int _new, int old, HPEN hPen, int x, int y )
{
   if( old < 0)
   { /* restore old line as it may not be a complete line due to redraw event */
      old = -old;
      SelectObject(hDC, hPen);
      MoveTo(hDC, (int)(x), (int)y);
      LineTo(hDC, (int)(x+old), (int)y);
   } 

   if( _new > old )
   {  /* extend bar */
      SelectObject(hDC, hPen);
      MoveTo(hDC, x + old, y);
      LineTo(hDC, x + _new, y);
   }
   else
   {  /* shorten bar */
      SelectObject(hDC, penBG);
      MoveTo(hDC, (int)(x+_new), (int)y);
      LineTo(hDC, (int)(x+old), (int)y);
   }
}


/*{{{  newbar function */
int newbar( int value, int max )
{
   if ( value >= max )
      value = max - 1;

   if ( value == 0 ) 
      return 0;
		
   return (value * bar_length) / max;
}

/*}}}*/

/* This routine is called for every processor in the network.   */
static  int     NetworkWalk(RmProcessor Processor, ...)
{
   if (RmGetProcessorPurpose(Processor) == RmP_Helios)
   {  char *arg;
      arg = (char *)RmGetProcessorId(Processor);
      if(InitProcTable(nprocs, arg))
         nprocs++;
   }
   return 0;
}

Proc_Info* CreateProcTable( int no_processors)
{  Proc_Info * processor_info;
   processor_info = (Proc_Info *)malloc(sizeof(Proc_Info)*no_processors);
   if( processor_info == NULL )
     return NULL;
   memset(processor_info,0,sizeof(Proc_Info)*no_processors);
   return processor_info;
}

int InitProcTable(int position, char * proc_name)
{  int  w;
   char temp_name[100];

   w = strlen(proc_name);
   strcpy(temp_name, "/");
   strcat(temp_name, proc_name);
   strcat(temp_name, "/tasks");

   info[position].Proc = Locate(NULL,temp_name);
   info[position].Name = proc_name;

   if( info[position].Proc == NULL ) 
   {  fprintf(stderr,"mslights: failed to find %s\n\n",proc_name);
      return 0;
   }
   if( w > maxw ) 
      maxw = w;
   return 1;
}

void update_bars(void)
{
   int i;

   SetPriority(ServerPri);	/* ensure we keep the display active */ 

   for( i = 0; i < nprocs; i++ )
      ServerInfo(info[i].Proc, info[i].OldInfo);

   while (1)
   {  Delay(OneSec/4);
      Wait(&one_access);
      for( i = 0; i < nprocs; i++)
	 draw_bars(i);
      Signal(&one_access);
   }
}


long WINAPI MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
      case WM_CREATE:
      {
	 brGrey		= CreateSolidBrush(ltgrey);
	 penBG 		= CreatePen(PS_SOLID, bar_height, ltgrey);
         penwhite	= CreatePen(PS_SOLID, 1, white);
         pengrey	= CreatePen(PS_SOLID, 1, grey);
	 penLoad 	= CreatePen(PS_SOLID, bar_height, crLoadBar);
	 penLink 	= CreatePen(PS_SOLID, bar_height, crLinkBar);
	 penMem 	= CreatePen(PS_SOLID, bar_height, crMemBar);
         
	 break;
      }
      case WM_PAINT:
      {
	 RECT rect;
	 int  i;
	 PAINTSTRUCT ps;

	 BeginPaint(hWnd, &ps);
	 SelectObject(ps.hdc, hFont);
         SetBkMode(ps.hdc,TRANSPARENT);
	 
	 rect.left 	= (int)panel_xstart;
	 rect.right 	= (int)panel_xend;

	 Wait(&one_access);
	 for(i = 0; i < nprocs; i++ )
	 {
	    rect.top = (int)panel_y(i);
	    rect.bottom = (int)panel_y(i) + (int)panel_height;
	    FillRect( ps.hdc, &rect, GetStockObject(LTGRAY_BRUSH));

            SelectObject(ps.hdc,penwhite);
            MoveTo(ps.hdc, rect.left, rect.top);
            LineTo(ps.hdc, rect.right,rect.top);
            MoveTo(ps.hdc, rect.left, rect.top);
            LineTo(ps.hdc, rect.left,rect.bottom);
            SelectObject(ps.hdc,pengrey);
            MoveTo(ps.hdc, rect.left+1, rect.bottom);
            LineTo(ps.hdc, rect.right,rect.bottom);
            MoveTo(ps.hdc, rect.right, rect.top+1);
            LineTo(ps.hdc, rect.right,rect.bottom);

	    info[i].LoadBar    = -info[i].LoadBar;	/* invalidate the whole line */
	    info[i].LinkBar    = -info[i].LinkBar;
	    info[i].MemBar     = -info[i].MemBar;

	 }

	 for(i = 0; i < nprocs; i++ )
	 {
	    TextOut(ps.hdc, border, panel_y(i), info[i].Name, strlen(info[i].Name));
	 } 
	 EndPaint(hWnd, &ps);
	 Signal(&one_access);
	 break;
      }

      case WM_CHAR:
	 if (((char)wParam == 'q') || ((char)wParam == 'Q'))
	 {
	    msg = WM_CLOSE;
	    wParam = 0;
	    lParam = 0L;
	 }
	 /* fall through to close the window */

      case WM_CLOSE:
	 return DefWindowProc(hWnd, msg, wParam, lParam);

      case WM_DESTROY:
	 DeleteObject(brGrey);
	 DeleteObject(penLoad);
	 DeleteObject(penLink);
	 DeleteObject(penMem);
	 DeleteObject(penBG);
	 PostQuitMessage(0);
	 return DefWindowProc(hWnd, msg, wParam, lParam);

      default:
	 return DefWindowProc(hWnd, msg, wParam, lParam);
   }

   return 0L;
}


int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nShow)
{  int	width, height;                  /* width and height of the window */
   int screen_height;
   int  i;
   int  name_width;
   int 	font_height;
   WNDCLASS wc;
   MSG 	msg;
   HDC 	hDC;
   TEXTMETRIC tm;
   int 	argc;
   char **argv;
   RECT rect;

   InitSemaphore(&one_access, 1);
   GetArgcArgv(&argc, &argv);   /* we prefer using this to lpszCmdLine */
   
   if( argc <= 1 )
   {  
      RmNetwork Network;

      /* Get details of the current network into local memory */
      Network = RmGetNetwork();
      if (Network == (RmNetwork) NULL)
      {  
	 fprintf(stderr, "mslights: failed to get network details.\n");
	 fprintf(stderr, "        : %s\n", RmMapErrorToString(RmErrno));
	 exit(EXIT_FAILURE);
      }

      /* Walk down the current network examining every processor        */
      (void) RmApplyProcessors(Network, (int (*)(RmProcessor,...))&NetworkCount);
      info = CreateProcTable( nprocs);
      if( info == NULL )
      {  fprintf(stderr, "failed to get info\n\n");
	 exit(EXIT_FAILURE);
      }
      nprocs = 0;
      (void) RmApplyProcessors(Network, &NetworkWalk);
   }
   else
   {  
      info = CreateProcTable( argc -1);
      if( info == NULL )
      {  fprintf(stderr,"failed to get info\n\n");
	 exit(EXIT_FAILURE);
      }

      for( i = 1; i < argc; i++ )
      {
         if(InitProcTable(nprocs, argv[i]))
	    nprocs++;
      } 
   }

   wc.style 		= CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc 	= MainWndProc;
   wc.hInstance 	= hInst;
   wc.cbClsExtra 	= 0;
   wc.cbWndExtra 	= 0;
   wc.hIcon 		= NULL;
   wc.hCursor 		= LoadCursor(NULL, IDC_CROSS);
   wc.hbrBackground 	= GetStockObject(LTGRAY_BRUSH);
   wc.lpszMenuName 	= NULL;
   wc.lpszClassName 	= WNAME;
   if (!RegisterClass(&wc))
      return 255;           /* program ends */

   hMainWnd = CreateWindow(WNAME, "MS Lights",
			WSTYLE,
			0, 0, 5, 5, NULL, NULL, hInst, NULL);
      

   hDC 		= GetDC(hMainWnd);
   hFont 	= GetStockObject(ANSI_VAR_FONT);

   SelectObject(hDC, hFont);
   GetTextMetrics(hDC, &tm);      
   ReleaseDC(hMainWnd, hDC);

   name_width 	= maxw * tm.tmAveCharWidth + border;
   panel_width 	= (tm.tmHeight + tm.tmExternalLeading)*6 + 4*border + name_width;

   font_height 	= tm.tmHeight + tm.tmExternalLeading;

   if ((7*border) < font_height)
      panel_height = font_height;
   else
      panel_height = 7*border;

   screen_height = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYCAPTION);
   if (panel_y(nprocs + 1) > screen_height)
   {
      nprocs = screen_height / (panel_y(1) - (border));
      fprintf(stderr,"mslights: Limiting processors monitored to %d\n",nprocs);
   }


   panel_xstart = name_width + border*2;
   panel_xend 	= panel_xstart + panel_width;

   bar_x 	= name_width + 3*border;
   bar_length 	= panel_width - 2*border;

	
   rect.left 	= 0;
   rect.top 	= 0;

   rect.right 	= (int)(panel_width + name_width + 3*border);
   rect.bottom 	= (int)((panel_height+border)*nprocs + border);

   /* adjust the window size to allow for the correct client area */
   AdjustWindowRect(&rect, WSTYLE, FALSE);
   width 	= rect.right + abs(rect.left);
   height 	= rect.bottom + abs(rect.top);

   /* now we resize the window */
   MoveWindow(hMainWnd, 0, 0, width, height, FALSE);

   /* show the window on the screen */
   ShowWindow(hMainWnd, SW_SHOWNOACTIVATE);


   /* spawn a parallel task that updates the bars */
   if (Fork(10000, update_bars, 0)) 
      /* enter the standard windows message loop */
      while (GetMessage(&msg, NULL, NULL, NULL))
      {
	  TranslateMessage(&msg);   /* does nothing but included */
	  DispatchMessage(&msg);
      }

   /* the application has terminated so unregister class */
   UnregisterClass(WNAME, hInst);
	      
   return msg.wParam;
} /* WinMain */



