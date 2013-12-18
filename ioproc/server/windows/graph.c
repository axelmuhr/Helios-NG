/**************************************************************************/
/**************************************************************************/
/**            Helios I/O Server Under Microsoft Windows                 **/
/**                                                                      **/
/**        Copyright (C) 1993, Perihelion Software Limited               **/
/**                      All Rights Reserved                             **/
/**                                                                      **/
/**   Graph.c (version 2)                                                **/
/**                                                                      **/
/**************************************************************************/
/**************************************************************************/

#define Graphics_Module
#include "helios.h"
#include <ddeml.h>

#include "windows\graph.h"
#include "windows\hash.h"
#include "windows\graphdef.h"

/**************************************************************************/
/**   This provides the Microsoft Windows support to the I/O server.     **/
/**   There are four groups of functions:                                **/
/**          Graph_XXXX - These are the /graphics server functions,      **/
/**                       providing the usual server functionality.      **/
/**                                                                      **/
/**          IO_XXXX    - All requests via the private protocol above    **/
/**                       specify a function number, which maps onto     **/
/**                       an IO_XXXX function.                           **/
/**                                                                      **/
/**          GraphProc  - The callback function for all window classes   **/
/**                       registered from Helios.                        **/
/**                                                                      **/
/**          all others - Private helper functions used internally       **/
/**                       and by the message loop for handling the       **/
/**                       deferred messages.                             **/
/**                                                                      **/
/**************************************************************************/

long FAR PASCAL GraphProc(HWND, unsigned, WORD, LONG);

#include "windows\graphfn.h"       /* include IO_XXXX function prototypes */


/****** Helper functions for the graphics server **************************/
PRIVATE Graphnode  *find_app_name(LPCSTR);
PRIVATE Graphnode  *create_graph_node(LPCSTR);
PRIVATE Windownode *find_window_node(HWND);
PRIVATE BOOL remove_window_node(HWND);
PRIVATE void close_windows(Windownode *);
PRIVATE void unregister_classes(Classlink *);
PRIVATE Classnode *find_class(LPCSTR);
PRIVATE BOOL remove_class(Classnode *);
PRIVATE Classnode *add_class(WNDCLASS FAR *);
PRIVATE void remove_menus(Menunode *);
PRIVATE void remove_instance(DdeInstancenode *);
PRIVATE void remove_menu_item(HMENU, UINT);
PRIVATE BOOL remove_app_servers(Graphnode *);
PRIVATE BOOL remove_app_convs(Graphnode *);
PRIVATE void marshal_params(HWND, UINT, WPARAM, LPARAM);
PRIVATE word add_msg_to_buffer(Windownode *, LPMSG);
PRIVATE void send_msg_to_port(Port, LPMSG, word);
PRIVATE void build_message_and_send(HWND, UINT, WPARAM, LPARAM);
PRIVATE void pop_mcb(void);


/* variables shared between graph.c, winsrvr.c and hash.c */
LRESULT DeferredResult;
BOOL    bExitMessageLoop;
HashTable hashWindows;         /* a hash table for created windows */

/* local global variables */

static int       open_graph = 0;
static DirHeader Graph_List;
static List      Class_List;
static List      MCB_List;

/* Graph_Routines is an array containing the addresses of the routines    */
/* providing the graphics interface. This allows us to index into the     */
/* function, speeding up the whole process. The position of each function */
/* in the array, corresponds to the message number received from the      */
/* graphics library, invoking the function. Eg. The Graphics Library on   */
/* the transputer issues the OpenGraph call with a message number of 0.   */
/* The message number is OR'ed with the private message format,           */
/* i.e. 0x60000000. The private message format is masked off, leaving the */
/* message number, which is 0.                                            */


VoidFnPtr Graph_Routines[num_routines]=
   {
     IO_RegisterPort,       IO_RegisterClass,       IO_CreateWindow,            /* 0,, */
     IO_DefWindowProc,      IO_PostQuitMessage,     IO_PostMessage,
     IO_DestroyWindow,      IO_ShowWindow,          IO_UnregisterClass,
     IO_GetWindowRect,      IO_GetClientRect,       IO_CreatePen,               /* ,10, */
     IO_CreateSolidBrush,   IO_CreateFont,          IO_SelectObject,
     IO_GetDC,              IO_ReleaseDC,           IO_BeginPaint,
     IO_EndPaint,           IO_GetStockObject,      IO_DPtoLP,                  /* ,,20 */
     IO_LPtoDP,             IO_ClientToScreen,      IO_ScreenToClient,
     IO_MoveTo,             IO_LineTo,              IO_FillRect,
     IO_TextOut,            IO_GetTextMetrics,      IO_GetTextAlign,
     IO_SetTextAlign,       IO_GetTextExtent,       IO_Ack,                     /* 30,, */
     IO_NegAck,             IO_DeleteObject,        IO_MoveWindow,
     IO_AdjustWindowRect,   IO_EnableWindow,        IO_GetActiveWindow,
     IO_GetClassLong,       IO_GetClassWord,        IO_GetDesktopWindow,        /* ,40, */
     IO_GetFocus,           IO_GetNextWindow,       IO_GetParent,
     IO_GetTopWindow,       IO_GetWindow,           IO_GetWindowLong,
     IO_GetWindowWord,      IO_IsChild,             IO_IsWindow,                /* ,,50 */
     IO_IsWindowEnabled,    IO_IsWindowVisible,     IO_SetClassLong,
     IO_SetClassWord,       IO_SetWindowLong,       IO_SetWindowWord,
     IO_SetCapture,         IO_ReleaseCapture,      IO_SetCursor,
     IO_LoadCursor,         IO_GetBkColor,          IO_GetBkMode,               /* 60,, */
     IO_GetDeviceCaps,      IO_GetMapMode,          IO_GetSystemMetrics,
     IO_GetTextColor,       IO_SetBkColor,          IO_SetBkMode,
     IO_SetMapMode,         IO_SetTextColor,        IO_Arc,                     /* ,70, */
     IO_Chord,              IO_CreateHatchBrush,    IO_CreatePatternBrush,
     IO_Ellipse,            IO_FloodFill,           IO_GetCurrentPosition,
     IO_GetNearestColor,    IO_GetPixel,            IO_GetPolyFillMode,         /* ,,80 */
     IO_GetROP2,            IO_InvalidateRect,      IO_InvertRect,
     IO_Pie,                IO_Polygon,             IO_Polyline,
     IO_Rectangle,          IO_SetPixel,            IO_SetPolyFillMode,
     IO_SetROP2,            IO_UnrealizeObject,     IO_UpdateWindow,            /* 90,, */
     IO_ValidateRect,       IO_CreatePalette,       IO_GetPaletteEntries,
     IO_GetNearestPaletteIndex, IO_RealizePalette,  IO_SelectPalette,
     IO_SetPaletteEntries,  IO_BitBlt,              IO_CreateBitmap,            /* ,100, */
     IO_CreateCompatibleBitmap, IO_CreateDIBitmap,  IO_GetBitmapBits,
     IO_GetDIBits,          IO_SetBitmapBits,       IO_SetDIBits,
     IO_SetDIBitsToDevice,  IO_CreateCompatibleDC,  IO_RegisterBuffer,          /* ,,110 */
     IO_AppendBuffer,       IO_DeleteBuffer,        IO_AppendMenu,
     IO_CheckMenuItem,      IO_CreatePopupMenu,     IO_CreateMenu,
     IO_DeleteMenu,         IO_DestroyMenu,         IO_DrawMenuBar,
     IO_EnableMenuItem,     IO_GetMenu,             IO_GetMenuItemCount,        /* 120,, */
     IO_GetMenuItemID,      IO_GetMenuState,        IO_GetMenuString,
     IO_GetSubMenu,         IO_GetSystemMenu,       IO_InsertMenu,
     IO_RemoveMenu,         IO_SetMenu,             IO_FindWindow,              /* ,130, */
     IO_RegisterIOMenu,     IO_DeleteDC,            IO_GetUpdateRect,
     IO_SendMessage,        IO_GetObject,           IO_DdeInitialize,
     IO_DdeUninitialize,    IO_DdeConnectList,      IO_DdeQueryNextServer,      /* ,,140 */
     IO_DdeDisconnectList,  IO_DdeConnect,          IO_DdeDisconnect,
     IO_DdeReconnect,       IO_DdeQueryConvInfo,    IO_DdeSetUserHandle,
     IO_DdeAbandonTransaction, IO_DdePostAdvise,    IO_DdeEnableCallback,
     IO_DdeNameService,     IO_DdeClientTransaction,IO_DdeCreateDataHandle,     /* 150,, */
     IO_DdeAddData,         IO_DdeGetData,          IO_DdeAccessData,
     IO_DdeUnaccessData,    IO_DdeFreeDataHandle,   IO_DdeGetLastError,
     IO_DdeCreateStringHandle, IO_DdeQueryString,   IO_DdeFreeStringHandle,     /* ,160, */
     IO_DdeKeepStringHandle,IO_DdeCmpStringHandles, IO_DdeReturnResult,
     IO_DdeReturnAdvise,    IO_GetBuffer,           IO_SetScrollRange,
     IO_GetScrollRange,     IO_SetScrollPos,        IO_GetScrollPos,		/* 170 */
     IO_ShowScrollBar,	    IO_EnableScrollBar
   };

/**************************************************************************/
/* This is the start of the graphics server code.  The graphics server    */
/* provides the support for the usual server protocol (opening, deleting, */
/* etc.) and the private protocol which is used to call the windows API   */
/* and private support functions (between graph.lib and the graphics      */
/* server).                                                               */
/**************************************************************************/


void Graph_Testfun(ret_code)
   bool *ret_code;
{
  *ret_code = graphics_registered;
}


void Graph_InitServer(myco)
   Conode *myco;
{
   InitList(&(Graph_List.list));  /* Initialise the linked list, containing */
   InitList(&Class_List);         /* init list of registered classes        */
   InitList(&MCB_List);
   lpfnDdeCallback = MakeProcInstance((FARPROC)DdeCallback, PROGINSTANCE);
   Graph_List.entries = 0L;       /* info on all open graphic windows       */
   SkipTimer = FALSE;
}

/* Graph_TidyServer will close down the graphics server on the PC. It closes */
/* all open graphic windows, deletes the linked list of open graphic         */
/* windows and then UnRegisters all registered classes.                      */

void Graph_TidyServer(myco)
   Conode *myco;
{ Graphnode *app, *next;

  SkipTimer = TRUE;

  /* for each app, remove the windows and unregister the classes */
  for (app = (Graphnode *) Graph_List.list.head; /* go through list of   */
       app->node.node.next ne (Node *) NULL; )   /* open graphic windows */
  {
       close_windows((Windownode *)app->window_list.head);
       unregister_classes((Classlink *)app->class_list.head);
       remove_menus((Menunode *)app->menu_list.head);
       remove_instance((DdeInstancenode *)app->dde_list.head);
       next = (Graphnode *) app->node.node.next;
       free(Remove(&(app->node.node)));             /* Delete linked   */
       app = next;
   }
   
   TidyHashTable(&hashWindows);
   FreeProcInstance(lpfnDdeCallback);
   use(myco);
}

/* Graph_Open is invoked by the standard Helios call Open, when applied to */
/* the graphics server. The function will open a new stream for either the */
/* graphics server itself, for an existing window, or for a new window. A  */
/* stream for a new graphic window is opened only if the O_Create flag is  */
/* set, and the OpenMode is specified, in the Helios Open call.            */
/*                                                                         */
/* To open a new graphic window, we first initialise a Graphnode for the   */
/* window, and add the node to the list of open graphic windows. If this   */
/* successful then we open a new stream for the window.                    */

void Graph_Open(myco)
   Conode *myco;
{
  char      *temp;
  word      openmode = mcb->Control[OpenMode_off];
  Graphnode *new_app;

  if (!strcmp(IOname, "graphics")) {      /* Stream for the server? */
     NewStream(Type_Directory, Flags_Closeable,
                (word) ((DirHeader far *) &(Graph_List)), WindowDir_Handlers);
     return;
  }

  for (temp = IOname; (*temp ne '/') && (*temp ne '\0'); temp++);

  if (*temp eq '\0') {
      Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
  }

  temp++;
  new_app = find_app_name(temp);   /* does the specified window exist? */
  if (new_app eq (Graphnode *) NULL) {  /* No, so stream is for a new    */
    if (openmode & O_Create) {            /* window */
          /* Initialise a Graphnode for the new window */
       if ((new_app = create_graph_node((LPCSTR) temp)) eq (Graphnode *) NULL) {
          Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_File, 0L, 0L);
          return;
       }
       open_graph = 1;  /* If Graphnode successfully created and added to */
                        /* to the list of open graphic windows, set       */
                        /* open graph flag. This flag will be accessed in */
                        /* the open_a_graph routine.                      */

    }
    else {
      Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
      return;
    }
  }

  NewStream(Type_File, Flags_Closeable + Flags_Interactive,
              (word) ((Graphnode far *) new_app), Graph_Handlers);
  use(myco);
}


/* Graph_Locate services the Helios Locate Request. */

void Graph_Locate(myco)
   Conode *myco;
{
   char      *name;
   word      temp;
   Graphnode *app;

   if (!strcmp(IOname, "graphics")) {   /* Locate the server */
      temp = FormOpenReply(Type_Directory, 0L, -1L, -1L);
      Request_Return(ReplyOK, open_reply, temp);
      return;
   }

   /* Locate request not for the server, but for a graphic window!! */

   for (name = IOname; (*name ne '/') && (*name ne '\0'); name++);

   if (*name eq '\0') {
      Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
   }

   name++;
   app = find_app_name(name);   /* Try to find graphic window */
   if (app ne (Graphnode *) NULL) {
      temp = FormOpenReply(Type_File, 0L, -1L, -1L);
      Request_Return(ReplyOK, open_reply, temp);
      return;
   }

   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
   use(myco);
}


/* Graph_Delete is used to delete an open graphic window. This routine is    */
/* invoked by the Graphics Library on the transputer, or can be invoked by   */
/* using the "rm" command on the Helios command line, to delete any apps     */
/* that did not terminate properly.  This will close all windows and         */
/* unregister all classes associated with that app.                          */
/*                                                                           */
/* The app can only be closed if there are no open streams associated with   */
/* it. After deleting the window, the Graphnode associated with the window   */
/* is removed from the list of open graphic windows.                         */

void Graph_Delete(myco)
   Conode *myco;
{ char      *name;
  Graphnode *app;

  if (!strcmp(IOname, "graphics")) {    /* Cannot delete the server */
     Request_Return(EC_Error + SS_IOProc + EG_InUse + EO_Server, 0L, 0L);
     return;
  }

  for (name = IOname; (*name ne '/') && (*name ne '\0'); name++);

  if (*name eq '\0') {
     Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
     return;
  }

  name++;
  app = find_app_name(name);

  if (app eq (Graphnode *) NULL) {  /* Does not exist!! */
    Request_Return(EC_Error + SS_IOProc + EG_Unknown +EO_File, 0L, 0L);
    return;
  }
  else {
    if (app->stream_count ne 0) {  /* can only delete if not in use */
        Request_Return(EC_Error + SS_IOProc + EG_InUse + EO_Stream, 0L, 0L);
        return;
    }

    /* first remove all windows and unregister all classes */
    close_windows((Windownode *)app->window_list.head);
    unregister_classes((Classlink *)app->class_list.head);
    remove_menus((Menunode *)app->menu_list.head);
    remove_app_convs(app);
    remove_app_servers(app);
    remove_instance((DdeInstancenode *)app->dde_list.head);

    RemoveAppHandles(&hashWindows, (void FAR *)app);

    free(Remove(&(app->node.node)));      /* remove the window from   */
    Graph_List.entries--;
    Request_Return(ReplyOK, 0L, 0L);
    return;
  }
  use(myco);
}



word Graph_InitStream(myco)
   Conode *myco;
{
   Graphnode *graph = (Graphnode *) myco->extra;

   graph->stream_count++;
   return(ReplyOK);
}


word Graph_TidyStream(myco)
   Conode *myco;
{
   Graphnode *graph = (Graphnode *) myco->extra;

   graph->stream_count--;
   return(0L);
}


/* Graph_PrivateStream intercepts a message from the transputer, generated  */
/* by the Graphics Library, masks off the Private Stream message format, to */
/* get the message number, and uses the message number to index into the    */
/* function that performs the task required by the Graphics Library. The    */
/* graphic window for which the message was intended, now becomes the       */
/* active graphic window, although the window as such is not activated in   */
/* MS-WINDOWS.                                                              */

void Graph_PrivateStream(myco)
   Conode *myco;
{
   int   mess_no;
   Graphnode *app = (Graphnode *) myco->extra;

   mess_no         = (int) mcb->MsgHdr.FnRc & 0xFFF;  /* get the message number */
   (Graph_Routines[mess_no]) (app);    /* call the desired function */
}


/* We should not get any read requests, but we set up a suitable reply anyway */

void Graph_Read(myco)
   Conode *myco;
{
   Request_Return(ReadRc_EOF, 0L, 0L);
   use(myco);
}


/* We should not get any write requests, but we set up a suitable reply */
/* anyway */

void Graph_Write(myco)
   Conode *myco;
{
   word count     = mcb->Control[WriteSize_off];
   word replyport = mcb->MsgHdr.Reply;

   mcb->Control[Reply1_off] = count;
   mcb->MsgHdr.Reply = replyport;
   Request_Return(WriteRc_Done, 1L, 0L);
   use(myco);
}


/* Graph_Close is used to close a stream associated with an open graphic */
/* window. The stream is actually a co-routine, which is destroyed by    */
/* calling seppuku.                                                      */

void Graph_Close(myco)
   Conode *myco;
{
   if (mcb->MsgHdr.Reply ne 0L)
       Request_Return(ReplyOK, 0L, 0L);

   Graph_TidyStream(myco);    /* decrement stream count */
   Graph_Delete(myco);    /* closed stream indicates a terminated program */

   Seppuku();                 /* kill off stream co-routine */
}


/* There are no strange stream attributes associated with graphic windows */
/* at present, but we may introduce some later.                           */

void Graph_GetAttr(myco)
   Conode *myco;
{
   Graphnode *graph = (Graphnode *) myco->extra;

   CopyAttributes((Attributes far *) mcb->Data,
                      (Attributes far *) &(graph->attr));
   Request_Return(ReplyOK, 0L, (word) sizeof(Attributes));
}


void Graph_SetAttr(myco)
   Conode *myco;
{
   Graphnode *graph = (Graphnode *) myco->extra;

   CopyAttributes((Attributes far *) &(graph->attr),
                          (Attributes far *) mcb->Data);
   Request_Return(ReplyOK, 0L, 0L);
}



/* Graph_ObjectInfo is used to return information about either the graphics */
/* server, or an application. For the graphics server, we return the        */
/* standard ObjInfo structure used in the I/O Server, as well as the width  */
/* and height of the screen, obtained from the GetSystemmetrics call.       */
/* The size of the message buffer in the server is also returned!.          */
/* The structure returned to the transputer is the DeviceInfo structure.    */
/*                                                                          */
/* For an application, we return the number of registered classes, the      */
/* number of open windows and the number of open streams.                   */

void Graph_ObjectInfo(myco)
   Conode *myco;
{
   DeviceInfo far *Heliosinfo = (DeviceInfo far *) mcb->Data;
   char           *name;
   Graphnode      *app;
   HDC            hDC;
   int            ct;

   Heliosinfo->obj.DirEntry.Type   = swap(Type_File);       /* standard I/O */
   Heliosinfo->obj.DirEntry.Flags  = swap(0L);              /* server return*/
   Heliosinfo->obj.DirEntry.Matrix = swap(DefFileMatrix);   /* to an        */
   Heliosinfo->obj.Account         = swap(0L);              /* ObjectInfo   */
   Heliosinfo->obj.Creation        = swap(Startup_Time);    /* request      */
   Heliosinfo->obj.Access          = swap(Startup_Time);
   Heliosinfo->obj.Modified        = swap(Startup_Time);

   if (!strcmp(IOname, "graphics")) {     /* Info on the graphics server */
      Heliosinfo->obj.DirEntry.Type   = swap(Type_Directory);
      Heliosinfo->obj.DirEntry.Matrix = swap(DefDirMatrix);
      strcpy(Heliosinfo->obj.DirEntry.Name, "graphics");
      Heliosinfo->obj.Size            = swap(0L);

#ifdef never
    /* Put in the info required by the Graphics Library*/

      Heliosinfo->info[0] = maxdata;
      Heliosinfo->info[1] = (word)PROGINSTANCE;
#endif

      Request_Return(ReplyOK, 0L, (word) sizeof(DeviceInfo));
      return;
   }

   for (name = IOname; (*name ne '/') && (name ne '\0'); name++);

   if (*name eq '\0') {
      Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
   }

   name++;
   app = find_app_name(name);

   if (app ne (Graphnode *) NULL) {   /* Info on open graphic windows */
      strcpy(Heliosinfo->obj.DirEntry.Name, app->node.direntry.Name);
      Heliosinfo->obj.Size = swap((word) app->stream_count);

#ifdef never
      for (ct = 2; ct < 18; ct++)
         Heliosinfo->info[ct] = 0;

      Heliosinfo->info[0] = (word)app->num_classes;
      Heliosinfo->info[1] = (word)app->num_windows;
#endif

      Request_Return(ReplyOK, 0L, (word) sizeof(DeviceInfo));
      return;
   }

   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
   use(myco);
}


/***************************************************************************/
/* TimerProc                                                               */
/*                                                                         */
/* Intercepts the timer function and sends a null message to each app.     */
/***************************************************************************/

void CALLBACK TimerProc(HWND hWnd, UINT message, UINT wParam, DWORD lParam)
{
   Graphnode *app;
   MSG msg;
   int insert;

   if (SkipTimer)
        return;

   for (app = (Graphnode *) Graph_List.list.head;   /* search through list */
        app->node.node.next ne (Node *) NULL;       /* of open graphic     */
        app = (Graphnode *) app->node.node.next)    /* windows             */
   {
        msg.hwnd = NULL;
        msg.message = WM_NULL;
        msg.wParam = WM_TIMER;
        msg.lParam = 0;

        /* add the msg to the buffer */
        insert = app->msg_head;
        memcpy(&(app->msg_buffer[insert].msg), &msg, sizeof(MSG));
        app->msg_buffer[insert].sequence_no = app->send_sequence++;
        app->msg_head = (app->msg_head + 1) % MAX_MESSAGES;

        /* now send it to the port */
        send_msg_to_port(app->msg_port, &msg, app->msg_buffer[insert].sequence_no);
   }
}



/* The following routines are the helper functions used internally and     */
/* exported for use in the message loop (deferred functions).              */


/***************************************************************************/
/* InitTable                                                               */
/*                                                                         */
/* Initialises the Hash table                                              */
/***************************************************************************/

void InitTable(void)
{
   InitHashTable(&hashWindows);
}


/***************************************************************************/
/* NotifyMenu                                                              */
/*                                                                         */
/* Purpose                                                                 */
/*      When the shell receives a menu item that is unknown, it calls this */
/*      function assuming that the application that installed a menu item  */
/*      has registered with the server.                                    */
/*                                                                         */
/* Parameters                                                              */
/*      ID - The ID of the menu item called                                */
/***************************************************************************/

void NotifyMenu(UINT ID)
{
   Graphnode *app;
   Menunode  *menunode;

   for (app = (Graphnode *) Graph_List.list.head;   /* search through list */
        app->node.node.next ne (Node *) NULL;       /* of open graphic     */
        app = (Graphnode *) app->node.node.next)    /* windows             */
   {
        for (menunode=(Menunode *)app->menu_list.head;
             menunode->node.next ne (Node *)NULL;
             menunode = (Menunode *)menunode->node.next)
        {
            if (menunode->ID eq ID)
            {
                MSG msg;
                int insert;

                msg.hwnd = NULL;
                msg.message = WM_COMMAND;
                msg.wParam = ID;
                msg.lParam = 0;

                /* add the msg to the buffer */
                insert = app->msg_head;
                memcpy(&(app->msg_buffer[insert].msg), &msg, sizeof(MSG));
                app->msg_buffer[insert].sequence_no = app->send_sequence++;
                app->msg_head = (app->msg_head + 1) % MAX_MESSAGES;

                /* now send it to the port */
                send_msg_to_port(app->msg_port, &msg, app->msg_buffer[insert].sequence_no);
                return;
            }
        }
   }
}


/***************************************************************************/
/* close_windows                                                           */
/*                                                                         */
/* Parameters:                                                             */
/*      root  - A pointer to the root of the Windownode list of windows.   */
/*                                                                         */
/* Returns                                                                 */
/*      none                                                               */
/*                                                                         */
/* Side-effects                                                            */
/*      The routine will walk the list of windows, closing them and        */
/*      freeing the space used by the node.                                */
/*                                                                         */
/***************************************************************************/

void close_windows(Windownode *root)
{
   Windownode *window, *next;
   HWND hWnd;

   for (window=root;  window->node.next ne (Node *)NULL; )
   {
        /* remove the list of deferred messages */
        next = (Windownode *)window->node.next;
        hWnd = window->hwnd;
        free(Remove(&(window->node)));
        RemoveHandle(&hashWindows, hWnd, HT_HWND);
        DestroyWindow(hWnd);
        window = next;
   }
}

/***************************************************************************/
/* unregister_classes                                                      */
/*                                                                         */
/* Parameters:                                                             */
/*      root  - A pointer to the root of the Classlink list of classes.    */
/*                                                                         */
/* Returns                                                                 */
/*      none                                                               */
/*                                                                         */
/* Side-effects                                                            */
/*      The routine will walk the list of classes and unregister them      */
/*      freeing the space used by the node.                                */
/*                                                                         */
/***************************************************************************/

void unregister_classes(Classlink *root)
{
   Classlink *class, *next;

   for (class=root;  class->node.next ne (Node *)NULL; )
   {
        /* now remove class */
        remove_class(class->class);

        next = (Classlink *) class->node.next;
        free(Remove(&(class->node)));
        class = next;
   }
}


/***************************************************************************/
/* find_class                                                              */
/*                                                                         */
/* Parameters:                                                             */
/*      name  - The name of the class to find.                             */
/*                                                                         */
/* Returns                                                                 */
/*      A pointer to the Classnode if it exists or NULL                    */
/*                                                                         */
/***************************************************************************/

Classnode *find_class(LPCSTR name)
{
   Classnode *class;
   BOOL      bNotFound = TRUE;

   for (class = (Classnode *)Class_List.head;
        (bNotFound = mystrcmp((char *)(name),
                              (char *)&(class->szClassName))) &&
        (class->node.next ne (Node *)NULL);
        class = (Classnode *)class->node.next);

   if (bNotFound)
        return (Classnode *)NULL;

   return class;
}



/***************************************************************************/
/* remove_class                                                            */
/*                                                                         */
/* Parameters:                                                             */
/*      root  - A pointer to the class to be removed.                      */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if UnregisterClass succeeds or FALSE if not                   */
/*                                                                         */
/* Side-effects                                                            */
/*      The routine decrements the usage counter, and if it is now zero    */
/*      it will unregister the class and remove the node from memory.      */
/*                                                                         */
/***************************************************************************/

BOOL remove_class(Classnode *class)
{
   BOOL bRet=TRUE;
   if ((--(class->usage)) eq 0)
   {
        bRet = UnregisterClass((LPCSTR)&(class->szClassName[0]), PROGINSTANCE);
        free(Remove(&(class->node)));
   }
   return bRet;
}



/***************************************************************************/
/* add_class                                                               */
/*                                                                         */
/*      The routine searches the list for the class name.  If found and    */
/*      some of the settings are different, the addition will fail (false  */
/*      is returned).  If the settings are the same, a pointer to that     */
/*      Classnode is returned and the usage incremented.  If not found,    */
/*      the class is registered, a Classnode structure is added and        */
/*      returned to the calling program.                                   */
/*                                                                         */
/* Parameters:                                                             */
/*      lpWc  - A pointer to the class information to add                  */
/*                                                                         */
/* Returns                                                                 */
/*      NULL if unsuccessful, otherwise a pointer to the Classnode.        */
/*                                                                         */
/***************************************************************************/

Classnode *add_class(WNDCLASS FAR *lpWc)
{
   Classnode *class;
   ATOM      atomClass;

   if ((class = find_class(lpWc->lpszClassName)) ne NULL)
   {
        /* if found */
        if ((lpWc->style eq class->style) &&
            (lpWc->cbClsExtra eq class->cbClsExtra) &&
            (lpWc->cbWndExtra eq class->cbWndExtra) &&
            (lpWc->hInstance eq class->hInstance) &&
            (lpWc->hCursor eq class->hCursor) &&
            (lpWc->hbrBackground eq class->hbrBackground))
        {
           class->usage++;
           return class;
        }

        return (Classnode *)NULL;       /* error - name already used */
   }

   /* if not found, register class, add node and return new node */
   if ((atomClass=RegisterClass(lpWc)))
   {
        class = (Classnode *)malloc(sizeof(Classnode));
        if (class eq (Classnode *)NULL)
        {
           UnregisterClass(lpWc->lpszClassName, PROGINSTANCE);
           return (Classnode *)NULL;
        }
        class->usage = 1;
        class->style = lpWc->style;
        class->cbClsExtra = lpWc->cbClsExtra;
        class->cbWndExtra = lpWc->cbWndExtra;
        class->hInstance = lpWc->hInstance;
        class->hCursor = lpWc->hCursor;
        class->hbrBackground = lpWc->hbrBackground;
        class->atomClass = atomClass;
        strcpy(&(class->szClassName[0]), lpWc->lpszClassName);

        AddTail(&(class->node), &Class_List);
        return class;
   }

   return (Classnode *)NULL;            /* error - registration failed */
}


/***************************************************************************/
/* remove_menus                                                            */
/*                                                                         */
/* Parameters:                                                             */
/*      root  - A pointer to the root of the Menunode items                */
/*                                                                         */
/* Returns                                                                 */
/*      none                                                               */
/*                                                                         */
/* Side-effects                                                            */
/*      The routine will walk the list of registered menus and remove them */
/*      from the shell menu.                                               */
/*                                                                         */
/***************************************************************************/

void remove_menus(Menunode *root)
{
   Menunode *menunode, *next;
   HWND hWnd;

   for (menunode=root;  menunode->node.next ne (Node *)NULL; )
   {
        hWnd = FindWindow(szPopupClass, NULL);
        if (hWnd ne NULL)
            remove_menu_item(GetMenu(hWnd), menunode->ID);

        next = (Menunode *) menunode->node.next;
        free(Remove(&(menunode->node)));
        menunode = next;
   }
}


/***************************************************************************/
/* remove_menu_item                                                        */
/*                                                                         */
/* Parameters:                                                             */
/*      hMenu - The menu to look in                                        */
/*      ID    - The ID of the item to remove                               */
/*                                                                         */
/* Returns                                                                 */
/*      none                                                               */
/*                                                                         */
/* Side-effects                                                            */
/*      The routine will find the I/O server shell menu and search for the */
/*      item to remove it from the menu.  If it is a popup menu, it is     */
/*      destroyed.                                                         */
/*                                                                         */
/***************************************************************************/

void remove_menu_item(HMENU hMenu, UINT ID)
{
   int  i, count;

   if (hMenu eq (HMENU)NULL)
        return;
   count = GetMenuItemCount(hMenu);
   for (i=0; i<count; i++)
   {
        UINT id = GetMenuItemID(hMenu, i);
        if (id eq -1)
            remove_menu_item(GetSubMenu(hMenu, i), ID);
        else
            if (id eq ID)
            {
                RemoveMenu(hMenu, i, MF_BYPOSITION);
                break;
            }
   }
}


/***************************************************************************/
/* add_instance                                                            */
/*                                                                         */
/* Purpose                                                                 */
/*      This adds a DdeInstancenode to the list associated with an app.    */
/*                                                                         */
/* Parameters                                                              */
/*      app     - A pointer to the application node to which the instance  */
/*                is to be added                                           */
/*      idInst  - The instance to be added                                 */
/*      port    - The port to which this instance's data is to be sent     */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL add_instance(Graphnode FAR *app, DWORD idInst, Port port)
{
   DdeInstancenode *dde=(DdeInstancenode *)malloc(sizeof(DdeInstancenode));
   if (dde eq (DdeInstancenode *)NULL)
        return FALSE;
   dde->idInst = idInst;
   dde->port = port;
   dde->app = app;
   dde->bAdviseFlag = FALSE;
   dde->nAdviseCount = 0;
   dde->nAdviseTotal = 0;
   AddTail(&(dde->node), &(app->dde_list));
   return TRUE;
}


/***************************************************************************/
/* find_instance                                                           */
/*                                                                         */
/* Purpose                                                                 */
/*      This returns the DdeInstancenode containing the instance           */
/*                                                                         */
/* Parameters                                                              */
/*      app     - A pointer to the application node                        */
/*      idInst  - The instance to search for                               */
/*                                                                         */
/* Returns                                                                 */
/*      A pointer to the instance or null if it does not exist             */
/***************************************************************************/

DdeInstancenode FAR *find_instance(Graphnode FAR *app, DWORD idInst)
{
   DdeInstancenode *dde;
   BOOL             bNotFound = TRUE;

   for (dde = (DdeInstancenode *)app->dde_list.head;
        (dde->node.next != (Node *)NULL) &&
        (bNotFound=(dde->idInst != idInst));
        dde = (DdeInstancenode *)dde->node.next);

   if (bNotFound)
       return (DdeInstancenode *)NULL;
   
   return dde;
}


/***************************************************************************/
/* delete_instance                                                         */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes the instance from the app's instance list             */
/*                                                                         */
/* Parameters                                                              */
/*      app     - A pointer to the application node to which the instance  */
/*                is to be removed                                         */
/*      idInst  - The instance to be removed                               */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL delete_instance(Graphnode FAR *app, DWORD idInst)
{
   DdeInstancenode *dde = find_instance(app, idInst);

   if (dde == (DdeInstancenode *)NULL)
        return FALSE;
   free(Remove(&(dde->node)));
   return TRUE;
}


/***************************************************************************/
/* remove_instance                                                         */
/*                                                                         */
/* Parameters:                                                             */
/*      root  - A pointer to the instance to remove                        */
/***************************************************************************/

void remove_instance(DdeInstancenode *dde)
{
    DdeInstancenode *next;
    while (dde->node.next != (Node *)NULL)
    {
        next = (DdeInstancenode *)dde->node.next;
        free(Remove(&dde->node));
        dde = next;
    }
}


/***************************************************************************/
/* add_server                                                              */
/*                                                                         */
/* Purpose                                                                 */
/*      This adds an HT_DDE_SERVER entry into the hash table with an       */
/*      associated DdeInstancenode *.                                      */     
/*                                                                         */
/* Parameters                                                              */
/*      hsz     - The server string handle                                 */
/*      pInst   - The pointer to the DdeInstancenode                       */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL add_server(BIG_HANDLE hsz, DdeInstancenode FAR *pInst)
{
   AddHandle(&hashWindows, hsz, HT_DDE_SERVER, (VOID FAR *)pInst);
   return TRUE;
}


/***************************************************************************/
/* delete_server                                                           */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes an HT_DDE_SERVER entry from the hash table            */
/*                                                                         */
/* Parameters                                                              */
/*      hsz     - The server string handle                                 */
/*      pInst   - The pointer to the DdeInstancenode                       */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL delete_server(BIG_HANDLE hsz, DWORD idInst)
{
   void FAR *pPos;
   DdeInstancenode *dde;
   Node *node;
   pPos = NULL;
   
   do
   {
        dde = (DdeInstancenode FAR *)GetData(&hashWindows, hsz, HT_DDE_SERVER, &pPos);
        if (idInst == dde->idInst)
        {
            node = (Node *)pPos;
            free(Remove(node->prev));
            return TRUE;
        }
   } while (pPos != NULL);

   return FALSE;
}


/***************************************************************************/
/* remove_servers                                                          */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes all HT_DDE_SERVER entries that correspond with the    */
/*      indicated instance.                                                */
/*                                                                         */
/* Parameters                                                              */
/*      idInst  - The instance id to be removed                            */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL remove_servers(DWORD idInst)
{
   void FAR *pPos;
   DdeInstancenode *dde;
   Node *node;
   int i;
   
   for (i=0; i<HashTableSize; i++)
   {
        pPos = (void FAR *)hashWindows.bucket[i].head;
        while ((dde = (DdeInstancenode FAR *)GetData(&hashWindows, NULL, HT_DDE_SERVER, &pPos)) != NULL)
            if (idInst == dde->idInst)
            {
                node = (Node *)pPos;
                free(Remove(node->prev));
            }
   }

   return FALSE;
}


/***************************************************************************/
/* remove_app_servers                                                      */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes all HT_DDE_SERVER entries that correspond with the    */
/*      indicated application.                                             */
/*                                                                         */
/* Parameters                                                              */
/*      app     - The pointer to the application data.                     */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL remove_app_servers(Graphnode *app)
{
   void FAR *pPos;
   DdeInstancenode *dde;
   Node *node;
   int i;
   
   for (i=0; i<HashTableSize; i++)
   {
        pPos = (void FAR *)hashWindows.bucket[i].head;
        while ((dde = (DdeInstancenode FAR *)GetData(&hashWindows, NULL, HT_DDE_SERVER, &pPos)) != NULL)
            if (app == dde->app)
            {
                node = (Node *)pPos;
                free(Remove(node->prev));
            }
   }

   return FALSE;
}


/***************************************************************************/
/* find_server                                                             */
/*                                                                         */
/* Purpose                                                                 */
/*      This returns a DdeInstancenode which is registered as the indi-    */
/*      cated server                                                       */
/*                                                                         */
/* Parameters                                                              */
/*      hsz     - The handle to search for                                 */
/*                                                                         */
/* Returns                                                                 */
/*      A pointer to the instance or null if it does not exist             */
/***************************************************************************/

DdeInstancenode FAR *find_server(BIG_HANDLE hsz)
{
   void FAR *pPos;
   pPos = NULL;

   return (DdeInstancenode FAR *)GetData(&hashWindows, hsz, HT_DDE_SERVER, &pPos);
}


/***************************************************************************/
/* add_conv                                                                */
/*                                                                         */
/* Purpose                                                                 */
/*      This adds an HT_CONV entry in the hash tables with the associated  */
/*      DdeInstancenode *.                                                 */
/*                                                                         */
/* Parameters                                                              */
/*      hConv   - The conversation handle                                  */
/*      pInst   - The pointer to the DdeInstancenode                       */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL add_conv(BIG_HANDLE hConv, DdeInstancenode FAR *pInst)
{
   DdeConvnode *conv = (DdeConvnode *)malloc(sizeof(DdeConvnode));
   if (conv == (DdeConvnode *)NULL)
       return FALSE;

   conv->dde = pInst;
   conv->bIsBlocked = FALSE;
   conv->hData = NULL;
   conv->hAdvData = NULL;
   AddHandle(&hashWindows, hConv, HT_HCONV, (VOID FAR *)conv);
   return TRUE;
}


/***************************************************************************/
/* delete_conv                                                             */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes an HT_HCONV entry from the hash table                 */
/*                                                                         */
/* Parameters                                                              */
/*      hConv   - The conversation handle                                  */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL delete_conv(BIG_HANDLE hConv)
{
   void FAR *pPos;
   DdeConvnode *conv;
   Node *node;

   pPos = NULL;
   conv = (DdeConvnode *)GetData(&hashWindows, hConv, HT_HCONV, &pPos);
   if (conv != (DdeConvnode *)NULL)
   {
       node = (Node *)pPos;
       free(Remove(node->prev));
       free(conv);
       return TRUE;
   }
   return FALSE;
}


/***************************************************************************/
/* remove_convs                                                            */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes all HT_HCONV entries that correspond with the         */
/*      indicated instance.                                                */
/*                                                                         */
/* Parameters                                                              */
/*      idInst  - The instance id to be removed                            */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL remove_convs(DWORD idInst)
{
   void FAR *pPos;
   DdeConvnode *conv;
   Node *node;
   int i;
   
   for (i=0; i<HashTableSize; i++)
   {
        pPos = (void FAR *)hashWindows.bucket[i].head;
        while ((conv = (DdeConvnode FAR *)GetData(&hashWindows, NULL, HT_HCONV, &pPos)) != NULL)
            if (idInst == conv->dde->idInst)
            {
                node = (Node *)pPos;
                DdeDisconnect(GetHandle((void FAR *)node->prev));
                free(Remove(node->prev));
                free(conv);
            }
   }

   return FALSE;
}


/***************************************************************************/
/* remove_app_convs                                                        */
/*                                                                         */
/* Purpose                                                                 */
/*      This removes all HT_HCONV entries that correspond with the         */
/*      indicated application.                                             */
/*                                                                         */
/* Parameters                                                              */
/*      app     - The pointer to the application data.                     */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL remove_app_convs(Graphnode *app)
{
   void FAR *pPos;
   DdeConvnode *conv;
   Node *node;
   int i;
   
   for (i=0; i<HashTableSize; i++)
   {
        pPos = (void FAR *)hashWindows.bucket[i].head;
        while ((conv = (DdeConvnode FAR *)GetData(&hashWindows, NULL, HT_HCONV, &pPos)) != NULL)
            if (app == conv->dde->app)
            {
                node = (Node *)pPos;
                DdeDisconnect(GetHandle((void FAR *)node->prev));
                free(Remove(node->prev));
                free(conv);
            }
   }

   return FALSE;
}


/***************************************************************************/
/* find_conv                                                               */
/*                                                                         */
/* Purpose                                                                 */
/*      This returns a DdeConvnode for the conversation handle             */
/*                                                                         */
/* Parameters                                                              */
/*      hConv   - The handle to search for                                 */
/*                                                                         */
/* Returns                                                                 */
/*      A pointer to the instance or null if it does not exist             */
/***************************************************************************/

DdeConvnode FAR *find_conv(BIG_HANDLE hConv)
{
   void FAR *pPos;
   pPos = NULL;

   return (DdeConvnode *)GetData(&hashWindows, hConv, HT_HCONV, &pPos);
}


/***************************************************************************/
/* find_conv_dde                                                           */
/*                                                                         */
/* Purpose                                                                 */
/*      This returns a DdeInstancenode which is registered as the indi-    */
/*      cated conversation                                                 */
/*                                                                         */
/* Parameters                                                              */
/*      hConv   - The handle to search for                                 */
/*                                                                         */
/* Returns                                                                 */
/*      A pointer to the instance or null if it does not exist             */
/***************************************************************************/

DdeInstancenode FAR *find_conv_dde(BIG_HANDLE hConv)
{
   void FAR *pPos;
   DdeConvnode *conv;
   pPos = NULL;

   conv = (DdeConvnode *)GetData(&hashWindows, hConv, HT_HCONV, &pPos);
   if (conv == NULL)
       return NULL;
   else
       return conv->dde;
}


/***************************************************************************/
/* add_pending                                                             */
/*                                                                         */
/* Purpose                                                                 */
/*      This adds a pending connection to the instance record passed to    */
/*      the routine                                                        */
/*                                                                         */
/* Parameters                                                              */
/*      pInst     - The pointer to the DdeInstancenode                     */
/*      hszServer - The server string handle                               */
/*      hszTopic  - The topic string handle                                */
/*                                                                         */
/* Returns                                                                 */
/*      TRUE if successful                                                 */
/***************************************************************************/

BOOL add_pending(BIG_HANDLE hszServer, BIG_HANDLE hszTopic, DdeInstancenode FAR *pInst)
{
   DdeServicenode *service = (DdeServicenode *)malloc(sizeof(DdeServicenode));
   if (service == (DdeServicenode *)NULL)
        return FALSE;
   service->server = hszServer;
   service->dde = pInst;
   AddHandle(&hashWindows, hszTopic, HT_DDE_SERVICE, (VOID FAR *)service);
   return TRUE;
}


/***************************************************************************/
/* find_pending                                                            */
/*                                                                         */
/* Purpose                                                                 */
/*      This finds the indicated server/topic pair and returns the         */
/*      associated instance pointer.  It also removes the service node     */
/*      of the returned pointer.                                           */
/*                                                                         */
/* Parameters                                                              */
/*      hszServer - The server string handle                               */
/*      hszTopic  - The topic string handle                                */
/*                                                                         */
/* Returns                                                                 */
/*      The DdeInstancenode * or NULL if not found                         */
/***************************************************************************/

DdeInstancenode FAR * find_pending(BIG_HANDLE hszServer, BIG_HANDLE hszTopic)
{
   void FAR *pPos;
   DdeServicenode *service;
   DdeInstancenode *dde;
   Node *node;
   pPos = NULL;
   
   do
   {
        service = (DdeServicenode FAR *)GetData(&hashWindows, hszTopic, HT_DDE_SERVICE, &pPos);
        if (hszServer == service->server)
        {
            dde = service->dde;
            node = (Node *)pPos;
            free(Remove(node->prev));
            free(service);
            return dde;
        }
   } while (pPos != NULL);

   return NULL;
}


/***************************************************************************/
/* find_window_node                                                        */
/*                                                                         */
/* Parameters :-                                                           */
/*                                                                         */
/*         handle - A MS-WINDOWS handle for an open graphic window         */
/*                                                                         */
/* Function :-                                                             */
/*      The function searches through the list of open graphic windows,    */
/*      until it finds the Windownode structure which contains the speci-  */
/*      field handle.                                                      */
/*                                                                         */
/* Returns :-                                                              */
/*      The address of the Windownode structure if the handle is found,    */
/*      NULL if the handle was not found.                                  */
/***************************************************************************/

Windownode *find_window_node(handle)
   HWND handle;
{
   void FAR *pPos;
   pPos = NULL;
   return (Windownode *)GetData(&hashWindows, handle, HT_HWND, &pPos);
}

BOOL remove_window_node(HWND hWnd)
{
   Windownode *window = find_window_node(hWnd);
   if (window ne (Windownode *)NULL)
   {
        window->app->num_windows--;
        RemoveHandle(&hashWindows, window->hwnd, HT_HWND);
        free(Remove(&(window->node)));
        return TRUE;
   }
   return FALSE;
}


/***************************************************************************/
/* find_app_name                                                           */
/*                                                                         */
/* Parameters :-                                                           */
/*                                                                         */
/*         name - Pointer to name to search for                            */
/*                                                                         */
/* Function :-                                                             */
/*      The function searches through the list of open apps to get a       */
/*      pointer to a Graphnode corresponding to that name.  It returns     */
/*      NULL if not found.                                                 */
/*                                                                         */
/* Returns :-                                                              */
/*      The address of the Graphnode structure if the handle is found,     */
/*      NULL if the handle was not found.                                  */
/***************************************************************************/

Graphnode *find_app_name(name)
   LPCSTR name;
{
   Graphnode *app;  /* address of the Graphnode structure */

   for (app = (Graphnode *) Graph_List.list.head;   /* search through list */
        app->node.node.next ne (Node *) NULL;       /* of open graphic     */
        app = (Graphnode *) app->node.node.next)    /* windows             */
   {
        if (!mystrcmp((char *)name, &(app->node.direntry.Name[0])))
           return(app);   /* return address of Graphnode structure if found */
   }

   return((Graphnode *) NULL);  /* NULL if not found */
}


/***************************************************************************/
/* create_graph_node                                                       */
/*                                                                         */
/* Parameters :-                                                           */
/*                                                                         */
/*         temp - A string containing a unique name for the graphic        */
/*                window associated with the Graphnode structure being     */
/*                created.                                                 */
/*                                                                         */
/* Function :-                                                             */
/*      The function creates a Graphnode structure, initialises the fields */
/*      of the structure, and adds the structure to the end of the list of */
/*      open graphic windows. Memory for the Graphnode structure is        */
/*      allocated from local memory.                                       */
/*                                                                         */
/* Returns :-                                                              */
/*      The address of the Graphnode structure if it was successfully      */
/*      created and added to the list, NULL if it was not.                 */
/***************************************************************************/

Graphnode *create_graph_node(name)
   LPCSTR name;
{
   Graphnode *new_app;       /* address of the created Graphnode structure */

   /* Allocate memory for the Graphnode structure. */
   new_app = (Graphnode *)malloc(sizeof(Graphnode));
   if (new_app eq NULL)
   {
      return((Graphnode *) NULL);
   }

   new_app->stream_count = 0;            /* no streams open yet   */
   new_app->msg_port = NullPort;         /* no port registered    */
   InitList(&(new_app->class_list));     /* no classes registered */
   InitList(&(new_app->window_list));    /* no windows open yet   */
   InitList(&(new_app->menu_list));
   InitList(&(new_app->dde_list));
   new_app->num_classes = 0;
   new_app->num_windows = 0;
   new_app->app_id = 0;

   new_app->msg_head = 0;
   new_app->msg_tail = 0;
   new_app->send_sequence = 0L;

   strcpy(&(new_app->node.direntry.Name[0]), name);   /* insert name of */
                                        /* window associated with structure */

   new_app->node.direntry.Type = Type_File;
   Graph_List.entries++;
   InitAttributes((Attributes far *) &(new_app->attr));
   AddTail(&(new_app->node.node), &(Graph_List.list)); /* Add to list of  */
   return(new_app);                                    /* graphic windows */
}


void unmarshal_params(HWND *hWnd, UINT *msg, WPARAM *wParam, LPARAM *lParam)
{
   *hWnd = (HWND)mcb->Control[0];
   *msg = (UINT)mcb->Control[1];
   *wParam = (WPARAM)mcb->Control[2];
   *lParam = (LPARAM)mcb->Control[3];
}


void marshal_params(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   mcb->Control[0] = (word)hWnd;
   mcb->Control[1] = (word)msg;
   mcb->Control[2] = (word)wParam;
   mcb->Control[3] = (word)lParam;
}

void my_Request_Return(word FnRc, word CtrlSize, word DataSize)
{
   pop_mcb();
   Request_Return(ReplyOK, CtrlSize, DataSize);
}

Graphnode *get_first_app(void)
{
    return (Graphnode *)Graph_List.list.head;
}

void send_dde_to_port(Port port, UINT wType, UINT wFmt, DWORD hConv,
                DWORD hsz1, DWORD hsz2, DWORD hData, DWORD dwData1, DWORD dwData2)
{
   mcb->MsgHdr.Reply = port;
   mcb->MsgHdr.Dest = NullPort;
   mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
   mcb->Control[0] = wType;
   mcb->Control[1] = wFmt;
   mcb->Control[2] = hConv;
   mcb->Control[3] = hsz1;
   mcb->Control[4] = hsz2;
   mcb->Control[5] = hData;
   mcb->Control[6] = dwData1;
   mcb->Control[7] = dwData2;
   Request_Return(ReplyOK, 8L, 0);
}

word add_msg_to_buffer(Windownode *window, LPMSG lpMsg)
{
   Graphnode *app = window->app;
   int insert = app->msg_head;

   memcpy(&(app->msg_buffer[insert].msg), lpMsg, sizeof(MSG));
   app->msg_buffer[insert].sequence_no = app->send_sequence++;
   app->msg_head = (app->msg_head + 1) % MAX_MESSAGES;

   return app->msg_buffer[insert].sequence_no;
}

void send_msg_to_port(Port port, LPMSG lpMsg, word sequence)
{
   mcb->MsgHdr.Reply = port;
   mcb->MsgHdr.Dest = NullPort;
   mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
   mcb->Control[0] = sequence;
   mcb->Control[1] = lpMsg->hwnd;
   mcb->Control[2] = lpMsg->message;
   mcb->Control[3] = lpMsg->wParam;
   mcb->Control[4] = lpMsg->lParam;
   Request_Return(ReplyOK, 5L, 0);
}

void build_message_and_send(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
   MSG msg;
   Windownode *window = find_window_node(hWnd);

   if (window eq (Windownode *)NULL)
       return;

   msg.hwnd = hWnd;
   msg.message = iMessage;
   msg.wParam = wParam;
   msg.lParam = lParam;
   send_msg_to_port(window->app->msg_port, &msg, add_msg_to_buffer(window, &msg));
}

void push_mcb(void)
{
   MCBnode *pmcb = (MCBnode *)malloc(sizeof(MCBnode));
   memcpy(&(pmcb->mcb), mcb, sizeof(MCB));
   AddHead(&(pmcb->node), &MCB_List);
}

void pop_mcb(void)
{
   MCBnode *pmcb = (MCBnode *)MCB_List.head;
   if (pmcb->node.next != (Node *)NULL)
   {
        memcpy(mcb, &(pmcb->mcb), sizeof(MCB));
        free(Remove(&(pmcb->node)));
   }
}

/***************************************************************************/
/***************************************************************************/
/*                                                                         */
/* The following functions are to handle the function calls made from      */
/* Helios, that is the ones in graph.lib.  Most of the function names are  */
/* the same as the windows equivalent (with IO_ on the front).  This       */
/* indicates that IO_RegisterClass, for instance, provides the             */
/* RegisterClass functionality for the graphics libraries.  There are      */
/* some functions which are for internal use (between the graph.lib and    */
/* IO server), such as the IO_RegisterPort function used by the library    */
/* to register the port to which the windows messages will be sent.        */
/*                                                                         */
/***************************************************************************/
/***************************************************************************/

/* 000 */
void IO_RegisterPort(Graphnode *app)
{
	app->msg_port		= (Port)(mcb->MsgHdr.Reply);
	mcb->Control[0]		= maxdata;
	mcb->Control[1]		= (word)PROGINSTANCE;
	mcb->MsgHdr.Flags	= MsgHdr_Flags_preserve;
	Request_Return(ReplyOK, 2L, 0L);
	GraphicsDebug(("graphics (internal): RegisterPort 0x%x", app->msg_port));
}

/* 001 */
void IO_RegisterClass(Graphnode *app)
{
   WNDCLASS wc;
   Classnode *class;

   if (mcb->Data[0] eq '\0')
   {
        /* error in class name */
        mcb->Control[0] = (word)NULL;
        Request_Return(ReplyOK, 1L, 0L);
        return;
   }

   wc.style = (UINT)(mcb->Control[0]);
   wc.lpfnWndProc = (WNDPROC)GraphProc;
   wc.cbClsExtra = (int)(mcb->Control[1]);
   wc.cbWndExtra = (int)(mcb->Control[2]);
   wc.hInstance = (HINSTANCE)(mcb->Control[3]);
   wc.hIcon = LoadIcon(PROGINSTANCE, "graphicon");
   wc.hCursor = (HCURSOR)(mcb->Control[4]);
   wc.hbrBackground = (HBRUSH)(mcb->Control[5]);
   wc.lpszMenuName = NULL;
   wc.lpszClassName = (LPCSTR)&(mcb->Data[0]);

   push_mcb();
   if ((class = add_class(&wc)) ne (Classnode *)NULL)
   {
        /* class creation OKAY, so add a link to it */
        Classlink *link = (Classlink *)malloc(sizeof(Classlink));
        if (link eq (Classlink *)NULL)
        {
          remove_class(class);
          mcb->Control[0] = (word)NULL;
          my_Request_Return(ReplyOK, 1L, 0L);
          return;
        }
        link->class = class;
        Addtail(&(link->node), &(app->class_list));
        mcb->Control[0] = (word)class->atomClass;
        GraphicsDebug(("RegisterClass succeeded: '%s'", wc.lpszClassName));
        my_Request_Return(ReplyOK, 1L, 0L);
        return;
   }

   mcb->Control[0] = (word)NULL;
   GraphicsDebug(("RegisterClass failed: '%s'", wc.lpszClassName));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 002 */
void IO_CreateWindow(Graphnode *app)
{
   Classnode *class;
   /* get parameters from control and data blocks */
   DWORD dwStyle = (DWORD)mcb->Control[0];
   int   x = (int)mcb->Control[1],
         y = (int)mcb->Control[2],
         width = (int)mcb->Control[3],
         height = (int)mcb->Control[4];
   HWND  hwndParent = (HWND)mcb->Control[5];
   HMENU hMenu = (HMENU)mcb->Control[6];
   HINSTANCE hInst = (HINSTANCE)mcb->Control[7];
   LPCSTR lpszClassName, lpszWindowName;
   HWND  hWnd;

   lpszClassName = (LPCSTR)&(mcb->Data[0]);
   push_mcb();
   class = find_class(lpszClassName);

   /* find start of window name string in the control block */
   for (lpszWindowName = lpszClassName; *lpszWindowName ne '\0'; lpszWindowName++);
   lpszWindowName++;

   GraphicsDebug(("CreateWindow(,,0x%lx,%d,%d,%d,%d,0x%x,0x%x,0x%x)",
        dwStyle, x, y, width, height, hwndParent, hMenu, hInst));

   hWnd = CreateWindow(lpszClassName, lpszWindowName, dwStyle,
                       x, y, width, height, hwndParent, hMenu, hInst, NULL);
   if (hWnd ne NULL)
   {
        if (class ne (Classnode *)NULL)
        {
            /* make a new Windownode if one of our classes (not "button" etc) */
            Windownode *window = (Windownode *)malloc(sizeof(Windownode));
            if (window eq (Windownode *)NULL)
            {
                DestroyWindow(hWnd);
                mcb->Control[0] = (word)NULL;
                my_Request_Return(ReplyOK, 1L, 0L);
                return;
            }
            window->num = 0;
            window->hwnd = hWnd;
            window->class = class;
            window->app = app;
            window->rectInvalid.left = 0;
            window->rectInvalid.top = 0;
            window->rectInvalid.right = 0;
            window->rectInvalid.bottom = 0;
            app->num_windows++;
            AddHandle(&hashWindows, hWnd, HT_HWND, (VOID FAR *)window);
            AddTail(&(window->node), &(app->window_list));
            SendMessage(hWnd, WM_CREATE, 0, 0L);
        }
        mcb->Control[0] = (word)hWnd;
        GraphicsDebug(("CreateWindow succeeded: '%s','%s' HWND=0x%x", lpszClassName, lpszWindowName, hWnd));
        my_Request_Return(ReplyOK, 1L, 0L);
        return;
   }
   mcb->Control[0] = (word)NULL;
   GraphicsDebug(("CreateWindow failed: '%s','%s'", lpszClassName, lpszWindowName));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 003 */
void IO_DefWindowProc(Graphnode *app)
{
   HWND   hWnd;
   UINT   message;
   WPARAM wParam;
   LPARAM lParam;
   LRESULT ret;

   unmarshal_params(&hWnd, &message, &wParam, &lParam);

   /* call function and supply function return as reply */
   push_mcb();
   ret = DefWindowProc(hWnd, message, wParam, lParam);
   mcb->Control[0] = ret;
   GraphicsDebug(("DefWindowProc(0x%x,0x%x,0x%x,0x%lx) = %x", hWnd, message, wParam, lParam, ret));
   my_Request_Return(ReplyOK, 1L, 0L);
}


/* 004 */
void IO_PostQuitMessage(Graphnode *app)
{
    /* defunct message - handled on Helios side */
}


/* 005 */
void IO_PostMessage(Graphnode *app)
{
   HWND   hWnd;
   UINT   msg;
   WPARAM wParam;
   LPARAM lParam;
   LRESULT ret;

   unmarshal_params(&hWnd, &msg, &wParam, &lParam);
   push_mcb();

   /* we may need to trap some other messages here, so that they are not */
   /* posted but handled in some other way.                              */

   /* else send via usual PostMessage */
   ret = PostMessage(hWnd, msg, wParam, lParam);
   mcb->Control[0] = ret;
   GraphicsDebug(("PostMessage(0x%x,0x%x,0x%x,0x%lx) = 0x%x", hWnd, msg, wParam, lParam, ret));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 006 */
void IO_DestroyWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   LRESULT ret;

   push_mcb();
   ret = DestroyWindow(hWnd);
   mcb->Control[0] = (word)ret;
   GraphicsDebug(("DestroyWindow(0x%x) = 0x%x", hWnd, ret));
   my_Request_Return(ReplyOK, 1L, 0L);
}


/* 007 */
void IO_ShowWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int  nCmdShow = (int)mcb->Control[1];
   LRESULT ret;

   push_mcb();
   ret = (word)ShowWindow(hWnd, nCmdShow);
   mcb->Control[0] = (word)ret;
   GraphicsDebug(("ShowWindow(0x%x,0x%x) = 0x%x", hWnd, nCmdShow, ret));
   my_Request_Return(ReplyOK, 1L, 0L);
}


/* 008 */
void IO_UnregisterClass(Graphnode *app)
{
   HINSTANCE hInst = (HINSTANCE)mcb->Control[0];
   LPCSTR lpszClassName = (LPCSTR)&(mcb->Data[0]);
   Classnode *class = find_class(lpszClassName);
   BOOL bRet;

   push_mcb();
   if (class eq (Classnode *)NULL)
        bRet = FALSE;
   else
   {
        /* we have our class, so find classlink pointing to it & remove */
        Classlink *link;
        for (link = (Classlink *)app->class_list.head;
                   (class != link->class) &&
                   (link->node.next ne (Node *) NULL);
                   link = (Classlink *) link->node.next);
        if (class == link->class)
        {
            free(Remove(&(link->node)));
            bRet = remove_class(class);
        }
        else
            bRet = FALSE;
   }
   mcb->Control[0] = (word)bRet;
   GraphicsDebug(("UnregisterClass(0x%x,%s) = 0x%x", hInst, lpszClassName, bRet));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 009 */
void IO_GetWindowRect(Graphnode *app)
{
   HWND hWnd = mcb->Control[0];
   RECT rect;
   push_mcb();
   GetWindowRect(hWnd, &rect);
   memcpy(&mcb->Data[0], &rect, sizeof(RECT));
   GraphicsDebug(("GetWindowRect(0x%x) = (%d,%d,%d,%d)", hWnd, rect.left,
                 rect.top, rect.right, rect.bottom));
   my_Request_Return(ReplyOK, 0L, sizeof(RECT));
}

/* 010 */
void IO_GetClientRect(Graphnode *app)
{
   HWND hWnd = mcb->Control[0];
   RECT rect;
   push_mcb();
   GetClientRect(hWnd, &rect);
   memcpy(&mcb->Data[0], &rect, sizeof(RECT));
   GraphicsDebug(("GetClientRect(0x%x) = (%d,%d,%d,%d)", hWnd, rect.left,
                 rect.top, rect.right, rect.bottom));
   my_Request_Return(ReplyOK, 0L, sizeof(RECT));
}

/* 011 */
void IO_CreatePen(Graphnode *app)
{
   int style = (int)mcb->Control[0];
   int width = (int)mcb->Control[1];
   COLORREF color = (COLORREF)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)CreatePen(style, width, color);
   GraphicsDebug(("CreatePen(0x%x,%d,0x%08lx) = 0x%x", style, width, color, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HPEN, app);
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 012 */
void IO_CreateSolidBrush(Graphnode *app)
{
   COLORREF color = (COLORREF)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)CreateSolidBrush(color);
   GraphicsDebug(("CreateSolidBrush(0x%08lx) = 0x%x", color, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBRUSH, app);
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 013 */
void IO_CreateFont(Graphnode *app)
{
   int height = (int)mcb->Control[0];
   int width = (int)mcb->Control[1];
   int escape = (int)mcb->Control[2];
   int orientation = (int)mcb->Control[3];
   int weight = (int)mcb->Control[4];
   BYTE italic = mcb->Data[0];
   BYTE underline = mcb->Data[1];
   BYTE strikeout = mcb->Data[2];
   BYTE charset = mcb->Data[3];
   BYTE precision = mcb->Data[4];
   BYTE clipprecision = mcb->Data[5];
   BYTE quality = mcb->Data[6];
   BYTE pitch = mcb->Data[7];
   LPCSTR lpszFace = &mcb->Data[8];
   push_mcb();

   mcb->Control[0] = (word)CreateFont(height, width, escape, orientation, weight,
                                italic, underline, strikeout, charset,
                                precision, clipprecision, quality, pitch,
                                lpszFace);
   GraphicsDebug(("CreateFont(...) = 0x%x", mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HFONT, app);
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 014 */
void IO_SelectObject(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   HGDIOBJ obj = (HGDIOBJ)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)SelectObject(hDC, obj);
   GraphicsDebug(("SelectObject(0x%x,0x%x) = 0x%x", hDC, obj, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 015 */
void IO_GetDC(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetDC(hWnd);
   GraphicsDebug(("GetDC(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 016 */
void IO_ReleaseDC(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   HDC  hDC  = (HDC)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)ReleaseDC(hWnd, hDC);
   GraphicsDebug(("ReleaseDC(0x%x,0x%x) = 0x%x", hWnd, hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 017 */
void IO_BeginPaint(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   Windownode *window = find_window_node(hWnd);
   PAINTSTRUCT FAR *lpps = (PAINTSTRUCT FAR *)&mcb->Data[0];
   push_mcb();
   if (window != (Windownode *)NULL)
        InvalidateRect(hWnd, &window->rectInvalid, FALSE);
   mcb->Control[0] = (word)BeginPaint(hWnd, lpps);
   GraphicsDebug(("BeginPaint(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, sizeof(PAINTSTRUCT));
}

/* 018 */
void IO_EndPaint(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   Windownode *window = find_window_node(hWnd);
   PAINTSTRUCT FAR *lpps = (PAINTSTRUCT FAR *)&mcb->Data[0];
   push_mcb();
   EndPaint(hWnd, lpps);
   if (window ne (Windownode *)NULL)
   {
        window->rectInvalid.left = 0;
        window->rectInvalid.top = 0;
        window->rectInvalid.right = 0;
        window->rectInvalid.bottom = 0;
   }
   GraphicsDebug(("EndPaint(0x%x)", hWnd));
   my_Request_Return(ReplyOK, 0L, 0L);
}

/* 019 */
void IO_GetStockObject(Graphnode *app)
{
   int obj_no = (int)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetStockObject(obj_no);
   GraphicsDebug(("GetStockObject(%d) = 0x%x", obj_no, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 020 */
void IO_DPtoLP(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int pnts = (int)mcb->Control[1];
   LPPOINT lppt = (LPPOINT)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = DPtoLP(hDC, lppt, pnts);
   my_Request_Return(ReplyOK, 1L, sizeof(POINT)*pnts);
}

/* 021 */
void IO_LPtoDP(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int pnts = (int)mcb->Control[1];
   LPPOINT lppt = (LPPOINT)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = LPtoDP(hDC, lppt, pnts);
   my_Request_Return(ReplyOK, 1L, sizeof(POINT)*pnts);
}

/* 022 */
void IO_ClientToScreen(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   LPPOINT lppt = (LPPOINT)&mcb->Data[0];
   push_mcb();
   ClientToScreen(hWnd, lppt);
   my_Request_Return(ReplyOK, 0L, sizeof(POINT));
}

/* 023 */
void IO_ScreenToClient(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   LPPOINT lppt = (LPPOINT)&mcb->Data[0];
   push_mcb();
   ScreenToClient(hWnd, lppt);
   my_Request_Return(ReplyOK, 0L, sizeof(POINT));
}

/* 024 */
void IO_MoveTo(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int x   = (int)mcb->Control[1];
   int y   = (int)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = MoveTo(hDC, x, y);
   GraphicsDebug(("MoveTo(0x%x,%d,%d) = 0x%x", hDC, x, y, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 025 */
void IO_LineTo(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int x   = (int)mcb->Control[1];
   int y   = (int)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = LineTo(hDC, x, y);
   GraphicsDebug(("LineTo(0x%x,%d,%d) = 0x%x", hDC, x, y, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 026 */
void IO_FillRect(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   HBRUSH hbr = (HBRUSH)mcb->Control[1];
   LPRECT lprt = (LPRECT)&mcb->Data[0];
   push_mcb();
   FillRect(hDC, lprt, hbr);
   GraphicsDebug(("FillRect(0x%x,0x%x,(%d,%d,%d,%d))", hDC, hbr, lprt->left,
                lprt->top, lprt->right, lprt->bottom));
   my_Request_Return(ReplyOK, 0L, 0L);
}

/* 027 */
void IO_TextOut(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int xStart = (int)mcb->Control[1];
   int yStart = (int)mcb->Control[2];
   int cb = (int)mcb->Control[3];
   LPCSTR lpszString = (LPCSTR)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = TextOut(hDC, xStart, yStart, lpszString, cb);
   GraphicsDebug(("TextOut(0x%x,%d,%d,%d,%s) = %d", hDC, xStart, yStart,
                cb, lpszString, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 028 */
void IO_GetTextMetrics(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   TEXTMETRIC FAR *lptm = (TEXTMETRIC FAR *)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = GetTextMetrics(hDC, lptm);
   GraphicsDebug(("GetTextMetrics(0x%x) = 0x%x", hDC, mcb->Control[0]));

   /* I must move the data so that it is word aligned for helios */
   memmove(&mcb->Data[26], &mcb->Data[25], 6);
   my_Request_Return(ReplyOK, 1L, sizeof(TEXTMETRIC)+1);
}

/* 029 */
void IO_GetTextAlign(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = GetTextAlign(hDC);
   GraphicsDebug(("GetTextAlign(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 030 */
void IO_SetTextAlign(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   UINT align = (UINT)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = SetTextAlign(hDC, align);
   GraphicsDebug(("SetTextAlign(0x%x,0x%x) = 0x%x", hDC, align, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 031 */
void IO_GetTextExtent(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int cb = (int)mcb->Control[1];
   LPCSTR lpszString = (LPCSTR)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = GetTextExtent(hDC, lpszString, cb);
   GraphicsDebug(("GetTextExtent(0x%x,%d,%s) = 0x%x", hDC, cb, lpszString, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1L, 0L);
}

/* 032 */
void IO_Ack(Graphnode *app)
{
   word AckNo = mcb->Control[0];
   int *msg_tail = &app->msg_tail;
   int *msg_head = &app->msg_head;
   MSGnode *msg_buffer = &app->msg_buffer[0];

   Request_Return(ReplyOK, 0L, 0L);

   GraphicsDebug(("(internal) Ack WM_ message 0x%08lx", AckNo));

   while ((*msg_tail ne *msg_head) &&
          (AckNo >= msg_buffer[*msg_tail].sequence_no))
        *msg_tail = (*msg_tail + 1) % MAX_MESSAGES;
}

/* 033 */
void IO_NegAck(Graphnode *app)
{
   word NegAckNo = mcb->Control[0];
   int i;
   int *msg_tail = &app->msg_tail;
   int *msg_head = &app->msg_head;
   MSGnode *msg_buffer = &app->msg_buffer[0];

   Request_Return(ReplyOK, 0L, 0L);

   GraphicsDebug(("(internal) NegAck WM_ message 0x%08lx", NegAckNo));

   /* first ack to 1 less than negack */
   while ((*msg_tail ne *msg_head) &&
          (NegAckNo > msg_buffer[*msg_tail].sequence_no))
        *msg_tail = (*msg_tail + 1) % MAX_MESSAGES;

   /* now resend the rest */
   for (i=*msg_tail; i<*msg_head; i++)
       send_msg_to_port(app->msg_port, &(msg_buffer[i].msg), msg_buffer[i].sequence_no);
}

/* 034 */
void IO_DeleteObject(Graphnode *app)
{
   HGDIOBJ hObject = (HGDIOBJ)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = DeleteObject(hObject);
   GraphicsDebug(("DeleteObject(0x%x) = 0x%x", hObject, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       RemoveHandleApp(&hashWindows, hObject, (void FAR *)app);
   my_Request_Return(ReplyOK, 1, 0);
}


/* 035 */
void IO_MoveWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int x = (int)mcb->Control[1];
   int y = (int)mcb->Control[2];
   int width = (int)mcb->Control[3];
   int height = (int)mcb->Control[4];
   BOOL bRepaint = (BOOL)mcb->Control[5];
   push_mcb();
   mcb->Control[0] = MoveWindow(hWnd, x, y, width, height, bRepaint);
   GraphicsDebug(("MoveWindow(0x%x,%d,%d,%d,%d,%d) = 0x%x", hWnd, x, y, width, height, bRepaint, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}


/* 036 */
void IO_AdjustWindowRect(Graphnode *app)
{
   LONG dwStyle = (LONG)mcb->Control[0];
   BOOL bMenu = (BOOL)mcb->Control[1];
   LPRECT lprc = (LPRECT)&mcb->Data[0];
   push_mcb();
   AdjustWindowRect(lprc, dwStyle, bMenu);
   GraphicsDebug(("AdjustWindowRect(0x%x,0x%x)", dwStyle, bMenu));
   my_Request_Return(ReplyOK, 0, sizeof(RECT));
}

/* 037 */
void IO_EnableWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   BOOL bEnable = (BOOL)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)EnableWindow(hWnd, bEnable);
   GraphicsDebug(("EnableWindow(0x%x,0x%x) = 0x%x", hWnd, bEnable, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 038 */
void IO_GetActiveWindow(Graphnode *app)
{
   push_mcb();
   mcb->Control[0] = (word)GetActiveWindow();
   GraphicsDebug(("GetActiveWindow() = 0x%x", mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 039 */
void IO_GetClassLong(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetClassLong(hWnd, offset);
   GraphicsDebug(("GetClassLong(0x%x,%d) = 0x%lx", hWnd, offset, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 040 */
void IO_GetClassWord(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetClassWord(hWnd, offset);
   GraphicsDebug(("GetClassWord(0x%x,%d) = 0x%x", hWnd, offset, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 041 */
void IO_GetDesktopWindow(Graphnode *app)
{
   push_mcb();
   mcb->Control[0] = (word)GetDesktopWindow();
   GraphicsDebug(("GetDesktopWindow() = 0x%x", mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 042 */
void IO_GetFocus(Graphnode *app)
{
   push_mcb();
   mcb->Control[0] = (word)GetFocus();
   GraphicsDebug(("GetFocus() = 0x%x", mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 043 */
void IO_GetNextWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   UINT uFlags = (UINT)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetNextWindow(hWnd, uFlags);
   GraphicsDebug(("GetNextWindow(0x%x,0x%x) = 0x%x", hWnd, uFlags, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 044 */
void IO_GetParent(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetParent(hWnd);
   GraphicsDebug(("GetParent(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 045 */
void IO_GetTopWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetTopWindow(hWnd);
   GraphicsDebug(("GetTopWindow(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 046 */
void IO_GetWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   UINT uRel = (UINT)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetWindow(hWnd, uRel);
   GraphicsDebug(("GetWindow(0x%x,0x%x) = 0x%x", hWnd, uRel, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 047 */
void IO_GetWindowLong(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetWindowLong(hWnd, offset);
   GraphicsDebug(("GetWindowLong(0x%x,%d) = 0x%lx", hWnd, offset, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 048 */
void IO_GetWindowWord(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetWindowWord(hWnd, offset);
   GraphicsDebug(("GetWindowWord(0x%x,%d) = 0x%x", hWnd, offset, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 049 */
void IO_IsChild(Graphnode *app)
{
   HWND hParent = (HWND)mcb->Control[0];
   HWND hChild = (HWND)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)IsChild(hParent, hChild);
   GraphicsDebug(("IsChild(0x%x,0x%x) = 0x%x", hParent, hChild, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 050 */
void IO_IsWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)IsWindow(hWnd);
   GraphicsDebug(("IsWindow(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 051 */
void IO_IsWindowEnabled(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)IsWindowEnabled(hWnd);
   GraphicsDebug(("IsWindowEnabled(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 052 */
void IO_IsWindowVisible(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)IsWindowVisible(hWnd);
   GraphicsDebug(("IsWindowVisible(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 053 */
void IO_SetClassLong(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   LONG value = (LONG)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)SetClassLong(hWnd, offset, value);
   GraphicsDebug(("SetClassLong(0x%x,%d,0x%lx) = 0x%x", hWnd, offset, value, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 054 */
void IO_SetClassWord(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   WORD value = (WORD)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)SetClassWord(hWnd, offset, value);
   GraphicsDebug(("SetClassWord(0x%x,%d,0x%x) = 0x%x", hWnd, offset, value, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 055 */
void IO_SetWindowLong(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   LONG value = (LONG)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)SetWindowLong(hWnd, offset, value);
   GraphicsDebug(("SetWindowLong(0x%x,%d,0x%lx) = 0x%x", hWnd, offset, value, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 056 */
void IO_SetWindowWord(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   WORD value = (WORD)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)SetWindowWord(hWnd, offset, value);
   GraphicsDebug(("SetWindowWord(0x%x,%d,0x%x) = 0x%x", hWnd, offset, value, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 057 */
void IO_SetCapture(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)SetCapture(hWnd);
   GraphicsDebug(("SetCapture(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 058 */
void IO_ReleaseCapture(Graphnode *app)
{
   push_mcb();
   ReleaseCapture();
   GraphicsDebug(("ReleaseCapture() = 0x%x", mcb->Control[0]));
   my_Request_Return(ReplyOK, 0, 0);
}

/* 059 */
void IO_SetCursor(Graphnode *app)
{
   HCURSOR hCursor = (HCURSOR)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)SetCursor(hCursor);
   GraphicsDebug(("SetCursor(0x%x) = 0x%x", hCursor, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 060 */
void IO_LoadCursor(Graphnode *app)
{
   LPCSTR lpszName = (LPCSTR)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = LoadCursor(NULL, lpszName);
   GraphicsDebug(("LoadCursor(NULL, 0x%x) = 0x%x", lpszName, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 061 */
void IO_GetBkColor(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = GetBkColor(hDC);
   GraphicsDebug(("GetBkColor(0x%x) = 0x%08lx", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 062 */
void IO_GetBkMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = GetBkMode(hDC);
   GraphicsDebug(("GetBkMode(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 063 */
void IO_GetDeviceCaps(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int index = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetDeviceCaps(hDC, index);
   GraphicsDebug(("GetDeviceCaps(0x%x,%d) = 0x%x", hDC, index, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 064 */
void IO_GetMapMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = GetMapMode(hDC);
   GraphicsDebug(("GetMapMode(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 065 */
void IO_GetSystemMetrics(Graphnode *app)
{
   int index = (int)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetSystemMetrics(index);
   GraphicsDebug(("GetSystemMetrics(0x%x) = 0x%x", index, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 066 */
void IO_GetTextColor(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = GetTextColor(hDC);
   GraphicsDebug(("GetTextColor(0x%x) = 0x%08lx", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 067 */
void IO_SetBkColor(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   COLORREF cr = (COLORREF)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = SetBkColor(hDC, cr);
   GraphicsDebug(("SetBkColor(0x%x,0x%08lx) = 0x%x", hDC, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 068 */
void IO_SetBkMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int mode = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = SetBkMode(hDC, mode);
   GraphicsDebug(("SetBkMode(0x%x,0x%x) = 0x%x", hDC, mode, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 069 */
void IO_SetMapMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int mode = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = SetMapMode(hDC, mode);
   GraphicsDebug(("SetMapMode(0x%x,0x%x) = 0x%x", hDC, mode, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 070 */
void IO_SetTextColor(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   COLORREF cr = (COLORREF)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = SetTextColor(hDC, cr);
   GraphicsDebug(("SetTextColor(0x%x,0x%08lx) = 0x%x", hDC, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 071 */
void IO_Arc(Graphnode *app)
{
   HDC hDC       = (HDC)mcb->Control[0];
   int nLeft     = (int)mcb->Control[1];
   int nTop      = (int)mcb->Control[2];
   int nRight    = (int)mcb->Control[3];
   int nBottom   = (int)mcb->Control[4];
   int XStartArc = (int)mcb->Control[5];
   int YStartArc = (int)mcb->Control[6];
   int XEndArc   = (int)mcb->Control[7];
   int YEndArc   = (int)mcb->Control[8];
   push_mcb();
   mcb->Control[0] = Arc(hDC, nLeft, nTop, nRight, nBottom,
                         XStartArc, YStartArc, XEndArc, YEndArc);
   GraphicsDebug(("Arc(0x%x,%d,%d,%d,%d,%d,%d,%d,%d) = 0x%x", hDC, nLeft, nTop, 
        nRight, nBottom, XStartArc, YStartArc, XEndArc, YEndArc, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 072 */
void IO_Chord(Graphnode *app)
{
   HDC hDC        = (HDC)mcb->Control[0];
   int nLeft      = (int)mcb->Control[1];
   int nTop       = (int)mcb->Control[2];
   int nRight     = (int)mcb->Control[3];
   int nBottom    = (int)mcb->Control[4];
   int XStartLine = (int)mcb->Control[5];
   int YStartLine = (int)mcb->Control[6];
   int XEndLine   = (int)mcb->Control[7];
   int YEndLine   = (int)mcb->Control[8];
   push_mcb();
   mcb->Control[0] = Chord(hDC, nLeft, nTop, nRight, nBottom,
                           XStartLine, YStartLine, XEndLine, YEndLine);
   GraphicsDebug(("Chord(0x%x,%d,%d,%d,%d,%d,%d,%d,%d) = 0x%x", hDC, nLeft, nTop, 
        nRight, nBottom, XStartLine, YStartLine, XEndLine, YEndLine, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 073 */
void IO_CreateHatchBrush(Graphnode *app)
{
   int style = (int)mcb->Control[0];
   COLORREF cr = (COLORREF)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = CreateHatchBrush(style, cr);
   GraphicsDebug(("CreateHatchBrush(0x%x,0x%08lx) = 0x%x", style, cr, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBRUSH, app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 074 */
void IO_CreatePatternBrush(Graphnode *app)
{
   HBITMAP hBitmap = (HBITMAP)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = CreatePatternBrush(hBitmap);
   GraphicsDebug(("CreatePatternBrush(0x%x) = 0x%x", hBitmap, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBRUSH, app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 075 */
void IO_Ellipse(Graphnode *app)
{
   HDC hDC     = (HDC)mcb->Control[0];
   int nLeft   = (int)mcb->Control[1];
   int nTop    = (int)mcb->Control[2];
   int nRight  = (int)mcb->Control[3];
   int nBottom = (int)mcb->Control[4];
   push_mcb();
   mcb->Control[0] = Ellipse(hDC, nLeft, nTop, nRight, nBottom);
   GraphicsDebug(("Ellipse(0x%x,%d,%d,%d,%d) = 0x%x", hDC, nLeft, nTop, nRight, nBottom, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 076 */
void IO_FloodFill(Graphnode *app)
{
   HDC hDC     = (HDC)mcb->Control[0];
   int X       = (int)mcb->Control[1];
   int Y       = (int)mcb->Control[2];
   COLORREF cr = (COLORREF)mcb->Control[3];
   push_mcb();
   mcb->Control[0] = FloodFill(hDC, X, Y, cr);
   GraphicsDebug(("FloodFill(0x%x,%d,%d,0x%08lx) = 0x%x", hDC, X, Y, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 077 */
void IO_GetCurrentPosition(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetCurrentPosition(hDC);
   GraphicsDebug(("GetCurrentPosition(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 078 */
void IO_GetNearestColor(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   COLORREF cr = (COLORREF)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetNearestColor(hDC, cr);
   GraphicsDebug(("GetNearestColor(0x%x,0x%08lx) = 0x%08lx", hDC, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 079 */
void IO_GetPixel(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int X   = (int)mcb->Control[1];
   int Y   = (int)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)GetPixel(hDC, X, Y);
   GraphicsDebug(("GetPixel(0x%x,%d,%d) = 0x%08lx", hDC, X, Y, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 080 */
void IO_GetPolyFillMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetPolyFillMode(hDC);
   GraphicsDebug(("GetPolyFillMode(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 081 */
void IO_GetROP2(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetROP2(hDC);
   GraphicsDebug(("GetROP2(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 082 */
void IO_InvalidateRect(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   BOOL bErase = (BOOL)mcb->Control[1];
   LPRECT lprc;
   push_mcb();
   if (mcb->MsgHdr.DataSize eq 1)
       lprc = NULL;
   else
       lprc = (LPRECT)&mcb->Data[0];
   InvalidateRect(hWnd, lprc, bErase);
   if (mcb->MsgHdr.DataSize eq 1)
       GraphicsDebug(("InvalidateRect(0x%x,NULL,0x%x) = 0x%x", hWnd, bErase, 
                mcb->Control[0]));
   else
       GraphicsDebug(("InvalidateRect(0x%x,(%d,%d,%d,%d),0x%x) = 0x%x", hWnd, 
                lprc->left, lprc->top, lprc->right, lprc->bottom, bErase, mcb->Control[0]));
   my_Request_Return(ReplyOK, 0, 0);
}

/* 083 */
void IO_InvertRect(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   LPRECT lprc = (LPRECT)&mcb->Data[0];
   push_mcb();
   InvertRect(hWnd, lprc);
   GraphicsDebug(("InvertRect(0x%x,(%d,%d,%d,%d)) = 0x%x", hWnd, 
        lprc->left, lprc->top, lprc->right, lprc->bottom, mcb->Control[0]));
   my_Request_Return(ReplyOK, 0, 0);
}

/* 084 */
void IO_Pie(Graphnode *app)
{
   HDC hDC       = (HDC)mcb->Control[0];
   int nLeft     = (int)mcb->Control[1];
   int nTop      = (int)mcb->Control[2];
   int nRight    = (int)mcb->Control[3];
   int nBottom   = (int)mcb->Control[4];
   int XStartArc = (int)mcb->Control[5];
   int YStartArc = (int)mcb->Control[6];
   int XEndArc   = (int)mcb->Control[7];
   int YEndArc   = (int)mcb->Control[8];
   push_mcb();
   mcb->Control[0] = Pie(hDC, nLeft, nTop, nRight, nBottom,
                         XStartArc, YStartArc, XEndArc, YEndArc);
   GraphicsDebug(("Pie(0x%x,%d,%d,%d,%d,%d,%d,%d,%d) = 0x%x", hDC, nLeft, nTop, 
        nRight, nBottom, XStartArc, YStartArc, XEndArc, YEndArc, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 085 */
void IO_Polygon(Graphnode *app)
{
   HDC     hDC     = (HDC)mcb->Control[0];
   HGLOBAL hBuffer = (HGLOBAL)mcb->Control[1];
   int     count   = (int)mcb->Control[2];
   LPPOINT lppt;

   push_mcb();

   lppt = (LPPOINT)GlobalLock(hBuffer);
   if (lppt eq (LPPOINT)NULL)
   {
        mcb->Control[0] = FALSE;
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }
   mcb->Control[0] = Polygon(hDC, lppt, count);
   GraphicsDebug(("Polygon(0x%x,,%d) = 0x%x", hDC, count, mcb->Control[0]));
   GlobalUnlock(hBuffer);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 086 */
void IO_Polyline(Graphnode *app)
{
   HDC     hDC     = (HDC)mcb->Control[0];
   HGLOBAL hBuffer = (HGLOBAL)mcb->Control[1];
   int     count   = (int)mcb->Control[2];
   LPPOINT lppt;

   push_mcb();

   lppt = (LPPOINT)GlobalLock(hBuffer);
   if (lppt eq (LPPOINT)NULL)
   {
        mcb->Control[0] = FALSE;
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }
   mcb->Control[0] = Polyline(hDC, lppt, count);
   GraphicsDebug(("Polyline(0x%x,,%d) = 0x%x", hDC, count, mcb->Control[0]));
   GlobalUnlock(hBuffer);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 087 */
void IO_Rectangle(Graphnode *app)
{
   HDC hDC      = (HDC)mcb->Control[0];
   int nLeft    = (int)mcb->Control[1];
   int nTop     = (int)mcb->Control[2];
   int nRight   = (int)mcb->Control[3];
   int nBottom  = (int)mcb->Control[4];
   push_mcb();
   mcb->Control[0] = (word)Rectangle(hDC, nLeft, nTop, nRight, nBottom);
   GraphicsDebug(("Rectangle(0x%x,%d,%d,%d,%d) = 0x%x", hDC, nLeft, nTop, 
        nRight, nBottom, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 088 */
void IO_SetPixel(Graphnode *app)
{
   HDC hDC      = (HDC)mcb->Control[0];
   int X        = (int)mcb->Control[1];
   int Y        = (int)mcb->Control[2];
   COLORREF cr  = (COLORREF)mcb->Control[3];
   push_mcb();
   mcb->Control[0] = (word)SetPixel(hDC, X, Y, cr);
   GraphicsDebug(("SetPixel(0x%x,%d,%d,0x%08lx) = 0x%x", hDC, X, Y, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 089 */
void IO_SetPolyFillMode(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int mode = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)SetPolyFillMode(hDC, mode);
   GraphicsDebug(("SetPolyFillMode(0x%x,0x%x) = 0x%x", hDC, mode, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 090 */
void IO_SetROP2(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   int mode = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)SetROP2(hDC, mode);
   GraphicsDebug(("SetROP2(0x%x,0x%x) = 0x%x", hDC, mode, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 091 */
void IO_UnrealizeObject(Graphnode *app)
{
   HGDIOBJ hObj = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)UnrealizeObject(hObj);
   GraphicsDebug(("UnrealizeObject(0x%x) = 0x%x", hObj, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 092 */
void IO_UpdateWindow(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   push_mcb();
   UpdateWindow(hWnd);
   GraphicsDebug(("UpdateWindow(0x%x) = 0x%x", hWnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 0, 0);
}

/* 093 */
void IO_ValidateRect(Graphnode *app)
{
   HWND hWnd = (HWND)mcb->Control[0];
   LPRECT lprc = (LPRECT)&mcb->Data[0];
   push_mcb();
   ValidateRect(hWnd, lprc);
   GraphicsDebug(("ValidateRect(0x%x,(%d,%d,%d,%d)) = 0x%x", hWnd, lprc->left, 
        lprc->top, lprc->right, lprc->bottom, mcb->Control[0]));
   my_Request_Return(ReplyOK, 0, 0);
}

/* 094 */
void IO_CreatePalette(Graphnode *app)
{
   LOGPALETTE FAR *lplgpl = (LOGPALETTE FAR *)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = (word)CreatePalette(lplgpl);
   GraphicsDebug(("CreatePalette(..) = 0x%x", mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HPALETTE, app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 095 */
void IO_GetPaletteEntries(Graphnode *app)
{
   HPALETTE hPal   = (HPALETTE)mcb->Control[0];
   UINT     uStart = (UINT)mcb->Control[1];
   UINT     uNo    = (UINT)mcb->Control[2];
   PALETTEENTRY FAR *lppe = (PALETTEENTRY FAR *)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = GetPaletteEntries(hPal, uStart, uNo, lppe);
   GraphicsDebug(("GetPaletteEntries(0x%x,%d,%d,) = 0x%x", hPal, uStart, uNo, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, sizeof(PALETTEENTRY)*mcb->Control[0]);
}

/* 096 */
void IO_GetNearestPaletteIndex(Graphnode *app)
{
   HPALETTE hPal   = (HPALETTE)mcb->Control[0];
   COLORREF cr     = (COLORREF)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = GetNearestPaletteIndex(hPal, cr);
   GraphicsDebug(("GetNearestPaletteIndex(0x%x,0x%08lx) = 0x%x", hPal, cr, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 097 */
void IO_RealizePalette(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = RealizePalette(hDC);
   GraphicsDebug(("RealizePalette(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 098 */
void IO_SelectPalette(Graphnode *app)
{
   HDC      hDC      = (HDC)mcb->Control[0];
   HPALETTE hPal     = (HPALETTE)mcb->Control[1];
   BOOL     bBackgnd = (BOOL)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = SelectPalette(hDC, hPal, bBackgnd);
   GraphicsDebug(("SelectPalette(0x%x,0x%x,0x%x) = 0x%x", hDC, hPal, bBackgnd, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 099 */
void IO_SetPaletteEntries(Graphnode *app)
{
   HPALETTE hPal   = (HPALETTE)mcb->Control[0];
   UINT     uStart = (UINT)mcb->Control[1];
   UINT     uNo    = (UINT)mcb->Control[2];
   PALETTEENTRY FAR *lppe = (PALETTEENTRY FAR *)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = SetPaletteEntries(hPal, uStart, uNo, lppe);
   GraphicsDebug(("SetPaletteEntries(0x%x,%d,%d,) = 0x%x", hPal, uStart, uNo, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 100 */
void IO_BitBlt(Graphnode *app)
{
   HDC   hDest  = (HDC)mcb->Control[0];
   int   XDest  = (int)mcb->Control[1];
   int   YDest  = (int)mcb->Control[2];
   int   width  = (int)mcb->Control[3];
   int   height = (int)mcb->Control[4];
   HDC   hSrc   = (HDC)mcb->Control[5];
   int   XSrc   = (int)mcb->Control[6];
   int   YSrc   = (int)mcb->Control[7];
   DWORD dwROP  = (DWORD)mcb->Control[8];
   push_mcb();
   mcb->Control[0] = (word)BitBlt(hDest, XDest, YDest, width, height,
                                  hSrc, XSrc, YSrc, dwROP);
   GraphicsDebug(("BitBlt(0x%x,%d,%d,%d,%d,0x%x,%d,%d,0x%lx) = 0x%x", hDest, 
        XDest, YDest, width, height, hSrc, YSrc, XSrc, dwROP, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* 101 */
void IO_CreateBitmap(Graphnode *app)
{
   int     width  = (int)mcb->Control[0];
   int     height = (int)mcb->Control[1];
   UINT    planes = (UINT)mcb->Control[2];
   UINT    bpp    = (UINT)mcb->Control[3];
   HGLOBAL hMem   = (HGLOBAL)mcb->Control[4];
   VOID FAR *lpvBits;

   push_mcb();
   if (hMem ne NULL)
   {
        lpvBits = (VOID FAR *)GlobalLock(hMem);
        if (lpvBits eq (VOID FAR *)NULL)
        {
            mcb->Control[0] = NULL;
            ServerDebug("CreateBitmap failed to access init data");
            my_Request_Return(ReplyOK, 1, 0);
            return;
        }
   }
   else
       lpvBits = NULL;

   mcb->Control[0] = (word)CreateBitmap(width, height, planes, bpp, lpvBits);
   GraphicsDebug(("CreateBitmap(%d,%d,%d,%d,) = 0x%x", width, height, planes, bpp, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBITMAP, app);
   GlobalUnlock(hMem);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 102 */
void IO_CreateCompatibleBitmap(Graphnode *app)
{
   HDC hDC    = (HDC)mcb->Control[0];
   int width  = (int)mcb->Control[1];
   int height = (int)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)CreateCompatibleBitmap(hDC, width, height);
   GraphicsDebug(("CreateCompatibleBitmap(0x%x,%d,%d) = 0x%x", hDC, width, height, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBITMAP, app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 103 */
void IO_CreateDIBitmap(Graphnode *app)
{
   HDC hDC = mcb->Control[0];
   DWORD dwInit = mcb->Control[1];
   UINT ColorUse = mcb->Control[2];
   HGLOBAL hMem = mcb->Control[3];
   BITMAPINFOHEADER FAR *lpbih = (BITMAPINFOHEADER FAR *)&mcb->Data[0];
   BITMAPINFO FAR *lpbi = (BITMAPINFO FAR *)&mcb->Data[lpbih->biSize];
   void FAR *lpvBits = NULL;

   push_mcb();

   if (dwInit != CBM_INIT)
       lpbi = NULL;

   if (hMem != NULL)
   {
       lpvBits = (void FAR *)GlobalLock(hMem);
       if (lpvBits == NULL)
       {
           mcb->Control[0] = NULL;
           ServerDebug("CreateDIBitmap failed to access init data");
           my_Request_Return(ReplyOK, 1, 0);
           return;
       }
   }

   mcb->Control[0] = CreateDIBitmap(hDC, lpbih, dwInit, lpvBits, lpbi, ColorUse);
   GraphicsDebug(("CreateDIBitmap(0x%x,,0x%lx,,,%x) = 0x%x", hDC, dwInit, ColorUse, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HBITMAP, app);
   GlobalUnlock(hMem);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 104 */
void IO_GetBitmapBits(Graphnode *app)
{
   HBITMAP hBitmap  = (HBITMAP)mcb->Control[0];
   LONG    cbBuffer = (LONG)mcb->Control[1];
   HGLOBAL hMem = GlobalAlloc(GHND, cbBuffer);
   void FAR *lpvBits = (void FAR *)GlobalLock(hMem);
   push_mcb();

   if ((hMem == NULL) || (lpvBits == (void FAR *)NULL))
   {
        mcb->Control[0] = NULL;
        mcb->Control[1] = NULL;
        ServerDebug("GetBitmapBits failed to allocate data buffer");
        my_Request_Return(ReplyOK, 2, 0);
        return;
   }

   mcb->Control[0] = GetBitmapBits(hBitmap, cbBuffer, lpvBits);
   GraphicsDebug(("GetBitmapBits(0x%x,%ld,) = %ld", hBitmap, cbBuffer, mcb->Control[0]));
   GlobalUnlock(hMem);
   mcb->Control[1] = hMem;

   my_Request_Return(ReplyOK, 2, 0);
}

/* 105 */
void IO_GetDIBits(Graphnode *app)
{
   HDC     hDC          = (HDC)mcb->Control[0];
   HBITMAP hBitmap      = (HBITMAP)mcb->Control[1];
   UINT    start        = (UINT)mcb->Control[2];
   UINT    lines        = (UINT)mcb->Control[3];
   UINT    fuColorUse   = (UINT)mcb->Control[4];
   BITMAPINFO FAR *lpbi = (BITMAPINFO FAR *)&mcb->Data[0];
   HGLOBAL hMem;
   BITMAP  bm;
   DWORD   dwBitmapSize;
   void FAR *lpvBits;

   push_mcb();

   GetObject(hBitmap, sizeof(BITMAP), (void FAR *)&bm);
   dwBitmapSize = (DWORD)(bm.bmWidthBytes * bm.bmHeight * bm.bmPlanes);
   hMem = GlobalAlloc(GHND, dwBitmapSize);
   lpvBits = (void FAR *)GlobalLock(hMem);
   if ((hMem == NULL) || (lpvBits == (void FAR *)NULL))
   {
       mcb->Control[0] = 0;
       mcb->Control[1] = 0;
       mcb->Control[2] = 0;
       ServerDebug("GetDIBits failed to allocate data buffer");
       my_Request_Return(ReplyOK, 3, 0);
       return;
   }

   mcb->Control[0] = GetDIBits(hDC, hBitmap, start, lines, lpvBits, lpbi, fuColorUse);
   GraphicsDebug(("GetDIBits(0x%x,0x%x,%d,%d,,,0x%x) = %d", hDC, hBitmap, start, lines, fuColorUse, mcb->Control[0]));
   GlobalUnlock(hMem);
   mcb->Control[1] = hMem;
   mcb->Control[2] = dwBitmapSize;

   my_Request_Return(ReplyOK, 3, sizeof(BITMAPINFO));
}

/* 106 */
void IO_SetBitmapBits(Graphnode *app)
{
   HBITMAP hBitmap = (HBITMAP)mcb->Control[0];
   DWORD   dwCount = (DWORD)mcb->Control[1];
   HGLOBAL hMem    = (HGLOBAL)mcb->Control[2];
   void FAR *lpvBits;

   push_mcb();
   lpvBits = (void FAR *)GlobalLock(hMem);
   if (lpvBits eq (void FAR *)NULL)
   {
        mcb->Control[0] = 0;
        ServerDebug("SetBitmapBits: Unable to access bitmap data");
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }
   mcb->Control[0] = SetBitmapBits(hBitmap, dwCount, lpvBits);
   GraphicsDebug(("SetBitmapBits(0x%x,%ld,) = 0x%lx", hBitmap, dwCount, mcb->Control[0]));
   GlobalUnlock(hMem);

   my_Request_Return(ReplyOK, 1, 0);
}

/* 107 */
void IO_SetDIBits(Graphnode *app)
{
   HDC hDC = mcb->Control[0];
   HBITMAP hBitmap = mcb->Control[1];
   UINT start = mcb->Control[2];
   UINT count = mcb->Control[3];
   void FAR *lpvBits;
   BITMAPINFO FAR *lpbi = (BITMAPINFO FAR *)&mcb->Data[0];
   UINT ColorUse = mcb->Control[4];
   HGLOBAL hMem = mcb->Control[5];

   push_mcb();

   lpvBits = (void FAR *)GlobalLock(hMem);
   if (lpvBits == NULL)
   {
       mcb->Control[0] = 0;
       ServerDebug("SetDIBits: Unable to access bitmap data");
       my_Request_Return(ReplyOK, 1, 0);
       return;
   }

   mcb->Control[0] = SetDIBits(hDC, hBitmap, start, count, lpvBits, lpbi, ColorUse);
   GraphicsDebug(("SetDIBits(0x%x,0x%x,%d,%d,,,0x%x) = 0x%lx", hDC, hBitmap, start, count, ColorUse, mcb->Control[0]));
   GlobalUnlock(hMem);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 108 */
void IO_SetDIBitsToDevice(Graphnode *app)
{
    HDC hDC = mcb->Control[0];
    int XDest = mcb->Control[1];
    int YDest = mcb->Control[2];
    int cx = mcb->Control[3];
    int cy = mcb->Control[4];
    int XSrc = mcb->Control[5];
    int YSrc = mcb->Control[6];
    UINT uStart = mcb->Control[7];
    UINT uNo = mcb->Control[8];
    UINT ColorUse = mcb->Control[9];
    HGLOBAL hMem = mcb->Control[10];
    BITMAPINFO FAR *lpbi = (BITMAPINFO FAR *)&mcb->Data[0];
    void FAR *lpvBits = GlobalLock(hMem);

    push_mcb();

    if (lpvBits == NULL)
    {
         mcb->Control[0] = 0;
         my_Request_Return(ReplyOK, 1, 0);
         return;
    }
    mcb->Control[0] = SetDIBitsToDevice(hDC, XDest, YDest, cx, cy,
                                XSrc, YSrc, uStart, uNo, lpvBits, lpbi, ColorUse);
    GraphicsDebug(("SetDIBitsToDevice() = 0x%lx", mcb->Control[0]));
    GlobalUnlock(hMem);
    my_Request_Return(ReplyOK, 1, 0);
}

/* 109 */
void IO_CreateCompatibleDC(Graphnode *app)
{
   HDC hDC = (HDC)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)CreateCompatibleDC(hDC);
   GraphicsDebug(("CreateCompatibleDC(0x%x) = 0x%x", hDC, mcb->Control[0]));
   my_Request_Return(ReplyOK, 1, 0);
}

/* BLV - the code below manipulates Windows buffers which can exceed 64K,
 * i.e. which must be processed using huge pointers. Therefore the usual
 * inline versions of memcpy etc. do not work.
 */
#pragma function(memcpy)

/* 110 */
void IO_RegisterBuffer(Graphnode *app)
{
   DWORD dwSize = mcb->Control[0];

   push_mcb();
   mcb->Control[0] = GlobalAlloc(GHND, dwSize);
   GraphicsDebug(("RegisterBuffer(%ld) = 0x%x", dwSize, mcb->Control[0]));
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HGLOBAL, (void FAR *)app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 111 */
void IO_AppendBuffer(Graphnode *app)
{
   HGLOBAL hMem = (HGLOBAL)mcb->Control[0];
   DWORD offset = (DWORD)mcb->Control[1];
   DWORD size = (DWORD)mcb->Control[2];
   DWORD dwBufSize = GlobalSize(hMem);
   BYTE huge *lpData;
   push_mcb();

   /* check validity of buffer access to prevent protection faults */
   if ((offset < 0L) || (offset+size > dwBufSize))
   {
        mcb->Control[0] = FALSE;
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }

   lpData = (BYTE huge *)GlobalLock(hMem);

   memcpy(&lpData[offset], &mcb->Data[0], (UINT)size);

   GlobalUnlock(hMem);
   mcb->Control[0] = TRUE;
   my_Request_Return(ReplyOK, 1, 0);
}

/* 166 - send back to Helios */
void IO_GetBuffer(Graphnode *app)
{
   HGLOBAL hMem = (HGLOBAL)mcb->Control[0];
   int offset = (int)mcb->Control[1];
   DWORD size = (DWORD)mcb->Control[2];
   DWORD dwBufSize = GlobalSize(hMem);
   BYTE huge *lpData;

   push_mcb();

   /* check validity of buffer access to prevent protection faults */
   if ((offset < 0) || (offset+size > dwBufSize))
   {
        ServerDebug("GetBuffer ERROR: offset and size out of range");
        mcb->Control[0] = FALSE;
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }

   lpData = (BYTE huge *)GlobalLock(hMem);

   memcpy(&mcb->Data[0], &lpData[offset], (UINT)size);

   GlobalUnlock(hMem);
   mcb->Control[0] = size;
   my_Request_Return(ReplyOK, 1, size);
}

/* 112 */
void IO_DeleteBuffer(Graphnode *app)
{
   HGLOBAL hMem = mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (GlobalFree(hMem) ne NULL);
   GraphicsDebug(("DeleteBuffer(0x%x) = 0x%x", hMem, mcb->Control[0]));
   RemoveHandle(&hashWindows, hMem, HT_HGLOBAL);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 113 */
void IO_AppendMenu(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   uFlags   = (UINT)mcb->Control[1];
   UINT   ID       = (UINT)mcb->Control[2];
   LPCSTR lpszText = (LPCSTR)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = (word)AppendMenu(hMenu, uFlags, ID, lpszText);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 114 */
void IO_CheckMenuItem(Graphnode *app)
{
   HMENU hMenu  = (HMENU)mcb->Control[0];
   UINT  ID     = (UINT)mcb->Control[1];
   UINT  uCheck = (UINT)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)CheckMenuItem(hMenu, ID, uCheck);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 115 */
void IO_CreatePopupMenu(Graphnode *app)
{
   push_mcb();
   mcb->Control[0] = (word)CreatePopupMenu();
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HMENU, (void FAR *)app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 116 */
void IO_CreateMenu(Graphnode *app)
{
   push_mcb();
   mcb->Control[0] = (word)CreateMenu();
   if (mcb->Control[0] != NULL)
       AddHandle(&hashWindows, mcb->Control[0], HT_HMENU, (void FAR *)app);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 117 */
void IO_DeleteMenu(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   UINT   uFlags   = (UINT)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)DeleteMenu(hMenu, ID, uFlags);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 118 */
void IO_DestroyMenu(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)DestroyMenu(hMenu);
   RemoveHandle(&hashWindows, hMenu, HT_HMENU);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 119 */
void IO_DrawMenuBar(Graphnode *app)
{
   HWND  hWnd    = (HWND)mcb->Control[0];
   push_mcb();
   DrawMenuBar(hWnd);
   my_Request_Return(ReplyOK, 0, 0);
}

/* 120 */
void IO_EnableMenuItem(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   UINT   uFlags   = (UINT)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)EnableMenuItem(hMenu, ID, uFlags);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 121 */
void IO_GetMenu(Graphnode *app)
{
   HWND  hWnd    = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetMenu(hWnd);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 122 */
void IO_GetMenuItemCount(Graphnode *app)
{
   HMENU  hMenu = (HMENU)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = (word)GetMenuItemCount(hMenu);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 123 */
void IO_GetMenuItemID(Graphnode *app)
{
   HMENU  hMenu = (HMENU)mcb->Control[0];
   int    nPos  = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetMenuItemID(hMenu, nPos);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 124 */
void IO_GetMenuState(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   UINT   uFlags   = (UINT)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)GetMenuState(hMenu, ID, uFlags);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 125 */
void IO_GetMenuString(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   int    max      = (int)mcb->Control[2];
   UINT   uFlags   = (UINT)mcb->Control[3];
   LPSTR  str      = (LPSTR)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = (word)GetMenuString(hMenu, ID, str, max, uFlags);
   my_Request_Return(ReplyOK, 1, mcb->Control[0]+1);
}

/* 126 */
void IO_GetSubMenu(Graphnode *app)
{
   HMENU  hMenu = (HMENU)mcb->Control[0];
   int    nPos  = (int)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetSubMenu(hMenu, nPos);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 127 */
void IO_GetSystemMenu(Graphnode *app)
{
   HWND   hWnd = (HWND)mcb->Control[0];
   BOOL   bRevert = (BOOL)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)GetSystemMenu(hWnd, bRevert);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 128 */
void IO_InsertMenu(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   UINT   uFlags   = (UINT)mcb->Control[2];
   UINT   NewID    = (UINT)mcb->Control[3];
   LPSTR  str      = (LPSTR)&mcb->Data[0];
   push_mcb();
   mcb->Control[0] = (word)InsertMenu(hMenu, ID, uFlags, NewID, str);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 129 */
void IO_RemoveMenu(Graphnode *app)
{
   HMENU  hMenu    = (HMENU)mcb->Control[0];
   UINT   ID       = (UINT)mcb->Control[1];
   UINT   uFlags   = (UINT)mcb->Control[2];
   push_mcb();
   mcb->Control[0] = (word)RemoveMenu(hMenu, ID, uFlags);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 130 */
void IO_SetMenu(Graphnode *app)
{
   HWND   hWnd     = (HWND)mcb->Control[0];
   HMENU  hMenu    = (HMENU)mcb->Control[1];
   push_mcb();
   mcb->Control[0] = (word)SetMenu(hWnd, hMenu);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 131 */
void IO_FindWindow(Graphnode *app)
{
   int    length = (int)mcb->Control[0];
   LPCSTR lpszClassName = (LPCSTR)&mcb->Data[0];
   LPCSTR lpszWindow = (LPCSTR)&mcb->Data[length];
   push_mcb();
   if (strlen(lpszClassName) == 0)
       lpszClassName = NULL;
   if (strlen(lpszWindow ) == 0)
       lpszWindow = NULL;
   mcb->Control[0] = (word)FindWindow(lpszClassName, lpszWindow);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 133 */
void IO_RegisterIOMenu(Graphnode *app)
{
   UINT ID = (UINT)mcb->Control[0];
   Menunode *menunode;
   push_mcb();

   /* now we add the menu to the app's list */
   menunode = (Menunode *)malloc(sizeof(Menunode));
   if (menunode eq (Menunode *)NULL)
   {
        mcb->Control[0] = FALSE;
        my_Request_Return(ReplyOK, 1, 0);
        return;
   }

   menunode->ID = ID;
   AddTail(&(menunode->node), &(app->menu_list));
   mcb->Control[0] = TRUE;
   my_Request_Return(ReplyOK, 1, 0);
}

/* 133 */
void IO_DeleteDC(Graphnode *app)
{
   HDC hDC = (HWND)mcb->Control[0];
   push_mcb();
   mcb->Control[0] = DeleteDC(hDC);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 134 */
void IO_GetUpdateRect(Graphnode *app)
{
   HWND hWnd = mcb->Control[0];
   BOOL bErase = mcb->Control[1];
   Windownode *window = find_window_node(hWnd);
   RECT rect;
   push_mcb();

   if (window != (Windownode *)NULL)
        memcpy(&mcb->Data[0], &window->rectInvalid, sizeof(RECT));
   else
   {
        mcb->Control[0] = GetUpdateRect(hWnd, &rect, bErase);
        memcpy(&mcb->Data[0], &rect, sizeof(RECT));
   }

   my_Request_Return(ReplyOK, 1L, sizeof(RECT));
}

/* 135 */
void IO_SendMessage(Graphnode *app)
{
   HWND hWnd = mcb->Control[0];
   UINT msg  = mcb->Control[1];
   WPARAM wParam = mcb->Control[2];
   LPARAM lParam = mcb->Control[3];
   push_mcb();
   mcb->Control[0] = SendMessage(hWnd, msg, wParam, lParam);
   my_Request_Return(ReplyOK, 1, 0);
}

/* 136 */
void IO_GetObject(Graphnode *app)
{
   HGDIOBJ hObj = mcb->Control[0];
   int     cbBuffer = mcb->Control[1];
   void FAR *lpvBuffer = &mcb->Data[0];
   push_mcb();
   mcb->Control[0] = GetObject(hObj, cbBuffer, lpvBuffer);
   my_Request_Return(ReplyOK, 1, mcb->Control[0]);
}

/* 167 */
void IO_SetScrollRange(Graphnode *app)
{
	HWND hWnd	= (HWND) mcb->Control[0];
	int  nBar	= (int)  mcb->Control[1];
	int  nMinPos	= (int)  mcb->Control[2];
	int  nMaxPos	= (int)  mcb->Control[3];
	int  bRedraw	= (int)  mcb->Control[4];
	push_mcb();
	SetScrollRange(hWnd, nBar, nMinPos, nMaxPos, bRedraw);
	my_Request_Return(ReplyOK, 0, 0);
}

/* 168 */
void IO_GetScrollRange(Graphnode *app)
{
	HWND hWnd	= (HWND) mcb->Control[0];
	int  nBar	= (int)  mcb->Control[1];
	int  nMinPos;
	int  nMaxPos;

	push_mcb();
	GetScrollRange(hWnd, nBar, &nMinPos, &nMaxPos);
	mcb->Control[0]	= (word) nMinPos;
	mcb->Control[1] = (word) nMaxPos;
	my_Request_Return(ReplyOK, 2, 0);	
}

/* 169 */
void IO_SetScrollPos(Graphnode *app)
{
	HWND	hWnd	= (HWND) mcb->Control[0];
	int	nBar	= (int)  mcb->Control[1];
	int	nPos	= (int)  mcb->Control[2];
	int	bRedraw = (int)  mcb->Control[3];

	push_mcb();
	mcb->Control[0]	= SetScrollPos(hWnd, nBar, nPos, bRedraw);
	my_Request_Return(ReplyOK, 1, 0);
}

/* 170 */
void IO_GetScrollPos(Graphnode *app)
{
	HWND	hWnd	= (HWND) mcb->Control[0];
	int	nBar	= (int)  mcb->Control[1];

	push_mcb();
	mcb->Control[0]	= GetScrollPos(hWnd, nBar);
	my_Request_Return(ReplyOK, 1, 0);
}

/* 171 */
void IO_ShowScrollBar(Graphnode *app)
{
	HWND	hWnd	= (HWND) mcb->Control[0];
	WORD	wBar	= (WORD) mcb->Control[1];
	BOOL	bShow	= (WORD) mcb->Control[2];

	push_mcb();
	ShowScrollBar(hWnd, wBar, bShow);
	my_Request_Return(ReplyOK, 0, 0);
}

/* 172 */
void IO_EnableScrollBar(Graphnode *app)
{
	HWND	hWnd		= (HWND) mcb->Control[0];
	WORD	wSBFlags	= (WORD) mcb->Control[1];
	WORD	wArrowFlags	= (WORD) mcb->Control[2];

	push_mcb();
	mcb->Control[0]	= EnableScrollBar(hWnd, wSBFlags, wArrowFlags);
	my_Request_Return(ReplyOK, 0, 0);
}

/* This is the procedure called by MS-WINDOWS to process messages generated */
/* for windows opened by the graphics server. The only messages intercepted */
/* here are the mouse and keyboard messages.                                */

long FAR PASCAL GraphProc(hWnd, iMessage, wParam, lParam)
    HWND     hWnd;
    unsigned iMessage;
    WORD     wParam;
    LONG     lParam;
{
   LPMSG lpMsg;

   switch (iMessage) {
    /* specially handled messages */
      case WM_DESTROY:
        GraphicsDebug(("GraphProc(0x%x,WM_DESTROY,0x%x,0x%lx)", hWnd, wParam, lParam));
        build_message_and_send(hWnd, iMessage, wParam, lParam);
        remove_window_node(hWnd);
        break;

      case WM_PAINT:
      {
        Windownode *window = find_window_node(hWnd);
        GraphicsDebug(("GraphProc(0x%x,WM_PAINT,0x%x,0x%lx)", hWnd, wParam, lParam));
        build_message_and_send(hWnd, iMessage, wParam, lParam);
        InvalidateRect(hWnd, &window->rectInvalid, FALSE);
        GetUpdateRect(hWnd, &window->rectInvalid, FALSE);
        ValidateRect(hWnd, &window->rectInvalid);
        break;
      }

      case WM_SIZE:
        GraphicsDebug(("GraphProc(0x%x,WM_SIZE,0x%x,0x%lx)", hWnd, wParam, lParam));
        build_message_and_send(hWnd, iMessage, wParam, lParam);
        break;

    /* normal messages - are sent to helios with no special handling */
      case WM_ACTIVATE:
      case WM_CHAR:
      case WM_CHILDACTIVATE:
      case WM_CLOSE:
      case WM_CREATE:
      case WM_COMMAND:
      case WM_ENABLE:
      case WM_ENDSESSION:
      case WM_FONTCHANGE:
      case WM_HSCROLL:
      case WM_KILLFOCUS:
      case WM_LBUTTONDBLCLK:
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_MBUTTONDBLCLK:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MOUSEACTIVATE:
      case WM_MOVE:
      case WM_MOUSEMOVE:
      case WM_PALETTECHANGED:
      case WM_PARENTNOTIFY:
      case WM_QUERYNEWPALETTE:
      case WM_RBUTTONDBLCLK:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_SETFOCUS:
      case WM_SHOWWINDOW:
      /* case WM_SIZE: */
      case WM_SYSCHAR:
      case WM_SYSCOLORCHANGE:
      case WM_TIMECHANGE:
      case WM_VSCROLL:
      case WM_WININICHANGE:
        GraphicsDebug(("GraphProc(0x%x,0x%x,0x%x,0x%lx)", hWnd, iMessage, wParam, lParam));
        build_message_and_send(hWnd, iMessage, wParam, lParam);
        break;

    /* all others are handled by default handler */
      default:
        if (iMessage >= WM_USER)
        {
            GraphicsDebug(("GraphProc(0x%x,WM_USER,0x%x,0x%lx)", hWnd, iMessage, wParam, lParam));
            build_message_and_send(hWnd, iMessage, wParam, lParam);
        }
        else
            return(DefWindowProc(hWnd, iMessage, wParam, lParam));
   }
   return 0L;
}


