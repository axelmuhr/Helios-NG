/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                             All Rights Reserved.                     --
--                                                                      --
--  terminal.c                                                          --
--                                                                      --
--           This module contains code to interact with the various     --
--                                                                      --
--           forms of terminal devices.                                 --
--                                                                      --
--  Author:  BLV 19/10/88                                               --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: terminal.c,v 1.23 1994/07/06 10:44:59 mgun Exp $ */
/* Copyright (C) 1988, Perihelion Software Ltd.        			*/

/**
*** The usual header file and some function declarations.
**/

#define Terminal_Module
#include "helios.h"

#if !PC
#if SOLARIS
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdlib.h>
#endif

/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--
--     The console and window devices
--
--------------------------------------------------------------------------
------------------------------------------------------------------------*/

/**
*** These devices are probably the most complicated one that I need to
*** support. There are lots of nasties like converting ANSI escape sequences,
*** the Helios standard, to local ones, or like the user XOFF-ing at the
*** most inconvenient moments.
***
***  Console attributes. Attributes act globally on the
*** entire console device. If sensible windows are available each
*** window has its own set of attributes. Global attributes are
*** necessary to deal with XON/XOFF, i.e. whenever one Stream changes its
*** attributes this affects all the other streams. 
**/

/**
*** initialise_windowing() is called from inside initialise_devices() in
*** module devices.c. Its main job is to set up the ServerWindow.
**/
void initialise_windowing()
{
#if multiple_windows
/**
*** The window_list is initialised here as well as in Window_InitServer(),
*** just in case...
**/
  InitList(&Window_List.list);
  Window_List.entries = 0;

  Server_window.handle = create_a_window("Helios Server");

  if (Server_window.handle eq 0L)
   { printf("Unable to create a window for the Server.\r\n");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  strcpy(&(Server_window.node.direntry.Name[0]), "Helios Server");
  Server_window.node.account          = 10000;
  Server_window.node.direntry.Flags   = 0L;
  Server_window.break_handler.port    = NullPort;
  Server_window.break_handler.ownedby = (word *) NULL;
  Server_window.cooker.count          = 0;
  Server_window.readerco              = (Conode *) NULL;
  Server_window.writerco              = (Conode *) NULL;
  Server_window.head                  = 0;
  Server_window.tail                  = 0;
  Server_window.XOFF                  = 0;
  InitSemaphore(&Server_window.read_lock, 1);
  InitSemaphore(&Server_window.write_lock, 1);
  InitAttributes (&(Server_window.attr));

  AddAttribute   (&(Server_window.attr), ConsoleEcho);
  RemoveAttribute(&(Server_window.attr), ConsoleIgnoreBreak);
  AddAttribute   (&(Server_window.attr), ConsoleBreakInterrupt);
  AddAttribute   (&(Server_window.attr), ConsolePause);
  RemoveAttribute(&(Server_window.attr), ConsoleRawInput);
  RemoveAttribute(&(Server_window.attr), ConsoleRawOutput);

# if ARMBSD
  /* fix to stop just_attach'ings from echoing chars */
  if (get_config("raw_windows") != NULL)
  {
  RemoveAttribute   (&(Server_window.attr), ConsoleEcho);
  RemoveAttribute   (&(Server_window.attr), ConsoleIgnoreBreak);
  RemoveAttribute   (&(Server_window.attr), ConsoleBreakInterrupt);
  RemoveAttribute   (&(Server_window.attr), ConsolePause);
  AddAttribute      (&(Server_window.attr), ConsoleRawInput);
  AddAttribute      (&(Server_window.attr), ConsoleRawOutput);
  }
# endif
#endif

#if use_ANSI_emulator
  { word x, y;

#if multiple_windows
    window_size(Server_window.handle, &x, &y);
#else
#if (PC || ST)
    x = 80; y = 25;
#endif  /* (PC || ST) */
#endif  /* multiple_windows */
    
    if (!Init_Ansi(&Server_window, y, x))
     { printf("Unable to initialise ANSI emulator library for server window.\r\n");
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }
  }
#endif /* use_ANSI_emulator */

  Server_windows_nopop = (get_config("SERVER_WINDOWS_NOPOP") ne NULL);

  restart_windowing();
}

void restore_windowing()
{
#if multiple_windows
  close_window(Server_window.handle);
#endif

#if use_ANSI_emulator
  Tidy_Ansi(&(Server_window.screen));
#endif
}

void restart_windowing()
{
#if multi_tasking

#if multiple_windows
#if SOLARIS
  /* last 0 argument a dummyvalue to keep the C++ compiler happy */
  AddMultiwait(Multi_WindowInput, &(Server_window.any_data),
	       Server_window.handle, 0);
#else
  AddMultiwait(Multi_WindowInput, &(Server_window.any_data),
	       Server_window.handle);
#endif
#else
  AddMultiwait(Multi_WindowInput, &(Server_window.any_data));
#endif

#endif
}


void fn( add_key,          (int, Window *));
PRIVATE void fn( send_break_event, (Port));

void poll_the_windows()
{ int temp;

#if multi_tasking
  if (Server_window.any_data eq CoReady)
   {
     Server_window.any_data = 0;
     while ((temp = read_char_from_keyboard(Server_window.handle) ) ne -1)
      add_key(temp, &Server_window);
   }
#else
  while ((temp = read_char_from_keyboard(Server_window.handle) ) ne -1)
    add_key(temp, &Server_window);
#endif

#if multiple_windows
   { int temp;
     register Window *window;
     for (window = (Window *) Window_List.list.head;
          window->node.node.next ne (Node *) NULL;
          window = (Window *) window->node.node.next )

#if multi_tasking

       if (window->any_data eq CoReady)
        { window->any_data = 0;
          while ((temp = read_char_from_keyboard(window->handle)) ne -1)
           add_key(temp, window);
        }

#else

       while ((temp = read_char_from_keyboard(window->handle)) ne -1)
         add_key(temp, window);

#endif

   }
#endif /* multiple_windows */
}


/**
*** This routine is used to put characters typed at the keyboard into the
*** console input buffer. It takes care of certain special keys, e.g.
*** ctrl-C, ctrl-S and ctrl-Q.
***
*** Adding something to a circular buffer - the something is added to the head
*** of the table, the head is increased modulo the limit, and then there is
*** a check for overflow, i.e. has the entire table been filled. There are
*** two possible approaches here, either forget about the bit of data just
*** added or forget about the oldest bit of data, and different approaches
*** are suitable for different devices. For the console, if somebody has
*** typed 256 characters ahead I prefer to lose the newest bit of data.
**/

void add_key(key, window)
int key;
Window *window;
{ 
  switch(key)
   {
     case 0x03 :        /* ctrl-C */
                        /* if breaks are ignored, ignore ctrl-C */
                if (IsAnAttribute(&(window->attr), ConsoleIgnoreBreak))
                  return;
                else if /* is the event handler active ? */
                 (IsAnAttribute(&(window->attr), ConsoleBreakInterrupt))
                 { if (window->break_handler.port ne (word) NULL)
                     send_break_event(window->break_handler.port);
                   if (window->readerco ne (Conode *) NULL)
                    { (window->readerco)->flags |= CoFlag_CtrlCed;
                      (window->readerco)->type = CoReady;
                    }
		   window->head = 0;	/* keyboard buffer flushed by	*/
		   window->tail = 0;	/* resetting buffer ptrs	*/
		   window->cooker.count = 0;	/* Nothing there now */
		   
                   return;
                 }

                break;  /* otherwise put it in the buffer */

      case 0x11 :       /* ctrl-Q */
                 if (window->XOFF)
                  { window->XOFF = 0;
                    if (window->writerco ne (Conode *) NULL)  /* wake up the cortn */
                     window->writerco->type = CoReady;
                    return;
                  }
                 else
                  break;

      case 0x13 :      /* ctrl-S */
                 if (IsAnAttribute(&(window->attr), ConsolePause))
                  { window->XOFF = 1; return; }
                 else
                  break;
        }

  window->Table[window->head] = (UBYTE) key;
  window->head = (window->head + 1) & Console_limit;
  if (window->head eq window->tail)   /* buffer overflow */
    window->head = (window->head - 1) & Console_limit; 
  if (window->readerco ne (Conode *) NULL)
    (window->readerco)->type = CoReady;
}

PRIVATE int  fn( fill_buffer,  (word, Window *));
PRIVATE void fn(cooked_Console_select, (Conode*, Window*));
PRIVATE void fn(raw_Console_select,  (Conode*, Window*));

/**
*** Console_Select 
*** This routine is called when the client wants to wait
*** until there is something he can do such a reading
*** or writing. In some cases it should return on an 
*** exception which could be the break key
**/

void Console_Select(myco)
Conode *myco;
{
  /* The first thing we need to do is find  */
  /* out what and who we are waiting for    */

  word reply_port = mcb->MsgHdr.Reply;
  word mode       = mcb->MsgHdr.FnRc & Flags_Mode;
  
  Window* window  = (Window*) myco->extra;

  /* check we are doing a read select */

  if ((mode & (O_ReadOnly | O_WriteOnly)) eq O_WriteOnly)
  {
    Request_Return(O_WriteOnly,0L,0L);
    return;
  }  

  mode &= (O_WriteOnly | O_ReadOnly);
  
  window->readerco = myco;

  myco->type = CoReady;

  AddTail(Remove(&(myco->node)), SelectCo);

  if (IsAnAttribute(&(window->attr),ConsoleRawInput))  
  {
    raw_Console_select(myco,window);
  }
  else
  {
    cooked_Console_select(myco,window);
  }

  PostInsert(Remove(&(myco->node)), Heliosnode); 
  window->readerco = NULL;

  if (myco->type eq CoAbortSelect)
         {
           MsgHdr temp;
           memcpy(&temp,&mcb->MsgHdr,sizeof(MsgHdr));
      	
           mcb->MsgHdr.Dest = 0;
           mcb->MsgHdr.Reply = reply_port;
           Request_Return(EC_Error + SS_IOProc + EG_Exception + EE_Abort,0L,0L);
           memcpy(&mcb->MsgHdr,&temp,sizeof(MsgHdr));
         }

   else if (myco->type eq CoReady)
         {
           mcb->MsgHdr.Dest = 0;
           mcb->MsgHdr.Reply = reply_port;
           Request_Return(mode & O_ReadOnly,0L,0L);
         } 

   else if ((myco->type eq CoSuicide)||(myco->type eq CoTimeout))
         {
           (void) Console_TidyStream(myco);
           Seppuku();
         }
}


PRIVATE void cooked_Console_select(myco, window)
Conode *myco;
Window* window;
{ 
  Microwave *cooker = &(window->cooker);
  word  echo        = IsAnAttribute(&(window->attr), ConsoleEcho);
  int  temp =0;
  
  /* Simple case - we have already read a \n */

  for (temp = 0; temp < cooker->count; temp++)
   if (cooker->buffer[temp] eq '\n')
    { 
      myco->type = CoReady; 
      return;
    }

  goto missfillbuffer;

  forever
  { 
    temp = fill_buffer(echo, window);     /* fill_buffer does the editing */

missfillbuffer:

     /* Has ctrl-D | ctrl-C been detected ? */

     if (temp eq Cooked_EOF)
     {
       myco->flags |= CoFlag_EOFed;
     }

     if ((temp eq Cooked_Done) ||
         (myco->flags & CoFlag_EOFed) || 
         (myco->flags & CoFlag_CtrlCed))
     {
       myco->type = CoReady;
       return;
     }

    myco->timelimit = MAXINT;		/* Infinite Timelimit */

    myco->type = 0L;
    Suspend();                          /* wait a bit for more data */


    if ((myco->type eq CoTimeout) ||
        (myco->type eq CoSuicide) ||
        (myco->type eq CoAbortSelect)) return;

    if (myco->flags & CoFlag_CtrlCed) goto missfillbuffer;
  }

}


PRIVATE void raw_Console_select(myco, window)
Conode *myco;
Window* window;
{ 
  Microwave *cooker = &(window->cooker);

  goto misssuspend;

  forever
  {
    myco->timelimit = MAXINT;
    myco->type = 0L;
    Suspend();

misssuspend:


    if ((myco->type eq CoTimeout) ||
        (myco->type eq CoSuicide) ||
        (myco->type eq CoAbortSelect)) return;

    if ((window->head ne window->tail) || (cooker->count > 0))  return;
   
    if (myco->flags & CoFlag_CtrlCed) {
    	 myco->type = CoReady;
         return;
    }
  }
}


/**
*** When a break event has been detected, the console attributes are set such
*** that they are not ignored and are not turned into ordinary keys, and a
*** break_handler has been installed, the routine below is called to send off
*** the event. The event is set up, copied onto itself to take care of
*** swapping and time stamp conversion, and sent off. The FnRc is set to
*** IgnoreLost because I do not care about losing break events - if the user
*** detects that the break event has not worked he will just press the key
*** again.
**/

PRIVATE void send_break_event(port)
Port port;
{ 
  IOEvent *event = (IOEvent *) mcb->Data;

  event->Type    = Event_Break;
  event->Counter = 0;
  event->Stamp   = Now;

  copy_event(event, event);   /* This does all the swapping and conversions */

  mcb->MsgHdr.Reply = port;
  mcb->MsgHdr.Dest  = (word) NULL;
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

  Request_Return(EventRc_IgnoreLost, 0L, (word) sizeof(IOEvent));
}



/**
*** In the absence of multiple windows, there is a single window structure
*** Server_window which is used to store everything. InitServer() has to
*** initialise this structure completely.
**/
void Console_InitServer(myco)
Conode *myco;
{
  Server_window.break_handler.port    = NullPort;
  Server_window.break_handler.ownedby = (word *) NULL;
  Server_window.cooker.count          = 0;
  Server_window.readerco              = (Conode *) NULL;
  Server_window.writerco              = (Conode *) NULL;
  Server_window.head                  = 0;
  Server_window.tail                  = 0;
  Server_window.XOFF                  = 0;
  Server_window.node.account          = 10000;
  Server_window.node.direntry.Flags   = 0L;
  InitSemaphore(&Server_window.read_lock, 1);
  InitSemaphore(&Server_window.write_lock, 1);
  InitAttributes (&(Server_window.attr));
  AddAttribute   (&(Server_window.attr), ConsoleEcho);
  RemoveAttribute(&(Server_window.attr), ConsoleIgnoreBreak);
  AddAttribute   (&(Server_window.attr), ConsoleBreakInterrupt);
  AddAttribute   (&(Server_window.attr), ConsolePause);
  RemoveAttribute(&(Server_window.attr), ConsoleRawInput);
  RemoveAttribute(&(Server_window.attr), ConsoleRawOutput);

# if ARMBSD
  /* fix to stop just_attach'ings from echoing chars */
  if (get_config("raw_console") != NULL)
  {
  RemoveAttribute   (&(Server_window.attr), ConsoleEcho);
  RemoveAttribute   (&(Server_window.attr), ConsoleIgnoreBreak);
  RemoveAttribute   (&(Server_window.attr), ConsoleBreakInterrupt);
  RemoveAttribute   (&(Server_window.attr), ConsolePause);
  AddAttribute      (&(Server_window.attr), ConsoleRawInput);
  AddAttribute      (&(Server_window.attr), ConsoleRawOutput);
  }
# endif

  use(myco)
}

/**
*** Console_Open is responsible for creating a new Stream coroutine, which is
*** always possible if there is enough memory. All the entries in the coroutine
*** list node are set up, and the reply is set up. Flags_Interactive is a
*** hint to the transputer side that data should be read and written in fairly
*** small lumps.
***
*** At any moment in time there are likely to be lots of open console Streams,
*** because every C program including the shell has at least three for STDIN,
*** STDOUT, and STDERR. Hence all Console streams must refer to the same window.
**/

void Console_Open(myco)
Conode *myco;
{                                 /* check the device we are opening */
  if (strcmp(IOname, "console") )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  NewStream(Type_File, Flags_Selectable + Flags_Closeable + Flags_Interactive, (word) &Server_window, 
            Console_Handlers);
  use(myco)
}

#if multiple_windows

/**
*** The window device supports multiple console streams on a single
*** IO processor. It depends on a number of local routines, as follows :
***
*** word create_a_window(char *name) - this should create a new window
*** somewhere on the screen, with the name specified. This name  might appear
*** in the window's title bar, for example, or it might be ignored. It
*** should return a handle of some sort.
***
*** void close_window(handle) - this should destroy the window specified.
*** The handle will be something returned by create_a_window.
***
*** void window_size(handle, word *x, word *y) - this should store the current 
*** window size in the specified variables.
***
*** The Window server routines follow next. They are rather different from
*** the console ones because the window server is a directory, i.e. much
*** more complicated.
**/

/**
*** Test functions used to determine whether or not to support multiple
*** windows.
**/

void Window_Testfun(result)
bool *result;
{ char *temp = get_config("server_windows");
  if (temp ne NULL)
   *result = 1L;
  else
   *result = 0L;
}

void Console_Testfun(result)
bool *result;
{ char *temp = get_config("server_windows");
  if (temp ne NULL)
   *result = 0L;
  else
   *result = 1L;
}

/**
*** Window_InitServer() sets up the linked list of windows, initially Helios
*** has not created any windows so the list is empty. A separate window is
*** created for the Server's output, see initialise_devices() in devices.c .
**/
PRIVATE int Next_WindowID;

void Window_InitServer(myco)
Conode *myco;
{ InitList(&(Window_List.list));
  Window_List.entries = 0L;
  Next_WindowID = 1;
  myco->extra         = (ptr) &Window_List;
  use(myco);
}

/**
*** TidyServer goes through all the windows created by Helios and deletes
*** them all.
**/
void fn( delete_window, (Window *));

void Window_TidyServer(myco)
Conode *myco;
{ 
  WalkList(&(Window_List.list), (VoidNodeFnPtr)delete_window);
 
  use(myco)
}

/**
*** new_window tries to create a new window with the name given, fully
*** initialising the window structure. If it fails it sends a suitable
*** error message to the transputer, otherwise it returns true and
*** leaves it to the caller, Window_Open or Window_Create, to send a
*** suitable reply.
**/
PRIVATE Window *new_window(window_name)
char *window_name;
{ register Window *window = (Window *) malloc(sizeof(Window));
#if use_ANSI_emulator
  word rows, cols;
#endif

  if (window eq (Window *) NULL)
   { Request_Return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
     return(window);
   }

  window->handle = create_a_window(window_name);

  if (window->handle eq 0L)
   { iofree(window);
     Request_Return(EC_Warn + SS_IOProc + EG_Create + EO_Window, 0L, 0L);
     return((Window *) NULL);
   }

  strcpy(&(window->node.direntry.Name[0]), window_name);
  window->node.direntry.Type = Type_File;
  window->node.size          = 0L;
  window->node.account       = 0L;
  window->node.direntry.Flags= 0L;
  window->XOFF   = 0;
  window->head   = 0;
  window->tail   = 0;
  InitAttributes(&(window->attr));
  AddAttribute   (&(window->attr), ConsoleEcho);
  RemoveAttribute(&(window->attr), ConsoleIgnoreBreak);
  AddAttribute   (&(window->attr), ConsoleBreakInterrupt);
  AddAttribute   (&(window->attr), ConsolePause);
  RemoveAttribute(&(window->attr), ConsoleRawInput);
  RemoveAttribute(&(window->attr), ConsoleRawOutput);
  window->break_handler.port    = NullPort;
  window->break_handler.ownedby = 0L;
  window->cooker.count          = 0;
  window->readerco              = (Conode *) NULL;
  window->writerco              = (Conode *) NULL;
  InitSemaphore(&(window->read_lock), 1);
  InitSemaphore(&(window->write_lock), 1);

  Window_List.entries++;
  AddTail(&(window->node.node), &(Window_List.list));

#if use_ANSI_emulator
  window_size(window->handle, &cols, &rows);
  if (!Init_Ansi(window, rows, cols))
   { Remove(&(window->node.node)); 
     close_window(window->handle);
     iofree(window);
     Request_Return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
     return((Window *) NULL);
   }

  if (!real_windows)
   { sprintf(misc_buffer1, "%c1;%2dH%c7m%s%c0m\r\n", 0x009B,
          ((int)cols + 1) - strlen(window_name), 0x009B, window_name, 0x009B);
     window_output(misc_buffer1, window);
   }
#endif

#if multi_tasking
#if SOLARIS
  /* last 0 argument a dummyvalue to keep the C++ compiler happy */
  AddMultiwait(Multi_WindowInput, &(window->any_data), window->handle, 0); 
#else
  AddMultiwait(Multi_WindowInput, &(window->any_data), window->handle); 
#endif
#endif
  return(window);
}

void delete_window(window)
Window *window;
{
#if multi_tasking
#if SOLARIS
  /* last 0 argument a dummyvalue to keep the C++ compiler happy */
  ClearMultiwait(Multi_WindowInput, window->handle, 0);
#else
  ClearMultiwait(Multi_WindowInput, window->handle);
#endif
#endif

  close_window(window->handle);

#if use_ANSI_emulator
  Tidy_Ansi(&(window->screen));
#endif

   iofree(Remove(&(window->node.node)));
   Window_List.entries--;
}

/**
*** An open request for window means that somebody wants to look at the
*** window directory, i.e. find out what windows currently exist. An open
*** request for an existing window means that somebody wants to read from
*** and write to a real window. An Open request for a non-existant window
*** is satisfied iff the Create bit is set in the open mode, and a new
*** window is created automatically.
**/ 
void Window_Open(myco)
Conode *myco;
{ register char   *window_name;
  register Window *window;
  word   openmode = mcb->Control[OpenMode_off];

                                         /* check the device we are opening */
  if (!strcmp(IOname, "window") )        /* Opening the directory ? */
    { NewStream(Type_Directory, Flags_Closeable, (word) &Window_List,  
              WindowDir_Handlers);
      return;
    }

  window = (Window *) Dir_find_node(myco);
  if (window ne (Window *) NULL)
   { NewStream(Type_File, Flags_Closeable + Flags_Interactive + Flags_Selectable,
              (word) window, Console_Handlers);
     return;
   }

  for (window_name = IOname;
       (*window_name ne '/') && (*window_name ne '\0');
       window_name++);
  if (*window_name++ eq '\0')
   { Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_Server, 0L, 0L);
     return;
   }

  if (openmode & O_Create)
   { if ((window = new_window(window_name)) ne (Window *) NULL)
      NewStream(Type_File, Flags_Closeable + Flags_Interactive + Flags_Selectable,
                (word) window, Console_Handlers);
   }
  else
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_Window, 0L, 0L);
}


/**
*** Window_Create may be used to guarantee the existance of a particular
*** window or of the window server, i.e. act just like Locate. Alternatively
*** it may be used to create a new window.
**/
void Window_Create(myco)
Conode *myco;
{ register char *window_name;
  register Window *window;
  word   temp;
         
  if (!strcmp(IOname, "window") )
   { temp = FormOpenReply(Type_Directory, 0L, -1L, -1L); 
     Request_Return(ReplyOK, open_reply, temp);
     return;
   }

             /* Extract the name of the window from IOname */
  for (window_name = IOname; (*window_name ne '/') && (*window_name ne '\0');
       window_name++);
  if (*window_name eq '\0')
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  window_name++;

  window = (Window *) Dir_find_node(myco);
  if (window ne (Window *) NULL)
   { sprintf(&(misc_buffer1[0]), "%s.%d", IOname, Next_WindowID++);
     strcpy(IOname, &(misc_buffer1[0]));
   }

  if (new_window(window_name) ne (Window *) NULL)
   { temp = FormOpenReply(Type_File, 0L, -1L, -1L);
     Request_Return(ReplyOK, open_reply, temp);
   }
  else
   Request_Return(EC_Error + SS_IOProc + EG_Create + EO_Window, 0L, 0L);
}

/**
*** Delete is used to destroy windows. The window server cannot be
*** removed.
**/
void Window_Delete(myco)
Conode *myco;
{ register Window *window;
         
  if (!strcmp(IOname, "window") )
   { Request_Return(EC_Error + SS_IOProc + EG_InUse + EO_Server, 0L, 0L);
     return;
   }

  window = (Window *) Dir_find_node(myco);
  if (window ne (Window *) NULL)
   {
     if (window->node.account ne 0)
      { window->node.direntry.Flags |= WindowFlags_Deleted;
        Request_Return(ReplyOK, 0L, 0L);
/*        Request_Return(EC_Error + SS_IOProc + EG_InUse + EO_Stream, 0L, 0L);*/
        return; 
      }

     delete_window(window);

     Request_Return(ReplyOK, 0L, 0L);
     return;
   }

  Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_Window, 0L, 0L);
}


#endif

/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--
--     console and window streams
--
--------------------------------------------------------------------------
------------------------------------------------------------------------*/

/**
*** InitStream() is called when a new stream is opened. The extra field
*** in the coroutine list node points to the parent window, or to
*** Server_window if multiple windows are not available.
**/

word Console_InitStream(myco)
Conode *myco;
{
#if multiple_windows
  Window *window = (Window *) myco->extra;
  window->node.account++;
#else
  use(myco)
#endif
 
  return(ReplyOK);
}


/**
*** TidyStream() is called when the Server exits or reboots.
**/

word Console_TidyStream(myco)
Conode *myco;
{ Window *window = (Window *) myco->extra;

  if (window->break_handler.ownedby eq (word *) myco)
   { word oldport = window->break_handler.port;
     if (myco->type eq CoTimeout) return(1L);
     window->break_handler.port    = NullPort;
     window->break_handler.ownedby = (word *) NULL;
           /* Clear out the message trail, if any */
     if ((oldport ne (word) NULL) && !(Special_Reboot | Special_Exit))
      {
        mcb->MsgHdr.Dest = (word) NULL;
        mcb->MsgHdr.Reply = oldport;
        Request_Return( EC_Error + SS_Kernel + EG_Exception + EE_Abort, 0L, 0l);
      }
   }

#if multiple_windows
  window->node.account--;
  if (window->node.account eq 0L)
   if ((window->node.direntry.Flags & WindowFlags_Deleted) ne 0L)
    delete_window(window);
#endif

  return(0L);
}

/**
*** Closing a console Stream is relatively easy, because there are no
*** file handles that need closing or anything like that. 
**/

void Console_Close(myco)
Conode *myco;
{
  if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);

  (void) Console_TidyStream(myco);

  Seppuku();
}

/**
*** GetAttributes is responsible for sending off the current global attributes,
*** held in the parent window structure. Care has to be taken to get the
*** window sizes right. CopyAttributes()is used because it does all the
*** required swapping.
**/

void Console_GetAttr(myco)
Conode *myco;
{ Window *window = (Window *) myco->extra;

#if multiple_windows
#if MSWINDOWS
  window->attr.Time = window->screen.Cols;
  window->attr.Min  = window->screen.Rows;
#else
  word x, y;
  window_size(window->handle, &x, &y);
  window->attr.Time = (short) x;
  window->attr.Min  = (short) y;
#endif

#else
#if (ST || PC)
  window->attr.Min  = 80;
  window->attr.Time = 25;
#else
  window->attr.Min  = 80;   /* some default values */
  window->attr.Time = 20;
#endif
#endif  /* multiple_windows */

  CopyAttributes((Attributes *) mcb->Data, &(window->attr));
  Request_Return(ReplyOK, 0L, (word) sizeof(Attributes));
}

/**
*** SetAttributes just takes the new attributes which were part of the message
*** and makes them global. I have to make certain that a window is not left
*** permanently XOFF'ed due to a change of attributes.
**/

void Console_SetAttr(myco)
Conode *myco;
{ Window *window = (Window *) myco->extra;

  CopyAttributes(&(window->attr), (Attributes *) mcb->Data);
            
  if (!IsAnAttribute(&(window->attr), ConsolePause))
   { window->XOFF = 0;
     if (window->writerco ne (Conode *) NULL)
      window->writerco->type = CoReady;
   }

  Request_Return(ReplyOK, 0L, 0L);
}

/**
*** The only event that exists for a console stream is break events. See the
*** chapter on Data and Devices in the technical manual for details of this.
*** There are various things to consider. First of all, the request may be used
*** to disable an existing event handler by masking off the appropriate event,
*** but it is perfectly feasible for the Stream not to have the event handler
*** anymore. If the request does involve setting up an event handler I must
*** release any existing handler, and the structure break_handler contains a
*** pointer to the existing stream which owns the event. Then I set up this
*** stream to own the event handler and the event handler to be owned by this
*** stream. I hope the last sentence made sense. Finally I can reply to the
*** request.
***
*** It is not worth worrying about acknowledgements and negative acknowledge for
*** break events, so they are not supported.
**/

void Console_EnableEvents(myco)
Conode *myco;
{ word mask = mcb->Control[EnableEventsMask_off] & Event_Break;
  event_handler *handler = &(((Window *) myco->extra)->break_handler);
  word oldport = handler->port;
  
  if (mask eq 0L)       /* disable a break handler */
    { if (handler->port ne (word)NULL)    /* is there a break handler ? */
       { handler->port = (word) NULL;
         handler->ownedby = (word *) NULL;
       }
      mcb->Control[Reply1_off] = 0L;
      Request_Return(ReplyOK, 1L, 0L);
    }
  else                  /* install an event handler */
    { handler->port = mcb->MsgHdr.Reply;  /* and install new one */
      handler->ownedby = (word *) myco;
      mcb->Control[Reply1_off] = mask;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      Request_Return(ReplyOK, 1L, 0L);
    }

      /* if there was a previous events port send an AbortPort message */
  if (oldport ne (word) NULL)
   {
     mcb->MsgHdr.Dest  = (word) NULL;
     mcb->MsgHdr.Reply = oldport;
     Request_Return( EC_Error + SS_Kernel + EG_Exception + EE_Abort, 0L, 0l);
   }

}

/**
*** Reading from the keyboard is absolutely lovely. It involves extracting
*** data from the circular buffer where it will have been put by
*** poll_the_devices(), processing it if in cooked mode, and sending it off.
*** I have to be very careful with timeouts.
***
*** To extract data from the circular buffer I use a single routine
*** window_getchar(), which is the opposite of add_key(). The routine
*** returns 0 if there is no character ready, else the character, on the
*** reasoning that it is impossible to type '\0' on any normal keyboard and
*** if it were possible the system would crash anyway.
**/

PRIVATE void fn( raw_Console_read,    (Conode *, int, word, word));
PRIVATE void fn( cooked_Console_read, (Conode *, int, word, word));

              /* extract a single character from the console buffer, if any */
int window_getchar(window)
Window *window;
{ int tempch;

  if (window->head eq window->tail)   /* is there a character in the buffer ? */
     return(0);
  tempch = (int) window->Table[window->tail];
  window->tail = (window->tail + 1) & Console_limit;
  Debug(Keyboard_Flag, ("Read char 0x%x from window %s", tempch, window->node.direntry.Name));
  return(tempch);
}

/**
*** Console_Read() is called when a Read request is received from the
*** transputer. It extracts the read parameters from the message and from the
*** current attributes and calls a suitable routine to deal with the request.
*** This leaves most of the work to be done by raw and cooked_console_read().
**/


void Console_Read(myco)
Conode *myco;
{ word count = (mcb->Control)[ReadSize_off];
  word timeout = (mcb->Control)[ReadTimeout_off];
  word timelimit;
  word reply_port = mcb->MsgHdr.Reply;
  Window *window = (Window *) myco->extra;

  if (count eq 0L)          /* special case */
    { Request_Return(ReadRc_EOD, 0L, 0L);
      return;
    }
               /* I want a sensible limit on the amount read, 1 Kbyte reads */
               /* from a console are silly.                                 */
  if (count > Console_limit) count = Console_limit; 

  if (timeout eq -1L)                        /* And work out the timelimit */
    timelimit = MAXTIME;
  else
    timelimit = Now + divlong( timeout, time_unit);

  myco->timelimit = timelimit;
  Wait(&(window->read_lock));
  if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
   { 
     if (myco->type eq CoTimeout)
      { mcb->MsgHdr.Reply = reply_port;
        mcb->MsgHdr.Dest = NullPort;
        Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Stream, 0L, 0L);
        return;
      }
     else
      { (void) Console_TidyStream(myco); Seppuku(); }
   }

  window->readerco = myco;
     
  if (IsAnAttribute(&(window->attr), ConsoleRawInput))
    raw_Console_read(myco, (int) count, timelimit, reply_port);
  else
    cooked_Console_read(myco, (int) count, timelimit, reply_port);

  window->readerco = (Conode *) NULL;

  Signal(&(window->read_lock));

  if (myco->type eq CoSuicide)
   { (void) Console_TidyStream(myco);
     Seppuku();
   }
}

PRIVATE void fn( send_console_data, (word, word, byte *, word));
PRIVATE int  fn( empty_buffer, (Window *));

/**
*** In raw input mode, I read as many characters as I am asked for
*** or as there are available within the time limit. I perform no
*** editing of any sort, and echo all characters if the echo attribute is set.
**/

PRIVATE void raw_Console_read(myco, count, timelimit, reply_port)
Conode *myco;
int    count;
word   timelimit;
word   reply_port;
{ byte *buff, local_buf[80];
  Window *window  = (Window *) myco->extra;
  word echo       = IsAnAttribute(&(window->attr), ConsoleEcho);
  int  i, temp, ownbuf = 0;

                             /* Work out the buffer where to store the data */
  if (count < 80)
    buff = &(local_buf[0]);
  else
   { buff = (char *)malloc(count);
     if (buff eq (byte *) NULL)
      { Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
        return;
      }
     ownbuf++;
   }
                          /* Usually I want to wait for a key, so I move to */
                          /* PollingCo straight away.                       */
  AddTail(Remove(&(myco->node)), PollingCo);

  for (i = 0; i < count; )   /* extract anything stuck in the microwave */
   if ((temp = empty_buffer(window)) ne 0)
    buff[i++] = (byte) temp;
   else
    break;

  for ( ; i < count; )
     { if ((temp = window_getchar(window)) ne 0)
        { buff[i++] = (byte) temp;
          if (echo)       /* Should I echo the character ? */
            outputch(temp, window);
         }
       else
        { if (myco->flags & CoFlag_CtrlCed) {
            int waitabit;
            count = 0;   /* Braked so ensure flush */
            /* Let break get there 1st */
            for (waitabit=10; waitabit; waitabit--) Suspend();
            break;
          }
          myco->timelimit = timelimit;
          myco->type = 0L;
          Suspend();                       /* No, suspend coroutine for a bit. */

          if (myco->type eq CoSuicide)     /* Check for some special cases. */
           {  if (ownbuf) iofree(buff); return; }
          elif (myco->type eq CoTimeout)          /* Time limit exceeded ? */
           { 
             if (i > 0)                    /* Read any characters at all ? */
              { count = i; break; }
             else                           /* No, so return timeout. */
              { mcb->MsgHdr.Reply = reply_port;
                Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Stream,
                               0L, 0L);
                goto RawRead_end;
              }
           }
        }
     }

  send_console_data(reply_port, ReadRc_EOD, buff, (word) count);

RawRead_end:

  if (ownbuf) iofree(buff);
  PostInsert(Remove(&(myco->node)), Heliosnode); /* Finished, so wait for next request */
}

/**
***   Cooked input is rather more complicated. I am responsible
*** for all input processing, including cursor keys, backspace
*** and delete. Data cannot be sent until a carriage return has
*** been detected, because it might be erased. However, I must
*** echo it anyway so that the user can see what he is typing.
*** This means reading the data into a static buffer until
*** it is ready to be sent. Interleaving cooked reads is safe,
*** but I hate to think what will happen if cooked and raw
*** reads are mixed together.
***
*** As the data is read in and processed I store it in cooked_buffer, and
*** I use cooked_count to keep track of my current position in this buffer.
*** Routine fill_buffer() is responsible for actually getting and processing
*** the keys, returning one of Cooked_EOF, _EOF or _Done. There is a hideous
*** problem in that a particular cooked read may have extracted and processed
*** lots of data without satisfying the request, leaving the data in the
*** buffer. If flag outstanding is set then this is the case. The data still
*** in the buffer may satisfy the current Read request, leaving some data still
*** in the buffer or completely emptying the buffer.
***
*** If the Read has not yet been satisfied by now, I start to extract data from
*** the buffer and process it. If no data is available the coroutine must
*** suspend itself, partly to release the buffer and partly in order to let
*** poll_the_devices() be called because that is the routine which will put
*** more data into the buffer. As for the rest of the code, it is too hideous
*** to describe.
***
*** To handle the EOF condition properly, I must be able to send an EOF to a
*** stream even though it has already received a ReadRc_EOF. There is a conode
*** flag to remember this. 
***
*** If a ctrl-C has been typed while processing the cooked read, the read must
*** abort with EOF. It is assumed that there will be only one cooked read
*** happening in a window at any one time, so the coroutine can store a pointer
*** in the window structure. When a ctrl-C is detected the event handler zaps
*** a flag in the conode structure, which is detected next time I poll.
**/


PRIVATE void cooked_Console_read(myco, amount, timelimit, port)
Conode *myco;
int    amount;
word   timelimit;
word   port;
{ byte *buff, local_buf[80];
  Window    *window = (Window *) myco->extra;
  Microwave *cooker = &(window->cooker);
  word  echo        = IsAnAttribute(&(window->attr), ConsoleEcho);
  int  temp, ownbuf =0;

                /* is the stream trying to read past ctrl-D */
                             /* Work out the buffer where to store the data */
  if (amount < 80)
    buff = &(local_buf[0]);
  else
   { buff = (char *)malloc(amount);
     if (buff eq (byte *) NULL)
      { Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
        return;
      }
     ownbuf++;
   }


  for (temp = 0; temp < cooker->count; temp++)
   if (cooker->buffer[temp] eq '\n')
    { 
      temp = Cooked_Done;
      if (amount > temp) amount = temp + 1;    
      goto junkgoto;	  /* Sorry can't do it any other way */
    }

  if (myco->flags & CoFlag_CtrlCed)
  {
    cooker->count = 0;		/* AMS - Flush anything that got through */
    temp = Cooked_EOF;
    goto junkgoto;
  }

  if (myco->flags & CoFlag_EOFed)
  {
    if (cooker->count <= 0)
    { 
      Request_Return(ReadRc_EOF, 0L, 0L);
      if (ownbuf) iofree(buff);	
      myco->flags &= ~CoFlag_EOFed;  /* AMS - Reset EOF when nothing sent */
      return;
    }
    else	/* select may have seen the ^D */
    {
      temp = Cooked_EOF;
      goto junkgoto;
    }
  }
                          /* Usually I want to wait for a key, so I move to */
                          /* PollingCo straight away.                       */
  AddTail(Remove(&(myco->node)), PollingCo);

  forever
    { temp = fill_buffer(echo, window);     /* fill_buffer does the editing */

junkgoto:
       /* Has ctrl-D | ctrl-C been detected ? */

      if ((temp eq Cooked_EOF) || (temp eq Cooked_Done))
      {  int alex = cooker->count;   
         if (amount > cooker->count) amount = cooker->count;
         memcpy(buff, &(cooker->buffer[0]), amount);
         if (myco->flags & CoFlag_CtrlCed) {
           /* Wait till you think break signal delivered to process */
           {int sometime;
            for (sometime=10; sometime; sometime--)
               Suspend();   
           }
           /* Send empty data and reset Ctrl-C flag */ 
           send_console_data(port,ReadRc_EOD, buff, (word) amount);
           myco->flags &= ~CoFlag_CtrlCed ;
         } else send_console_data(port, (temp eq Cooked_EOF) ? ReadRc_EOF : ReadRc_EOD,
                                  buff, (word) amount);
#if SM90
#define memmove memcpy
#endif

         if ((cooker->count -= amount) > 0)
            memmove(&(cooker->buffer[0]), &(cooker->buffer[amount]), cooker->count);
	    
         /* AMS - Only set if data sent as must come round again, send	*/
         /* nothing and be reset in the code above (b4 this loop)  	*/
         if (alex && (temp eq Cooked_EOF))  myco->flags |= CoFlag_EOFed;

         break;
       }

      myco->timelimit = timelimit;
      myco->type      = 0L;
      Suspend();                                /* wait a bit for more data */

      if (myco->type eq CoSuicide)
        { if (ownbuf) iofree(buff); return; }

      if (myco->flags & CoFlag_CtrlCed)
       { temp = Cooked_EOF; goto junkgoto; }   /* Yukk !! */

      if (myco->type eq CoTimeout)        /* Has time limit been exceeded ? */
       { mcb->MsgHdr.Reply = port;
         Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Stream,
                        0L, 0L);
         break;
       }
                         /* Data is not yet ready, but there is still time. */
    }
  if (ownbuf) iofree(buff);
  PostInsert(Remove(&(myco->node)), Heliosnode); /* Finished, so wait for next request */
}


PRIVATE int empty_buffer(window)
Window *window;
{ Microwave *cooker = &(window->cooker);
  int result;
  UBYTE *buf = &(cooker->buffer[0]);
  register  int i;

  if (cooker->count <= 0) return(0);
  result = buf[0];
  cooker->count--;
  for (i = 0; i <= cooker->count; i++)
    buf[i] = buf[i+1];
  return(result);
}
  
           /* Routine fill_buffer takes data from the main console circular */
           /* buffer and performs local editing.                            */
PRIVATE int fill_buffer(echo, window)
word echo;
Window *window;
{ register int curr_ch;
  Microwave *cooker = &(window->cooker);
  UBYTE *buffer = (UBYTE *) &(cooker->buffer[0]);
  register int  count   = cooker->count;

  while ((curr_ch = window_getchar(window)) ne 0)
    switch(curr_ch)
      { case 0x0A :    /* treat carriage return and newline the same way... */
        case 0x0D : if (count eq Console_limit)
                     { if (echo)
                        window_output("\b\b  \b\b", window);
                       count--;
                     }
                    buffer[count++] = '\n';
                    if (echo) window_output("\r\n", window);
                    cooker->count = count;
                    return(Cooked_Done);

        case 0x04 : 
                    cooker->count = count;
                    return(Cooked_EOF);

        case 0x08 : if (count)              /* backspace if possible */
                     { count--;
                       if (echo)
                         window_output("\b \b", window);    /* do the erase */
                     }
                   break;

        case 0x7F : count = 0;             /* erase line */
                    if (echo)
                     { PRIVATE UBYTE clearline[4] =
                       { '\r', 0x9B, 0x4B, '\0' };
                       window_output((byte *)clearline, window);
                     }
                    break;

                   /* worry about the cursor keys later */

        default : if (isprint(curr_ch))
                   { if (count eq Console_limit) 
                      { if (echo)
                         window_output("\b \b", window);
                        count--;
                      }
                     buffer[count++] = (char) curr_ch;
                     if (echo) outputch(curr_ch, window);
                   }
      }

  if ((window->readerco)->flags & CoFlag_CtrlCed) count = 0;  /* Flush Locally */
  cooker->count = count;
  return(Cooked_EOD);        /* no more characters in the buffer, so return */
}

/**
*** Write data to the console.
***
***  This is ridiculously complicated. The first problem is that there
*** are two types of write requests, single writes and multiple writes.
*** Console_Write takes care of this bit of the protocol, sending its
*** data to routine write_out. The routine also takes care of XON/XOFF,
*** and hence it is responsible for all the timing.
***
***  Data arrives at write_out() in fairly small lumps, so that the
*** server is not locked up when handling large writes to the console :
*** Console_write() suspends itself between lumps. There would be another
*** problem here if I allowed multiple messages, but I ensure that I only
*** ever get one data message. I just hope that nobody tries to write
*** more than 64K to the screen at any one time.
***
***  Writeout() checks which output mode the console is in at the moment
*** and performs raw/cooked editing. The data is then output via routine
*** output(). Output() is used elsewhere to write data to the screen.
*** There is another routine outputch() to output a single character, and
*** this is used mainly for echoing. 
***
***  Incidentally, the system would get extremely confused if anybody
***tried to output '\0', or if writes were interleaved in the wrong way.
**/

#if (AMIGA || UNIX || MSWINDOWS)
#define CONSOLE_WRITE_SIZE 128L
#else
#define CONSOLE_WRITE_SIZE 40L
#endif

PRIVATE void fn( write_out, (UBYTE *, word, word, Window *));
PRIVATE void fn( write_co,  (Conode *));

PRIVATE Window *inherit_window;   /* data to be passed between coroutines */
PRIVATE UBYTE  *inherit_buf;
PRIVATE word   inherit_amount;
PRIVATE word   inherit_mode;
 
void Console_Write(myco)          /* handle a write to screen request */
Conode *myco;
{ Window *window = (Window *) myco->extra;
  word timeout = mcb->Control[WriteTimeout_off];
  word timelimit;
  word count   = mcb->Control[WriteSize_off];  /* amount of data to be written */
  word replyport = mcb->MsgHdr.Reply;
  int  newbuf=0;
  UBYTE *buff, *a_ptr, local_buf[80];
  Conode *writer = (Conode *) NULL;
  int   data_buffered = 0;

  if (count eq 0L)                    /* special case */
   { mcb->Control[Reply1_off] = 0L;
     Request_Return(WriteRc_Done, 1L, 0L);
     return;
   }

  if (count > 0x7FFFL)   /* silly */
   { Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }

  if (timeout eq -1L)
     timelimit = MAXTIME;
  else
     timelimit = Now + divlong( timeout, time_unit);

  if (count < 80)   /* check that I can buffer the data */
   buff = &(local_buf[0]);
  else
   { buff = (UBYTE *) malloc((uint) count);
     if (buff eq (UBYTE *) NULL)
      { ServerDebug("WARNING : I/O Server out of memory on write to screen.");
        Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
        return;
      }
     newbuf++;
   }

          /* if the data has arrived already, I must buffer it immediately */
          /* because another message may arrive whilst Wait'ing */
  if (mcb->MsgHdr.DataSize ne 0L)
   { memcpy(buff, mcb->Data, mcb->MsgHdr.DataSize);
     data_buffered = 1;
   }

  myco->timelimit = timelimit;
  Wait(&(window->write_lock));  /* get exclusive write access to the window */
  if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
   {
     if (myco->type eq CoSuicide)
      { (void) Console_TidyStream(myco); Seppuku(); }
     else
      { mcb->MsgHdr.Reply = replyport;
        Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Stream, 0L, 0L);
        return;
      }
    }

   unless(data_buffered)       /* the data was not sent with the write */
    { word fetched   = 0L;
      
            /* The data is read in in lumps of upto MaxData, */
            /* and put into the same buffer. Once the buffer */
            /* has been filled I start printing it.          */

      mcb->Control[Reply1_off] = (count > maxdata) ? maxdata : count;
      mcb->Control[Reply2_off] = maxdata;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve; /* must preserve the route here */
      mcb->MsgHdr.Reply = replyport;
      Request_Return(WriteRc_Sizes, 2L, 0L);

      a_ptr = buff;

      while ((fetched < count) && (timelimit > Now))
       { myco->timelimit = timelimit;
         Suspend();                         /* Wait for the data to be sent */
         if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
          { if (newbuf) iofree(buff);
            Signal(&(window->write_lock));
            (void) Console_TidyStream(myco);
            Seppuku();
          }
         fetched   += (word) mcb->MsgHdr.DataSize;
         memcpy(a_ptr, mcb->Data, mcb->MsgHdr.DataSize);
         a_ptr     += mcb->MsgHdr.DataSize;           /* next bit of buffer */
         timelimit = Now + divlong(timeout, time_unit);     /* reset timer */
       }

      if (fetched < count)   /* some data must have got lost */
       { mcb->MsgHdr.Reply = replyport;
         Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Message, 0L, 0L);
         if (newbuf) iofree(buff);
         Signal(&(window->write_lock));
         return;
       }
    }

        /* When I get here, all the data has arrived from the */
        /* transputer. I need to display count  characters    */
        /* starting at location buff.                         */
        /* The write may have to be done by a separate coroutine */
        /* which is created here. Once the coroutine has been */
        /* created it picks up its arguments from some statics*/
        /* and puts them on its stack.                        */
        /* For small writes this is not worthwhile, unless the*/
        /* window has been Xoff'ed                            */

  if ((count > CONSOLE_WRITE_SIZE) || window->XOFF)
   {
	/* At this point the data must always be buffered externally,	*/
	/* not on the stack, as the new coroutine may not be able	*/
	/* to access this stack.					*/
     if (!newbuf)
      { buff = (UBYTE *) malloc((uint) count);
        if (buff eq (UBYTE *) NULL)
         { ServerDebug("WARNING : I/O Server out of memory on write to screen.");
           Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
           return;
         }
        newbuf++;
        memcpy(buff, local_buf, (int) count);
      }

     writer = NewCo(write_co);

     if (writer eq (Conode *) NULL)
      { mcb->MsgHdr.Reply = replyport;
        ServerDebug("WARNING : I/O Server out of memory on write to screen.");
        Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
        if (newbuf) iofree(buff);
        Signal(&(window->write_lock));
        return;
      }

     AddTail(&(writer->node), PollingCo);
     writer->id        = -1;
     writer->timelimit = MAXINT;
     writer->name[0]   = '\0';
     writer->flags     = 0L;
     writer->type      = 0L;
 
     inherit_window = window;
     inherit_mode   = IsAnAttribute(&(window->attr), ConsoleRawOutput);
     inherit_amount = count;
     inherit_buf    = buff;
   }

        /* The Write() can now be acknowledged.               */
   mcb->MsgHdr.Reply = replyport;
   mcb->Control[Reply1_off] = count;
   Request_Return(WriteRc_Done, 1L, 0L);

  if (writer ne (Conode *) NULL)
    {
      StartCo(writer);   /* let the coroutine get on with the write */
                         /* and accept the next request.            */
    }
  else
   {
     write_out(buff, count, IsAnAttribute(&(window->attr), ConsoleRawOutput), window);
     if (newbuf) iofree(buff);
     Signal(&(window->write_lock));
   }
}

PRIVATE void write_co(myco)
Conode *myco;
{ UBYTE   *buf      = inherit_buf, *a_ptr;
  Window *window = inherit_window;
  word   amount  = inherit_amount;
  word   rawmode = inherit_mode;
  word   temp;

  a_ptr = buf;

  window->node.account++;   /* make sure the window is not destroyed */
  window->writerco = myco;

  while (amount > 0)
   { while (window->XOFF)
      { myco->type = 0L;
        Suspend();   /* wait for XON or terminate */
        if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
         goto done; 
      }

     temp = (amount > CONSOLE_WRITE_SIZE) ? CONSOLE_WRITE_SIZE : amount;

     write_out(a_ptr, temp, rawmode, window);

     a_ptr += temp; amount -= temp;

                   /* Suspend for a bit, to let the rest of the Server run */
     Multi_nowait++;
     myco->type = CoReady;
     Suspend();
     Multi_nowait--;
     if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
       {
	 goto done;
       }
  }
done:

  iofree(buf);
  window->writerco = (Conode *) NULL;
  window->node.account--;
  Signal(&(window->write_lock));

  Seppuku();
}

    /* Routine write_out() takes a fairly small amount of data, processes */
    /* it as per the current output mode (raw/cooked), and outputs it.    */
PRIVATE void write_out(data, amount, rawmode, window)
UBYTE *data;
word amount, rawmode;
Window *window;
{ register byte *newdata = (byte *) &(misc_buffer1[0]);

  if (rawmode)
    while (amount--) *newdata++ = *data++;
  else
    { 
      while (amount--)
       { 
#if 0
         if (((32 <= *data) && (*data <= 255)) ||    /* printable chars */
             (( 0x07 <= *data) && (*data <= 0x0D)))  /* usual control chars */
#endif
         *newdata++ = *data;
         if (*data eq '\n') *newdata++ = '\r';
         data++;
       }
    }

  *newdata = '\0';
  window_output(&(misc_buffer1[0]), window);
}



/**
*** This is a little routine to send back a reply to a console read request.
*** The console routines usually involve polling, so the message buffer will
*** be corrupted.
**/

PRIVATE void send_console_data(port, fnrc, buffer, amount)
word port, fnrc, amount;
byte *buffer;
{ byte *tempptr = mcb->Data;
  mcb->MsgHdr.Reply = port;
  mcb->MsgHdr.Dest  = NullPort;     /* Seems to be needed, not sure why */
  mcb->Data = buffer;

  Request_Return(fnrc, 0L, amount); 
  mcb->Data = tempptr;
}

            /* Output a single character, used mainly for echoing */
            /* I want to go via output(), to handle ANSI-VT52     */
            /* conversions etc, even though it seems a little bit */
            /* silly.                                             */
void outputch(ch, window)
int ch;
Window *window;
{ byte temp[4];
  temp[0] = (byte) ch; temp[1] = '\0';
  window_output(&(temp[0]), window);
}

char *lhex_out(dest, val, width)
char *dest;
UWORD val;
int width;
{ int i, j, k;

  if ((val eq 0L) && (width eq -1)) { *dest++ = '0'; return(dest); }

  if (width eq -1)
   { for (i = 7; i > 0; i--)   /* Work out most significant nibble */
     if (((val >> (i << 2)) & 0x0F) ne 0)
      break;
   }
  else
   i = width - 1;

  for (j = 0; j <= i; j++)
   { k = (int) ((val >> ((i - j) << 2)) & 0x0F);
     if (k > 9)
      *dest++ = (char) ('a' + k - 0x0a);
     else
      *dest++ = (char) ('0' + k);
   }

  return (dest);     
}

char *lint_out(dest, val)
char *	dest;
word 	val;
{
#ifdef __cplusplus
  UWORD tab[10];
#else
  static UWORD tab[10];
#endif

  int i, k;

  tab[0] = 1000000000L;
  tab[1] = 100000000L;
  tab[2] = 10000000L;
  tab[3] = 1000000L;
  tab[4] = 100000L;
  tab[5] = 10000L;
  tab[6] = 1000L;
  tab[7] = 100L;
  tab[8] = 10L;
  tab[9] = 1L;

  if (val eq 0L) { *dest++ = '0'; return(dest); } 
  if (val < 0L)
   { *dest++ = '-';
     if (val eq -1L)
      { *dest++ = '1'; return(dest); }
#if PC
     val = labs(val);
#else
     val = -1L * val;
#endif
   }

  for (i = 0; val < tab[i]; i++);

  for ( ; i < 10; i++)
   { k = 0;
     while (val >= tab[i]) { val -= tab[i]; k++; }
     *dest++ = (char) ('0' + k);
   }

  return(dest);
}

int ANSI_Out_Set = 0;

#if (ANSI_prototypes)

void ServerDebug(char *text, ...)
{ register char *dest = &(err_buff[0]);
  int  longs, width;
  va_list args;

  va_start(args, text);

  if (ANSI_Out_Set == 0)
    {
      printf (text, args);
      printf ("\n");
      return;
    }

  for ( ; *text ne '\0'; text++)
   if (*text ne '%')
    *dest++ = *text;
   else
    { longs = 0; width = -1;
      again:
      switch (*++text)
       { case '%' : *dest++ = '%'; break;
         case 'l' :
         case 'L' : longs = 1; goto again;
         case '0' : case '1' : case '2' : case '3' : case '4' :
         case '5' : case '6' : case '7' : case '8' : case '9' :
                    if (width eq -1)
                     width = *text - '0';
                    else
                     width = (10 * width) + *text - '0';
                    goto again;
         case 'c' :
         case 'C' : *dest++ = (char) va_arg(args, int); break;
         case 'p' :
         case 'P' : longs = 1;
         case 'x' :
         case 'X' : if (longs)
                       dest = lhex_out(dest, va_arg(args, UWORD), width);
                    else
                     dest = lhex_out(dest,(UWORD) va_arg(args, uint), width);
                    break;
         case 'd' :
         case 'D' : if (longs)
                       dest = lint_out(dest, va_arg(args, word));
                    else
                     dest = lint_out(dest, (word) va_arg(args, int)); 
                    break;
         case 's' :
         case 'S' : { char *dat = va_arg(args, char *);
                      int i = 0;
                      for ( ; (*dat ne '\0') && (i < 80);i++ )
                       *dest++ = *dat++;
                      for ( ; i < width; i++)
                       *dest++ = ' ';
                      break;
                    }
         case 'q' :
         case 'Q' : goto done;
       }
    }

  *dest++ = '\r'; *dest++ = '\n';

done: 
  *dest = '\0';

  if ((log_dest eq Log_to_screen) || (log_dest eq Log_to_both))
    output(err_buff);
  if ((log_dest eq Log_to_file) || (log_dest eq Log_to_both))
    write_to_log(err_buff);

  va_end(args);
}

#else

#if (SUN4 || TR5)
           /* The usual hack for variable arguments does not work on the Sun 4 */
           /* with its register stack. This does                               */
void ServerDebug(text, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9)
char *text;
int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
{ int mytable[10];
  register char *dest = &(err_buff[0]);
  int      longs, width;
  int      *args_ptr = mytable;

  mytable[0] = x0; mytable[1] = x1; mytable[2] = x2; mytable[3] = x3;
  mytable[4] = x4; mytable[5] = x5; mytable[6] = x6; mytable[7] = x7;
  mytable[8] = x8; mytable[9] = x9; 

  if (ANSI_Out_Set == 0)
    {
      printf (text, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);
      printf ("\n");
      return;
    }

#else

void ServerDebug(text, x)
char *text;
int x;
{ register char *dest = &(err_buff[0]);
  int  longs, width;
  int  *args_ptr = &x;

  if (ANSI_Out_Set == 0)
    {
      printf (text, args_ptr);
      printf ("\n");
      return;
    }

#endif

  for ( ; *text ne '\0'; text++)
   if (*text ne '%')
    *dest++ = *text;
   else
    { longs = 0; width = -1;
      again:
      switch (*++text)
       { case '%' : *dest++ = '%'; break;
         case 'l' :
         case 'L' : longs = 1; goto again;
         case '0' : case '1' : case '2' : case '3' : case '4' :
         case '5' : case '6' : case '7' : case '8' : case '9' :
                    if (width eq -1)
                     width = *text - '0';
                    else
                     width = (10 * width) + *text - '0';
                    goto again;
         case 'c' :
         case 'C' : *dest++ = (char) (*args_ptr++); break;
         case 'p' :
         case 'P' : longs = 1;
         case 'x' :
         case 'X' : if (longs)
                     { UWORD *tmpptr = (UWORD *) args_ptr;
                       dest = lhex_out(dest, *tmpptr++, width);
                       args_ptr = (int *) tmpptr;
                     }
                    else
                     dest = lhex_out(dest,(UWORD) (*args_ptr++), width);
                    break;
         case 'u' :
         case 'U' :
         case 'd' :
         case 'D' : if (longs)
                     { word *tmpptr = (word *) args_ptr;
                       dest = lint_out(dest, *tmpptr++);
                       args_ptr = (int *) tmpptr;
                     }
                    else
                     dest = lint_out(dest, (word) (*args_ptr++));
                    break;
         case 's' :
         case 'S' : { char **str_ptr = (char **) args_ptr;
                      char *dat;
                      int i;
                      dat = *str_ptr++;
                      args_ptr = (int *) str_ptr;
                      for ( i = 0; (*dat ne '\0') && (i < 80); i++ )
                       *dest++ = *dat++;
                      for ( ; i < width; i++)
                       *dest++ = ' ';
                      break;
                    }
         case 'q' :
         case 'Q' : goto done;
       }
    }

  *dest++ = '\r'; *dest++ = '\n';

done: 
  *dest = '\0';

  if ((log_dest eq Log_to_screen) || (log_dest eq Log_to_both))
    output(err_buff);
  if ((log_dest eq Log_to_file) || (log_dest eq Log_to_both))
    write_to_log(err_buff);
}

#endif  /* ANSI_prototypes */

/**---------------------------------------------------------------------------
***
*** That was the general code for the window and console devices. The code
*** below is an ANSI screen emulator. Most of the code is used to analyse
*** the input data, and maintain a shadow screen. Then there are a few
*** routines which actually have to do the work on the real screen.
*** I choose to have separate sets of routines for the different devices,
*** so these sets should probably be moved to the local modules at some
*** stage.
***
***---------------------------------------------------------------------------
**/

#if use_ANSI_emulator

/**
*** A general purpose ANSI terminal emulator, which can be easily modified
*** to deal with particular terminals. The first problem is whether or not
*** the terminal wraps. The global variable may be set by the local routines
*** if necessary. Wrapping is very difficult to cope with. On every line
*** except the last, a wrap is dealt with simply by moving back to the
*** appropriate position, inefficient but it works. On the last line it is
*** not possible to draw the character without an automatic scroll, so the
*** character is added to the map but not to the screen and the ANSI_dirty
*** flag is set. This flag should be inspected by the local linefeed routine
*** which may have to redraw the entire bottom line before scrolling. On the
*** ST and the PC I have enough control to stop the terminal from wrapping.
*** Hence the only case where this is important at present is Unix.
**/
int terminal_wraps = 0;

#define Mode_Plain      0
#define Mode_Bold       0x0001
#define Mode_Italic     0x0002
#define Mode_Underline  0x0004
#define Mode_Inverse    0x0008

/**
*** Here are the routines corresponding to the ANSI sequences.
**/
PRIVATE void fn( add_ch,            (int));
PRIVATE void fn( insert_chars,      (int));
PRIVATE void fn( cursor_up,         (int));
PRIVATE void fn( cursor_down,       (int));
PRIVATE void fn( cursor_right,      (int));
PRIVATE void fn( cursor_left,       (int));
PRIVATE void fn( cursor_next,       (int));
PRIVATE void fn( cursor_prev,       (int));
PRIVATE void fn( move_cursor,       (int, int));
PRIVATE void fn( erase_endofscreen, (void));
PRIVATE void fn( erase_endofline,   (void));
PRIVATE void fn( insert_line,       (void));
PRIVATE void fn( delete_line,       (void));
PRIVATE void fn( delete_char,       (int));
PRIVATE void fn( scroll_up,         (int));
PRIVATE void fn( scroll_down,       (int));
PRIVATE void fn( set_rendition,     (Screen *));

/**
*** These are the functions which have to do the work. They are described below.
**/
#if (MAC)
PUBLIC void fn( clear_screen,      (void));
PUBLIC void fn( clear_eol,         (void));
PUBLIC void fn( move_to,           (int, int));
PUBLIC void fn( refresh_screen,    (int, int));
PUBLIC void fn( backspace,         (void));
PUBLIC void fn( carriage_return,   (void));
PUBLIC void fn( linefeed,          (void));
PUBLIC void fn( set_mark,          (void));
PUBLIC void fn( use_mark,          (void));
PUBLIC void fn( foreground,        (int));
PUBLIC void fn( background,        (int));
PUBLIC void fn( set_bold,          (int));
PUBLIC void fn( set_italic,        (int));
PUBLIC void fn( set_underline,     (int));
PUBLIC void fn( set_inverse,       (int));
PUBLIC void fn( ring_bell,         (void));
#else
#if (!MSWINDOWS)
PRIVATE void fn( clear_screen,      (void));
PRIVATE void fn( clear_eol,         (void));
PRIVATE void fn( move_to,           (int, int));
PRIVATE void fn( refresh_screen,    (int, int));
PRIVATE void fn( backspace,         (void));
PRIVATE void fn( carriage_return,   (void));
PRIVATE void fn( linefeed,          (void));
PRIVATE void fn( set_mark,          (void));
PRIVATE void fn( use_mark,          (void));
PRIVATE void fn( foreground,        (int));
PRIVATE void fn( background,        (int));
PRIVATE void fn( set_bold,          (int));
PRIVATE void fn( set_italic,        (int));
PRIVATE void fn( set_underline,     (int));
PRIVATE void fn( set_inverse,       (int));
PRIVATE void fn( ring_bell,         (void));
#endif
#endif
/**
*** These routines are responsible for actually sending characters to the output
*** device. They may be defined as macros.
**/
#ifndef send_ch
#ifdef __cplusplus
extern "C"
{
#endif
void fn( send_ch, (unsigned char));
#ifdef __cplusplus
}
#endif
#endif
#ifndef flush
void fn( flush, (void));
#endif

/**
*** Init_Ansi() should be called during the initialisation process. Note that
*** it clears the screen, so that I know exactly what is on the screen when I
*** start up.
**/

PRIVATE void fn( reset_escape, (Screen *));

int ANSI_OutSet = 0;

word Init_Ansi(window, No_Rows, No_Cols)
Window *window;
word No_Rows, No_Cols;
{ int i;
  byte **map, *whole_map;
  Screen *screen = &(window->screen);

  if ((map = (byte **) malloc((int) No_Rows * sizeof(byte *)))
           eq (byte **) NULL)
    {
      return(false);
    }

  if ((whole_map = (byte *) malloc((int) No_Rows * (int) No_Cols))
                eq (byte *) NULL)
   { 
     iofree(map); 
     return(false);
   }

  memset(whole_map, ' ', (int) No_Rows * (int) No_Cols);

  map[0] = whole_map; 
  for (i = 1; i < (int) No_Rows; i++) 
   map[i] = map[i-1] + (int) No_Cols;

  screen->map          = map;

  screen->whole_screen = whole_map;
  screen->Rows         = (int) No_Rows;
  screen->Cols         = (int) No_Cols;
  screen->mode         = Mode_Plain;

  reset_escape(screen);

  { static char cls[2] = { 0x0C, 0x00 };
    ANSI_out(&(cls[0]), window);
  }

  ANSI_Out_Set++;

  return(true);
}

void Tidy_Ansi(screen)
Screen *screen;
{ 
  iofree(screen->whole_screen);
  iofree(screen->map);
}

/**
*** This resets a window's escape processing variables.
**/
PRIVATE void reset_escape(screen)
Screen *screen;
{ screen->flags &= ~(ANSI_escape_found | ANSI_in_escape);
  screen->flags |= ANSI_firstdigit;
  screen->args[0] = 0; screen->args[1] = 0; screen->args[2] = 0;
  screen->args[3] = 0; screen->args[4] = 0;
  screen->args_ptr = &(screen->args[0]); screen->gotdigits = 0;
}

/**
*** When I get text for a particular window, I always put the relevant
*** variables into statics rather than access them via the window pointer.
*** This should speed things up a bit.
**/
#if (MAC)
PUBLIC int    Cur_x, Cur_y, LastRow, LastCol, Screen_mode;
PUBLIC byte   **map;
PUBLIC Window *current_window;
PUBLIC Screen *current_screen;
PUBLIC word   handle;
#else
#if (MSWINDOWS)
int    Cur_x, Cur_y, LastRow, LastCol, Screen_mode;
byte   **map;
Window *current_window;
Screen *current_screen;
word   handle;
#else
PRIVATE int    Cur_x, Cur_y, LastRow, LastCol, Screen_mode;
PRIVATE byte   **map;
PRIVATE Window *current_window;
PRIVATE Screen *current_screen;
PRIVATE word   handle;
#endif
#endif
/**
*** This is a general-purpose window resizing routine for real windowing systems.
*** It should only be called when there is no other output going on, typically when
*** getting keyboard data from a window, and definitely not in an interrupt routine.
*** There is no attempt to recover if there is not enough memory, and there is no
*** checking for silly window sizes.
**/

void fn( redraw_screen, (Window *, word));

void Resize_Ansi(window, handle, rows, cols)
Window *window;
word handle, rows, cols;
{ byte **old_map, **new_map, *old_screen, *new_screen;
  Screen *screen = &(window->screen);
  int i, cols_to_copy, rows_to_copy;

  new_map = (byte **) malloc((int) rows * sizeof(byte *));
  if (new_map eq (byte **) NULL) return;
  new_screen = (byte *) malloc((int) rows * (int) cols);
  if (new_screen eq (byte *) NULL)
   { iofree(new_map); return; }
  new_map[0] = new_screen;
  for (i = 1; i < (int) rows; i++)
   new_map[i] = new_map[i-1] + (int) cols;

  memset(new_screen, ' ', (int) rows * (int) cols);

  old_map = screen->map;
  old_screen = screen->whole_screen;

  rows_to_copy = (int) (rows > screen->Rows) ? screen->Rows : (int) rows;
  cols_to_copy = (int) (cols > screen->Cols) ? screen->Cols : (int) cols;

  for (i = 0; i < rows_to_copy; i++)
   memcpy(new_map[i], old_map[i], cols_to_copy);

  iofree(old_map); iofree(old_screen);

  screen->map = new_map;
  screen->whole_screen = new_screen;
  screen->Rows = (int) rows;
  screen->Cols = (int) cols;
  if (screen->Cur_x >= (int) cols) screen->Cur_x = (int) cols - 1;
  if (screen->Cur_y >= (int) rows) screen->Cur_y = (int) rows - 1;
  redraw_screen(window, handle);
}


void ANSI_out(itsstr, window)
char *itsstr;
Window *window;
{ register Screen *screen = &(window->screen);
  UBYTE    *str  = (UBYTE *) itsstr;
  Cur_x          = screen->Cur_x;
  Cur_y          = screen->Cur_y;
  map            = screen->map;
  LastRow        = screen->Rows - 1;
  LastCol        = screen->Cols - 1;
  Screen_mode    = screen->mode;
  current_window = window;
  current_screen = screen;
  handle         = window->handle;

  for ( ; *str ne '\0'; str++)
   { if (screen->flags & ANSI_in_escape)
       {
	 switch(*str)
	 {        /* If it is a digit, it is part of the current arg */
         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
           *(screen->args_ptr) = (10 * *(screen->args_ptr)) + (*str - '0');
           if (screen->flags & ANSI_firstdigit)
             { screen->gotdigits++; screen->flags &= ~ANSI_firstdigit; }
           break;

                /* A semicolon marks the end of the current arg. */
	 case ';':  screen->args_ptr++; screen->flags |= ANSI_firstdigit; break;

	 case '@':  insert_chars((screen->gotdigits) ? screen->args[0] : 1);
	            goto end;

	 case 'A':  cursor_up((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'B':  cursor_down((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'C':  cursor_right((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'D':  cursor_left((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'E':  cursor_next((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'F':  cursor_prev((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'H':  move_cursor( (screen->gotdigits > 0) ? (screen->args[0] - 1) : 0,
                         (screen->gotdigits > 1) ? (screen->args[1] - 1) : 0);
                goto end;

	 case 'J':  erase_endofscreen(); goto end;

	 case 'K':  erase_endofline(); goto end;

	 case 'L':  insert_line(); goto end;

	 case 'M':  delete_line(); goto end;

	 case 'P':  delete_char((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'S':  scroll_up((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'T':  scroll_down((screen->gotdigits) ? screen->args[0] : 1);
                goto end;

	 case 'm':  set_rendition(screen);
	   default :

end:
	     reset_escape(screen);
	 }
       }
     else   /* Not in escape sequence... yet */
      {
	if ((screen->flags & ANSI_escape_found) && (*str eq '['))
         { screen->flags |= ANSI_in_escape;
           screen->flags &= ~ANSI_escape_found;
           continue;
         }

        screen->flags &= ~ANSI_escape_found;

#if TR5
        switch((int)*str)
#else
        switch(*str)
#endif
        {
	case 0x01 :  carriage_return(); linefeed(); break;
         
                /* Line feed only : record terminator should have */
                /* expanded already. */
	/* case 0xa */
	case '\n' :  linefeed(); break;

                /* Single carriage return */
        /* case 0xd */
	case '\r' :  carriage_return(); break;

                /* Bell */
	case 0x07 :  ring_bell(); break;

                /* Form Feed */
	case 0x0C :  clear_screen(); break;

                /* Backspace */
	case 0x08 :  backspace(); break;

               /* Vertical tab */
	case 0x0B :  cursor_up(1); break;

               /* CSI */
/*           case 0x009B : This is not a byte ???? */ 
/*           case 0xFF9B : */
	case 0x9B:
 			 screen->flags |= ANSI_in_escape;
                         break;

               /* possibly part of CSI */
	case 0x1B : screen->flags |= ANSI_escape_found; break;

              /* Horizontal tab */
	case 0x09 : { int i = 8 - (Cur_x % 8);
                         if ((Cur_x + i) > LastCol)
                          { carriage_return(); linefeed(); }
                         else
                          move_to(Cur_y, Cur_x + i);
                         break;
		    }

              /* Something else - is it printable ? */
	  default    :
	    if (*str >= ' ')
	      {
		add_ch(*str);
	      }
	}
      }
   }

  flush();
  screen->Cur_x = Cur_x;
  screen->Cur_y = Cur_y;
  screen->mode  = Screen_mode;
}

/**
***
**/

PRIVATE void insert_chars(count)
int count;
{ register int i;

  if ((Cur_x + count) > LastCol)  /* wipe to eol */
   { clear_eol();
     for (i = Cur_x; i <= LastCol; i++)
      map[Cur_y][i] = ' ';
     return;
   }

  for (i = LastCol; i >= (Cur_x + count); i--)
    map[Cur_y][i] = map[Cur_y][i - count];
  for (i = 0; i < count; i++)
    map[Cur_y][Cur_x+i] = ' ';

  set_mark();

  for (i = 0; i < count; i++)
    send_ch(' ');

  if (terminal_wraps && (Cur_y eq LastRow))
   { for (i = Cur_x; i < (LastCol - count); i++)
      send_ch(map[Cur_y][i]);
     current_screen->flags |= ANSI_dirty;
   }
  else
   for (i = Cur_x; i < (LastCol + 1 - count); i++)
    send_ch(map[Cur_y][i]);

  use_mark();
}

PRIVATE void cursor_up(count)
int count;
{ Cur_y -= count;
  if (Cur_y < 0) Cur_y = 0;
  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_down(count)
int count;
{ 
  Cur_y += count;
  if (Cur_y > LastRow) Cur_y = LastRow;

  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_right(count)
int count;
{
  Cur_x += count;
  if (Cur_x > LastCol) Cur_x = LastCol;

  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_left(count)
int count;
{ Cur_x -= count;
  if (Cur_x < 0) Cur_x = 0;

  if (Cur_x eq 0)
   carriage_return();
  else
   move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_next(count)
int count;
{ register int i;
  carriage_return();
  for (i = 0; i < count; i++) linefeed();
}

PRIVATE void cursor_prev(count)
int count;
{ Cur_y -= count; Cur_x = 0;
  if (Cur_y < 0) Cur_y = 0;
  move_to(Cur_y, Cur_x);
}

PRIVATE void move_cursor(y, x)
int y,x;
{ if (x < 0) x = 0;
  if (x > LastCol) x = LastCol;
  if (y < 0) y = 0;
  if (y > LastRow) y = LastRow;
  move_to(y, x);
}

PRIVATE void erase_endofscreen()
{ register int i, j;

  set_mark();

  for (i = Cur_y; i <= LastRow; i++)
   for (j = (i eq Cur_y) ? Cur_x : 0 ; j <= LastCol; j++)
    map[i][j] = ' ';

  if ((Cur_x eq 0) && (Cur_y eq 0))
    clear_screen();
  else
    refresh_screen(Cur_y, LastRow);

  use_mark();
}

PRIVATE void erase_endofline()
{ register int i;

  for (i = Cur_x; i <= LastCol; i++)
   map[Cur_y][i] = ' ';

  clear_eol();
}

PRIVATE void insert_line()
{ register int i;
  register byte *petr = map[LastRow];

  set_mark();

  for (i = LastRow; i > Cur_y; i--)
   map[i] = map[i-1];
  map[Cur_y] = petr;
  for (i = 0; i <= LastCol; i++) petr[i] = ' ';  
    
  refresh_screen(Cur_y, LastRow);  

  use_mark();
}

PRIVATE void delete_line()
{ register byte *petr = map[Cur_y];
  register int i;

  set_mark();
  
  for (i = Cur_y; i < LastRow; i++)
   map[i] = map[i+1];

  map[LastRow] = petr;

  for (i = 0; i <= LastCol; i++) petr[i] = ' ';

  refresh_screen(Cur_y, LastRow);

  use_mark();
}

PRIVATE void delete_char(count)
int count;
{ register int i;
  register char ch;

  set_mark();

  if ((Cur_x + count) > (LastCol + 1) ) count = LastCol + 1 - Cur_x;
  
  for (i = Cur_x; i < (LastCol + 1 - count); i++)
   { ch = map[Cur_y][i + count];
     send_ch(ch); map[Cur_y][i] = ch;
   }

  if (terminal_wraps && (Cur_y eq LastRow))
   { for (i = 0; i < (count - 1); i++)
      { send_ch(' '); map[LastRow][LastCol + i - count] = ' '; }
     map[LastRow][LastCol] = ' ';
         /* no need for dirty : it is blank anyway */
     current_screen->flags &= ~ANSI_dirty;
   }
  else   
   for (i = 0; i < count; i++)
    { send_ch(' '); map[Cur_y][LastCol - i] = ' '; }
   
  use_mark();
}

PRIVATE void scroll_up(count)
int count;
{ int i;

  set_mark();
  if (count > LastRow) { clear_screen(); use_mark(); return; }

  move_to(LastRow, 0);
  for (i = 0; i < count; i++) linefeed();
  use_mark();
}

PRIVATE void scroll_down(count)
int count;
{ int i;
  register int j;
  register byte *tempptr;
  
  set_mark();
  
  if (count > LastRow)
   clear_screen();
  else
   { for (i = 0; i < count; i++)
      { tempptr = map[LastRow];
        for (j = LastRow; j > 0; j--)
          map[j] = map[j-1];
        map[0] = tempptr;
      }
      
     for (i = 0; i < count; i++)
       for (j = 0; j <= LastCol; j++)
         map[i][j] = ' ';
     refresh_screen(0, LastRow);
   }
   
  use_mark();
}

PRIVATE void set_rendition(screen)
Screen *screen;
{ int i;

  /* check for the case of the missing argument and default to 0 */

  if (screen->gotdigits == 0) 
  {
    screen->gotdigits = 1;
    screen->args[0] = 0;
  }

  for (i = 0; i < screen->gotdigits; i++)
   { if ( (screen->args[i] >= 30) && (screen->args[i] <= 37))
      foreground(screen->args[i] - 30);
     elif ( (screen->args[i] >= 40) && (screen->args[i] <= 47))
      background(screen->args[i] - 30);
     elif (screen->args[i] eq 7)
      { if (!(Screen_mode & Mode_Inverse))
         { set_inverse(1); Screen_mode |= Mode_Inverse; }
      }
     elif (screen->args[i] eq 4)
      { if (!(Screen_mode & Mode_Underline))
         { set_underline(1); Screen_mode |= Mode_Underline; }
      }
     elif (screen->args[i] eq 3)
      { if (!(Screen_mode & Mode_Italic))
         { set_italic(1); Screen_mode |= Mode_Italic; }
      }
     elif (screen->args[i] eq 1)
      { if (!(Screen_mode & Mode_Bold))
         { set_bold(1); Screen_mode |= Mode_Bold; }
      }
     elif (screen->args[i] eq 0)
      { if (Screen_mode & Mode_Bold)
         { set_bold(0); Screen_mode &= ~Mode_Bold; }
        if (Screen_mode & Mode_Italic)
         { set_italic(0); Screen_mode &= ~Mode_Italic; }
        if (Screen_mode & Mode_Underline)
         { set_underline(0); Screen_mode &= ~Mode_Underline; }
        if (Screen_mode & Mode_Inverse)
         { set_inverse(0); Screen_mode &= ~Mode_Inverse; }
      }
   }
}

PRIVATE void add_ch(ch)
int ch;
{ if (terminal_wraps && (Cur_x eq LastCol))
   { if (Cur_y eq LastRow)
      { map[Cur_y][Cur_x] = (byte) ch;
        current_screen->flags |= ANSI_dirty;
        Cur_x++;
        return;
      }
     map[Cur_y][Cur_x] = (byte) ch;
     send_ch(ch);
     move_to(Cur_y, LastCol);
     Cur_x++;
     return; 
   }

  if (Cur_x > LastCol)     /* Implicit wrap-around */
   { carriage_return(); linefeed(); }

  map[Cur_y][Cur_x] = (byte) ch; send_ch(ch); Cur_x++;
}

/**
*** Here are the terminal-dependant routines. I start with some routines for
*** the PC, which are slightly unusual because everything is done in terms of
*** the video bios calls. Then I supply a version for the ST. Some of the
*** routines have to do work on the shadow screen as well as on the real
*** screen. However, before doing anything to the real screen I call
*** check_window(), which determines whether or not the relevant window is
*** actually active.
**/

#if (PC && !MSWINDOWS)
PUBLIC void fn( vbios_cls,        (void));
PUBLIC void fn( vbios_movecursor, (int, int));
PUBLIC void fn( vbios_outputch,   (int));
PUBLIC void fn( vbios_scroll,     (void));
PUBLIC void fn( vbios_bell,       (void));
PUBLIC int  fn( check_window,     (word));

/**
*** This routine should clear the entire screen and move the cursor to the
*** top-left corner.
**/

PRIVATE void clear_screen()
{ register int i, j;

  for (i = 0; i <= LastRow; i++)
   for (j = 0; j <= LastCol; j++)
    map[i][j] = ' ';
  Cur_x = 0; Cur_y = 0;

  if (check_window(handle))
      vbios_cls();
}

/**
*** This routine should clear the rest of the current line, including the
*** cursor position, leaving the cursor position unchanged.
**/
PRIVATE void clear_eol()
{
  if (check_window(handle))
    { int i;
      for (i = Cur_x; i <= LastCol; i++) vbios_outputch(' ');
      vbios_movecursor(Cur_y, Cur_x);
    }
}

/**
*** This routine should move the cursor to the position specified.
**/

PRIVATE void move_to(y, x)
int y, x;
{ Cur_x = x; Cur_y = y;

   if (check_window(handle))
     vbios_movecursor(Cur_y, Cur_x);
}

/**
*** This routine should refresh the screen from the top left, leaving
*** the cursor position unchanged.
**/

PRIVATE void refresh_screen(start_y, end_y)
{ register int i, j;

   if (!check_window(handle)) return;

   vbios_movecursor(start_y, 0);
   
   for (i = start_y; i <= end_y; i++)
    { 
      for ( j = 0; j <= LastCol; j++)
        vbios_outputch(map[i][j]);

      if (i < LastRow)
       vbios_movecursor(i+1, 0);
    }  
}

/**
*** This routine should cause an audio or visual alert of some sort.
**/
PRIVATE void ring_bell()
{
  if (check_window(handle))
    vbios_bell();  
}

/**
*** This routine should move the cursor left one position, without erasing the
*** character.
**/
PRIVATE void backspace()
{ if (Cur_x eq 0) return;
  Cur_x --;

  if (check_window(handle))
    vbios_movecursor(Cur_y, Cur_x);
}

/**
*** This routine should move the cursor to the first column of the current row.
**/

PRIVATE void carriage_return()
{ Cur_x = 0;

  if (check_window(handle))
    vbios_movecursor(Cur_y, Cur_x);
}

/**
*** This routine should move the cursor down one row. If the cursor is on the
*** bottom row the screen should scroll up one line, and the actual cursor
*** position will remain unchanged. The cursor should NOT move to the beginning
*** of the line.
**/

PRIVATE void linefeed()
{
  if (Cur_y >= LastRow)
   { register int i;
     register byte *ptr = map[0]; /* Scroll up */
     for (i = 0; i < LastRow; i++)
      map[i] = map[i+1];
     for (i = 0; i <= LastCol; i++)
      ptr[i] = ' ';
     map[LastRow] = ptr;
     Cur_y = LastRow;

     if (check_window(handle))
      { vbios_scroll();
        vbios_movecursor(Cur_y, Cur_x);
      }
     return;
   }
  else
   { Cur_y++;

     if (check_window(handle))
      vbios_movecursor(Cur_y, Cur_x);
   }
}

/**
*** set_mark() and use_mark() are used to remember and restore the current
*** screen position. Many terminal emulators provide a built-in facility
*** for this.
**/

PRIVATE int mark_x, mark_y;

PRIVATE void set_mark()
{
   mark_x = Cur_x; mark_y = Cur_y;
}

PRIVATE void use_mark()
{
  move_to(mark_y, mark_x);
}

/**
*** All the different attributes.
**/
extern byte vbios_attr;

PRIVATE void foreground(colour)
int colour;
{ colour = colour;
}

PRIVATE void background(colour)
int colour;
{ colour = colour;
}

PRIVATE void set_inverse(flag)
int flag;
{ if (check_window(handle))
   { if (flag)
      vbios_attr = 0x70;
     else
      vbios_attr = 0x07;
   }
}

PRIVATE void set_bold(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_underline(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_italic(flag)
int flag;
{ flag = flag;
}

/**
*** These routines are responsible for sending converted terminal output to
*** the screen, by one means or another. If there are multiple windows, I
*** always buffer the data and send it in one lump using send_to_window().
*** Otherwise, on the PC and the ST send_ch is #define'd to be an
*** immediate operation so there is no need to worry about flush().
**/

#if ANSI_prototypes
#ifdef __cplusplus
extern "C"
{
#endif
void send_ch (unsigned char ch)
#ifdef __cplusplus
}
#endif
#else
void send_ch(ch)
int ch;
#endif
{
  if (check_window(handle))
    vbios_outputch(ch);
}

void flush()
{
}

/**
*** The following code is used to switch screens.
**/
void redraw_screen(window, han)
Window *window;
word han;
{ Screen *screen = &(window->screen);
  char *text = &(window->node.direntry.Name[0]);
  register byte **map = screen->map;
  int i, j, k;

  handle = window->handle;

  vbios_cls();

  for (i = 0; i <= LastRow; i++)
   { for (j = LastCol; j >= 0; j--)
       if (map[i][j] ne ' ') break;
     for (k = 0; k <= j; k++)
      vbios_outputch(map[i][k]);

     if (i < LastRow)
      vbios_movecursor(i + 1, 0);
   }

  vbios_movecursor(0, LastCol + 1 - strlen(text));
  set_inverse(1);
  for ( ; *text ne '\0'; text++)
   vbios_outputch(*text);
  set_inverse(0);
  vbios_movecursor(screen->Cur_y, screen->Cur_x);
  use(han)
}

#endif /* PC specific routines */


#if ST
/**
*** This routine should clear the entire screen and move the cursor to the
*** top-left corner.
**/

PRIVATE void clear_screen()
{ register int i, j;
  for (i = 0; i <= LastRow; i++)
   for (j = 0; j <= LastCol; j++)
    map[i][j] = ' ';
  Cur_x = 0; Cur_y = 0;

  if (check_window(handle))
   { Bconout(2, 0x1B); Bconout(2, 'E'); }
}

/**
*** This routine should clear the rest of the current line, including the
*** cursor position, leaving the cursor position unchanged.
**/
PRIVATE void clear_eol()
{ if (check_window(handle))
   { Bconout(2, 0x1B); Bconout(2, 'K'); }
}

/**
*** This routine should move the cursor to the position specified.
**/

PRIVATE void move_to(y, x)
int y, x;
{ Cur_y = y; Cur_x = x;

  if (check_window(handle))
   { Bconout(2, 0x1B); Bconout(2, 'Y'); Bconout(2, 32+y); Bconout(2, 32+x); }
}

/**
*** This routine should refresh the screen from the top left, leaving
*** the cursor position unchanged.
**/

PRIVATE void refresh_screen(start_y, end_y)
{ register int i, j;

   if (!check_window(handle)) return;

   Bconout(2, 0x1B); Bconout(2, 'f');    /* Disable cursor */
   Bconout(2, 0x1B); Bconout(2, 'Y'); Bconout(2 ,start_y + 32);
   Bconout(2, 32);
   
   for (i = start_y; i <= end_y; i++)
    { 
      for ( j = 0; j <= LastCol; j++)
        Bconout(2, map[i][j]);

      if (i < LastRow)
       { Bconout(2, 0x1B); Bconout(2, 'Y'); Bconout(2, 32 + i + 1);
         Bconout(2, 32);
       }
    }  

  Bconout(2, 0x1B); Bconout(2, 'e');       /* Enable cursor */
}

/**
*** This routine should cause an audio or visual alert of some sort.
**/
PRIVATE void ring_bell()
{ PRIVATE byte beep_sound[] =
{ 0x00, 0x34, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00,
  0x06, 0x00, 0x07, 0xFE, 0x08, 0x10, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00,
  0x0C, 0x10, 0x0D, 0x09, 0xFF, 0x00
};

  if (check_window(handle))
   Dosound(beep_sound);
}

/**
*** This routine should move the cursor left one position, without erasing the
*** character.
**/
PRIVATE void backspace()
{ if (Cur_x eq 0) return;
  Cur_x --;
  if (check_window(handle))
    Bconout(2, 0x08);
}

/**
*** This routine should move the cursor to the first column of the current row.
**/

PRIVATE void carriage_return()
{ Cur_x = 0;
  if (check_window(handle))
    Bconout(2, 0x0D);
}

/**
*** This routine should move the cursor down one row. If the cursor is on the
*** bottom row the screen should scroll up one line, and the actual cursor
*** position will remain unchanged. The cursor should NOT move to the beginning
*** of the line.
**/

PRIVATE void linefeed()
{ 
  if (Cur_y >= LastRow)
   { register int i;
     register byte *ptr = map[0]; /* Scroll up */
     for (i = 0; i < LastRow; i++)
      map[i] = map[i+1];
     for (i = 0; i <= LastCol; i++)
      ptr[i] = ' ';
     map[LastRow] = ptr;
     Cur_y = LastRow;
   }
  else
   Cur_y++;

  if (check_window(handle))  
    Bconout(2, 0x0A);
}

/**
*** set_mark() and use_mark() are used to remember and restore the current
*** screen position. Many terminal emulators provide a built-in facility
*** for this.
**/

PRIVATE int mark_x, mark_y;

PRIVATE void set_mark()
{
   mark_x = Cur_x; mark_y = Cur_y;
}

PRIVATE void use_mark()
{
  move_to(mark_y, mark_x);
}

PRIVATE void foreground(colour)
int colour;
{ colour = colour;
}

PRIVATE void background(colour)
int colour;
{ colour = colour;
}

PRIVATE void set_inverse(flag)
int flag;
{ if (check_window(handle))
   { Bconout(2, 0x1B); Bconout(2, flag ? 'p' : 'q'); }
}

PRIVATE void set_bold(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_underline(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_italic(flag)
int flag;
{ flag = flag;
}

/**
*** These routines are responsible for sending converted terminal output to
*** the screen, by one means or another. If there are multiple windows, I
*** always buffer the data and send it in one lump using send_to_window().
*** Otherwise, on the PC and the ST send_ch is #define'd to be an
*** immediate operation so there is no need to worry about flush().
**/

#if multiple_windows

void send_ch(ch)
int ch;
{ if (check_window(handle))
   Bconout(2, ch);
}
#endif

void flush()
{
}

/**
*** The following code is used to switch screens.
**/
void redraw_screen(window, han)
Window *window;
word han;
{ Screen *screen = &(window->screen);
  char *text = &(window->node.direntry.Name[0]);
  byte **map = screen->map;
  int i, j, k;

  handle = window->handle;

  Bconout(2, 0x1B); Bconout(2, 'f');    /* Disable cursor */
  Bconout(2, 0x1B); Bconout(2, 'E');    /* Cls */ 

  for (i = 0; i <= LastRow; i++)
   { for (j = LastCol; j >= 0; j--)
       if (map[i][j] ne ' ') break;
     for (k = 0; k <= j; k++)
      Bconout(2, map[i][k]);

     Bconout(2, '\r');
     if (i < LastRow)
      Bconout(2, '\n');
   }

  move_to(0, LastCol + 1 - strlen(text));
  set_inverse(1);
  for ( ; *text ne '\0'; text++)
   Bconout(2, *text);
  set_inverse(0);
  move_to(screen->Cur_y, screen->Cur_x);
  Bconout(2, 0x1B); Bconout(2, 'e');    /* Enable cursor */
  use(han)
}

#endif /* ST specific routines */

#if (UNIX)
/**
*** On a unix box I want to use termcap capabilities for all my work,
*** irrespective of what type of windowing system I am on. The
*** sunlocal routines should set up the following strings using the
*** TERMCAP database.
**/
extern string termcap_cls;       /* clear screen and home cursor */
extern string termcap_ceol;      /* clear end-of-line */
extern string termcap_move;      /* the move-cursor string */
extern string termcap_inv;       /* set inverse video */
extern string termcap_nor;       /* set normal video */
extern string termcap_bell;      /* the bells, the bells !!! */

#if ANSI_prototypes
#ifdef __cplusplus
extern "C"
{
#endif
PRIVATE void send_string(char *);
PUBLIC char * tgoto (char *, int, int);
#ifdef __cplusplus
}
#endif
#else
PRIVATE void send_string();
#endif

/**
*** This routine should clear the entire screen and move the cursor to the
*** top-left corner. There must be a cls string on every terminal,
*** surely....
**/

PRIVATE void clear_screen()
{ register int  i, j;

  for (i = 0; i <= LastRow; i++)
   for (j = 0; j <= LastCol; j++)
    map[i][j] = ' ';
  Cur_x = 0; Cur_y = 0;

  send_string(termcap_cls);
}


/**
*** This routine should clear the rest of the current line, including the
*** cursor position, leaving the cursor position unchanged. Not all terminals are
*** equipped with a clear_eol string, so I may have to emulate it using spaces.
*** Cur_x is guaranteed to be set up. The map is guaranteed to have been
*** updated already.
**/
PRIVATE void clear_eol()
{ if (termcap_ceol eq NULL)
   { register int i;
     if (terminal_wraps && (Cur_y eq LastRow))
      { for (i = Cur_x; i < LastCol; i++)
         send_ch(' ');
        current_screen->flags |= ANSI_dirty;
      }
     else
      for (i =  Cur_x; i <= LastCol; i++)
       send_ch(' ');
     move_to(Cur_y, Cur_x);
   }
  else 
   send_string(termcap_ceol);
}

/**
*** This routine should move the cursor to the position specified. A cursor-move
*** facility should always be available.
**/

PRIVATE void move_to(y, x)
int y, x;
{ 
  Cur_y = y; Cur_x = x;
  send_string(tgoto(termcap_move, x, y));
}

/**
*** This routine should refresh the screen from the start_y, leaving
*** the cursor position unchanged.
**/

PRIVATE void refresh_screen(start_y, end_y)
{ int old_x, old_y;
  register int j, k;
  int i;
  register byte *a_ptr;

  old_x = Cur_x; old_y = Cur_y;
  move_to(start_y, 0); 

  if (termcap_ceol eq NULL)  /* no short cuts */
   { for (i = start_y; i <= end_y; i++)
      { a_ptr = map[i];
        if ((i eq LastRow) && terminal_wraps)
         { for (j = 0; j < LastCol; j++)
            send_ch(a_ptr[j]);
           current_screen->flags |= ANSI_dirty;
         }
        else
         for (j = 0; j <= LastCol; j++)
           send_ch(a_ptr[j]);

        if (i < LastRow)
         { if (terminal_wraps)
            move_to(i+1, 0);
           else	
            { send_ch('\r'); send_ch('\n'); }
         }
      }
     move_to(old_y, old_x);
     return;
   }

  for (i = start_y; i <= end_y; i++)
    { a_ptr = map[i]; 

      for (j = LastCol; j >= 0; j--)  /* find the last non-space */
       if (a_ptr[j] ne ' ') break;

      if (j < 0)
       { send_string(termcap_ceol); goto next_line; }

      if ((j eq LastCol) && terminal_wraps)  /* special case */
       { if (i eq LastRow)                   /* special case within special case */
          { for (k = 0; k < LastCol; k++)
             send_ch(a_ptr[k]);
            current_screen->flags |= ANSI_dirty;
            break;                           /* must have finished */
          }
         else
          { for (k = 0; k <= LastCol; k++)
             send_ch(a_ptr[k]);
            move_to(i+1, 0);                 /* position indeterminate */
            continue;                        /* after wrap */
          }
       }
      else
       { for (k = 0; k <= j; k++)
          send_ch(a_ptr[k]); 
         send_string(termcap_ceol);
       }

next_line:
      if (i < LastRow)
       {  send_ch('\r'); send_ch('\n'); } 
    } 
  move_to(old_y, old_x);
}

/**
*** This routine should cause an audio or visual alert of some sort.
**/
PRIVATE void ring_bell()
{
  send_string(termcap_bell);
}

/**
*** This routine should move the cursor left one position, without erasing the
*** character.
**/
PRIVATE void backspace()
{ if (Cur_x eq 0) return;
  Cur_x --;
  send_ch(0x08);
}

/**
*** This routine should move the cursor to the first column of the current row.
**/

PRIVATE void carriage_return()
{ Cur_x = 0;
  send_ch(0x0D);
}

/**
*** This routine should move the cursor down one row. If the cursor is on the
*** bottom row the screen should scroll up one line, and the actual cursor
*** position will remain unchanged. The cursor should NOT move to the beginning
*** of the line.
**/

PRIVATE void linefeed()
{
  if (Cur_y >= LastRow)
    {
      register int i;
      register byte *petr = map[0]; /* Scroll up */

      for (i = 0; i < LastRow; i++)
	map[i] = map[i+1];
      for (i = 0; i <= LastCol; i++)
	petr[i] = ' ';
      map[LastRow] = petr;
      Cur_y = LastRow;

      if (current_screen->flags & ANSI_dirty)   /* terminal wrapping problems */
	{
	  int old_x = Cur_x;
	  int old_y = Cur_y;
	  move_to(LastRow, 0);                  /* force a scroll */
	  send_ch(0x0A);
	  move_to(LastRow-1, LastCol);          /* update the bottom right corner */
	  send_ch(map[LastRow - 1][LastCol]);   /* this causes an automatic scroll */
	  move_to(old_y, old_x);
	  current_screen->flags &= ~ANSI_dirty;
	  return;
	}
    }
  else
    {
      Cur_y++;
    }

  send_ch(0x0A);
}

/**
*** set_mark() and use_mark() are used to remember and restore the current
*** screen position. Many terminal emulators provide a built-in facility
*** for this.
**/

PRIVATE int mark_x, mark_y;

PRIVATE void set_mark()
{
   mark_x = Cur_x; mark_y = Cur_y;
}

PRIVATE void use_mark()
{
  move_to(mark_y, mark_x);
}

PRIVATE void foreground(colour)
int colour;
{ colour = colour;
}

PRIVATE void background(colour)
int colour;
{ colour = colour;
}

PRIVATE void set_inverse(flag)
int flag;
{ if (flag)
   send_string(termcap_inv);  
  else
   send_string(termcap_nor);
}

PRIVATE void set_bold(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_underline(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_italic(flag)
int flag;
{ flag = flag;
}

/**
*** These routines are responsible for sending converted terminal output to
*** the screen, by one means or another. If there are multiple windows, I
*** always buffer the data and send it in one lump using send_to_window().
**/

#define MAX_BUFF   512	
int ANSI_buffer_count = 0;
PRIVATE unsigned char out_buff[MAX_BUFF + 1];

PRIVATE void send_string(str)
char *str;
{ for ( ; *str ne '\0'; str++)
   { out_buff[ANSI_buffer_count++] = *str;
     if (ANSI_buffer_count >= MAX_BUFF) flush();
   }
}

void send_ch(ch)
unsigned char ch;
{ 
  out_buff[ANSI_buffer_count++] = ch;
  if (ANSI_buffer_count >= MAX_BUFF)
    flush();
}

void flush()
{ if (ANSI_buffer_count eq 0) return;
  out_buff[ANSI_buffer_count] = '\0';
  send_to_window((char *)out_buff, (Window *)handle);
  ANSI_buffer_count = 0;
}

/**
*** When the server is running from a dumb terminal this routine may be
*** called to switch to a new window. 
**/

#ifdef SOLARIS
PRIVATE char redraw_buf[1024];
#else
PRIVATE byte *redraw_buf[128];	/* was char */
#endif

void redraw_screen(window, handle)
Window *window;
word handle;
{ Screen *screen = &(window->screen);
  char *text = &(window->node.direntry.Name[0]);
  register byte **map = screen->map;
  register int i, j;
  int Rows = screen->Rows; int Cols = screen->Cols;

  write(handle, termcap_cls, strlen(termcap_cls));

  for (i = 0; i < (terminal_wraps ? Rows - 1 : Rows); i++)
   { for (j = Cols - 1; j >= 0; j--)
       if (map[i][j] ne ' ') break;
     if (j >= 0)
       write(handle, map[i], j+1);

     if (i < (Rows - 1))
      write(handle, "\r\n", (i < (Rows - 1)) ? 2 : 1 );   
   }

  if (terminal_wraps)
   { for (j = Cols -1; j >= 0; j--)
      if (map[Rows - 1][j] ne ' ') break;
     if (j >= 0)
      { if (j eq (Cols - 1))
         { screen->flags |= ANSI_dirty;
           j--;
         }
        write(handle, map[Rows - 1], j + 1);
      }
   }

  if (!real_windows)
   {
#if SOLARIS
     char * redraw_bufp = redraw_buf;

     strcpy(redraw_bufp, tgoto(termcap_move, Cols - strlen(text), 0));
     strcat(redraw_bufp, termcap_inv);
     strcat(redraw_bufp, text);
     strcat(redraw_bufp, termcap_nor);
#else
     strcpy(redraw_buf, tgoto(termcap_move, Cols - strlen(text), 0));
     strcat(redraw_buf, termcap_inv);
     strcat(redraw_buf, text);
     strcat(redraw_buf, termcap_nor);
#endif
   }
  else
   redraw_buf[0] = '\0';

#if SOLARIS
  {
    char * redraw_bufp = redraw_buf;

    strcat(redraw_bufp, tgoto(termcap_move, screen->Cur_x, screen->Cur_y));
  }
#else
  strcat(redraw_buf, tgoto(termcap_move, screen->Cur_x, screen->Cur_y));
#endif

#if SOLARIS
  {
    char * redraw_bufp = redraw_buf;
    
    write(handle, redraw_buf, strlen(redraw_buf));
  }
#else
  write(handle, redraw_buf, strlen(redraw_buf));
#endif
}

#endif /* unix specific routines */

#endif /* use_ANSI_emulator */

#if HELIOS
	/* When compiling under Helios there are horrible problems with	*/
	/* header file clashes... This hack solves one of them.		*/
extern void helios_send_to_window(char *text, int handle);
void send_to_window(char *text, Window *window)
{ helios_send_to_window(text, window->handle);
}
#endif

