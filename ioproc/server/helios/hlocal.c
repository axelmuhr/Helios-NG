/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1992, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  hlocal.c                                                            --
--                                                                      --
--  Author:  BLV                                                        --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1987, Perihelion Software Ltd.        */

/*{{{  header files etc. */
	/* N.B. This refers to /helios/include/helios.h, not to		*/
	/* ../helios.h. It is more useful for the I/O Server to		*/
	/* be able to use the standard header files than the weirdo	*/
	/* stuff in the I/O Server headers.				*/
#include <helios.h>
#include <syslib.h>
#include <sem.h>
#include <setjmp.h>
#include <time.h>
#include <nonansi.h>
#include <stdlib.h>
#include <string.h>
#include <attrib.h>
#include <codes.h>
#include <unistd.h>
#include <gsp.h>
#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <link.h>
#include <rmlib.h>
#include <servlib.h>
#include <stdarg.h>
#undef Malloc

#define eq ==
#define ne !=

#include "debugopt.h"
#include "helios/hlocal.h"
#undef Wait
#undef InitSemaphore
#undef Signal
extern word	 Special_Reboot, Special_Exit, Special_Status, DebugMode;
extern char	*get_config(char *);
extern char	*Heliosdir;
extern char	 misc_buffer1[];
extern void	 FreeList(List *);
extern Node	*AddTail(Node *, List *);
extern void	 ServerDebug(char *, ...);
extern jmp_buf	 exit_jmpbuf;
/*}}}*/
/*{{{  statics and initialisation, plus file name translation */

	/* When the I/O Server is running under Helios the majority	*/
	/* of accesses to /helios can go to the /helios directory.	*/
	/* An alternative one could be specified to test an		*/
	/* experimental version of Helios. The main exception is	*/
	/* accesses to /helios/etc: it will always be necessary to have	*/
	/* an alternative etc directory with its own version of initrc	*/
	/* and its own resource map. Hence this initialisation code	*/
	/* prepares the necessary statics for check_helios_world(),	*/
	/* which performs the  necessary name translation.		*/
static	char	*etc_directory;
static	char	etc_name[128];
static	int	etc_length;

void unix_initialise_devices(void)
{ etc_directory = get_config("etc_directory");
  if (etc_directory eq NULL) return;
  strcpy(etc_name, Heliosdir);
  strcat(etc_name, "/etc");
  etc_length = strlen(etc_name);
}

	/* Accesses to /helios/etc have to be intercepted.	*/
	/* This routine is called after the name translation in	*/
	/* files.c, so the name should be something like	*/
	/* <heliosdir>/xyz					*/
void check_helios_name(char *name)
{
  if (etc_directory eq NULL) return;
  if (!strncmp(name, etc_name, etc_length))
   { strcpy(misc_buffer1, etc_directory);
     strcat(misc_buffer1, &(name[etc_length]));
     strcpy(name, misc_buffer1);
   }
}

/*}}}*/
/*{{{  coroutine library */
/**
*** The coroutine library works using threads and semaphores, as follows:
***
*** 1) CreateCo spawns a new thread. This initialises a setjmp buffer to allow
***    the coroutine to be aborted, waits for the first CallCo, and invokes
***    the user function.
*** 2) CallCo signals the child coroutine and blocks, waiting for the child
***    to do a Waitco
*** 3) WaitCo signals the parent coroutine and blocks, waiting for the parent
***    to do another CallCo. If the parent performs a DeleteCo then this
***    is detected using the Finished flag.
**/

typedef struct coroutine {
	Semaphore	Wait;
	jmp_buf		Jmp_buf;
	word		Parent;
	bool		Finished;
	word		Result;
} coroutine;

	word		CurrentCo;
PRIVATE coroutine	root_coroutine;

word	InitCo(void)
{ CurrentCo			= (word) &root_coroutine;
  root_coroutine.Parent		= CurrentCo;
  root_coroutine.Finished	= FALSE;
  root_coroutine.Result		= 0;
  InitSemaphore(&(root_coroutine.Wait), 0);
}

word	CallCo(coroutine *cortn, word arg)
{ coroutine	*current	= (coroutine *) CurrentCo;
  cortn->Result			= arg;
  cortn->Parent			= CurrentCo;
  CurrentCo			= (word) cortn;
  Signal(&(cortn->Wait));
  Wait(&(current->Wait));
  return(current->Result);
}

static void CreateCo_aux(VoidFnPtr fn, coroutine *cortn)
{
  if (setjmp(cortn->Jmp_buf) ne 0)
   { free(cortn);
     return;
   }
  Wait(&(cortn->Wait));
  forever
   (*fn)(cortn);
}

coroutine *CreateCo(VoidFnPtr fn, word stacksize)
{ coroutine	*newco	= New(coroutine);

  if (newco eq NULL) return(NULL);
  newco->Finished	= FALSE;
  InitSemaphore(&(newco->Wait), 0);
  unless(Fork(stacksize, &CreateCo_aux, 8, fn, newco))
   { Free(newco);
     return(NULL);
   }

  return(newco);
}

word	WaitCo(word result)
{ coroutine	*me	= (coroutine *) CurrentCo;
  coroutine	*parent	= (coroutine *) me->Parent;

  parent->Result	= result;
  CurrentCo		= (word) parent;
  Signal(&(parent->Wait));
  Wait(&(me->Wait));
  if (me->Finished)
   longjmp(me->Jmp_buf, 1);
  else
   return(me->Result);
}

word	DeleteCo(coroutine *cortn)
{ cortn->Finished	= TRUE;
  Signal(&(cortn->Wait));
}

/*}}}*/
/*{{{  screen I/O */
/**
*** Screen I/O involves multiple windows, using the current window server
*** as held in the environment. create_a_window() performs the following:
***  1) find the window server from the environment.
***  2) allocate a new window structure.
***  3) create the window. The name is derived from the user-supplied name and
***     the current task.
***  4) open two streams to the window, one for reading and one for writing.
***  5) set the window to raw mode etc.
***  6) spawn a thread to perform keyboard reads in the background. This
***     thread synchronises with routine read_char_from_keyboard()
***
*** The other routines follow fairly naturally. Note that there is no
*** support for resizable windows.
**/

typedef struct	Window	{
	Object		*Created;
	Stream		*Reading;
	Stream		*Writing;
	int	 	 Rows;
	int	 	 Cols;
	Semaphore	 KeyboardData;
	int		 KeyboardInput;
	Semaphore	 ReaderContinue;
	int		 KeyboardState;
	int		 KeyboardSave;
} Window;

#define	Keyboard_Normal		0
#define	Keyboard_GotCSI		1
#define Keyboard_Got9		2
#define Keyboard_GotHotkey	3
#define Keyboard_SentCSI	4

static void	keyboard_reader(Window *);

Window	*create_a_window(char *name)
{ Environ	*my_environ	= getenviron();
  Object	*window_server	= my_environ->Objv[OV_CServer];
  Object	*my_task	= my_environ->Objv[OV_Task];
  Window	*new_window	= New(Window);
  char		 name_buf[65];
  Attributes	 attr;

  if (new_window eq NULL) return(NULL);

  sprintf(name_buf, "%s:%.31s", objname(my_task->Name), name);
  new_window->Created	= Create(window_server, name_buf, Type_Stream, 0, NULL);
  if (new_window->Created eq NULL) goto fail;

  new_window->Reading	= Open(new_window->Created, NULL, O_ReadOnly);
  new_window->Writing	= Open(new_window->Created, NULL, O_WriteOnly);
  if ((new_window->Reading eq NULL) || (new_window->Writing eq NULL))
   goto fail;

  if (GetAttributes(new_window->Reading, &attr) < Err_Null) goto fail;
  new_window->Rows	= attr.Min;
  new_window->Cols	= attr.Time;
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsolePause);
  AddAttribute(&attr, ConsoleRawInput);
  AddAttribute(&attr, ConsoleRawOutput);
  RemoveAttribute(&attr, ConsoleIgnoreBreak);
  RemoveAttribute(&attr, ConsoleBreakInterrupt);
  if (SetAttributes(new_window->Reading, &attr) < Err_Null) goto fail;

  new_window->KeyboardState	= Keyboard_Normal;
  InitSemaphore(&(new_window->KeyboardData), 0);
  InitSemaphore(&(new_window->ReaderContinue), 0);
  unless(Fork(2000, &keyboard_reader, 4, new_window))
   goto fail;

  return(new_window);

fail:
  if (new_window->Reading ne NULL) Close(new_window->Reading);
  if (new_window->Writing ne NULL) Close(new_window->Writing);
  if (new_window->Created ne NULL) Delete(new_window->Created, NULL);
  Free(new_window);
  return(NULL);
}

void	window_size(Window *window, word *x, word *y)
{ *x = window->Cols;
  *y = window->Rows;
}

void	close_window(Window *window)
{ Stream	*reading	= window->Reading;

	/* Synchronise with the reader thread */
  window->Reading		= Null(Stream);
  Abort(reading);
  Signal(&(window->ReaderContinue));

  Close(reading);
  Close(window->Writing);
  Delete(window->Created, Null(char));

	/* Assume the reader has gone away by now	*/
  Free(window);
}

void	helios_send_to_window(BYTE *data, Window *window)
{ int	len = strlen(data), index, result;

  for (index = 0; index < len; )
   { result = Write(window->Writing, &(data[index]), len - index, 20 * OneSec);
     if (result < 0) break;
     index += result;
   }
}

	/* This runs as a separate thread reading a character from the	*/
	/* keyboard and synchronising with read_char_from_keyboard()	*/
	/* below. The thread will exit when the Reading stream is	*/
	/* aborted, in close_window().					*/
static	void	keyboard_reader(Window *window)
{ BYTE	buf[2];
  word	result;

  while(window->Reading ne Null(Stream))
   { result = Read(window->Reading, buf, 1, 20 * OneSec);
     if (result > 0)
      { window->KeyboardInput	= ((int) buf[0]) & 0x00FF;
	Signal(&(window->KeyboardData));
	Wait(&(window->ReaderContinue));
      }
   }
}

	/* This is the keyboard input routine. Note that it is	*/
	/* responsible for enabling debugging options, setting	*/
	/* the reboot flag, etc. The hot-key is F10, and a	*/
	/* finite state machine is used similar to that in the	*/
	/* Unix I/O Server. The magic sequence is CSI 9 ~ xxx	*/
	/* F10 is the only key that can generate CSI 9		*/
static void debug_option(int key);

int	read_char_from_keyboard(Window *window)
{ int	result = -1;

  if (window->KeyboardState eq Keyboard_SentCSI)
   { window->KeyboardState = Keyboard_Normal;
     return(window->KeyboardSave);
   }

	/* If the keyboard reader does not have another		*/
	/* character TestWait() will fail.			*/
  if (!TestWait(&(window->KeyboardData)))
   return(-1);

  result = window->KeyboardInput;
  Signal(&(window->ReaderContinue));	/* Restart the reader thread	*/

  switch(window->KeyboardState)
   { case Keyboard_Normal :
     default		  :
		if (result eq 0x09B)
		 { window->KeyboardState = Keyboard_GotCSI;
		   return(-1);
		 }
		else
		 { window->KeyboardState = Keyboard_Normal;
		   return(result);
		 }

     case Keyboard_GotCSI :
		if (result ne '9')
		 { window->KeyboardState = Keyboard_SentCSI;
		   window->KeyboardSave  = result;
		   return(0x009B);
		 }
		else
		 { window->KeyboardState = Keyboard_Got9;
		   return(-1);
		 }

     case Keyboard_Got9 :
		if (result ne '~')	/* What the ...	*/
		 { window->KeyboardState = Keyboard_Normal;
		   return(result);
		 }
		else
		 { window->KeyboardState = Keyboard_GotHotkey;
		   return(-1);
		 }

     case Keyboard_GotHotkey :
		debug_option(result);
		window->KeyboardState = Keyboard_Normal;
		return(-1);
   }
}

static void debug_option(int key)
{ int i;

  switch (key)
   { case 'a': case 'A': if (!debugflags)
		 	  debugflags = All_Debug_Flags;
		 	 else
			  debugflags = 0;
			 return;

     case '0' : Special_Reboot = TRUE; return;
     case '9' : Special_Exit   = TRUE; return;
     case '8' : Special_Status = TRUE; return;
     case '7' : DebugMode      = TRUE; return;
   }

  for (i = 0; options_list[i].flagchar ne '\0'; i++)
   if (key eq options_list[i].flagchar)
    { debugflags ^= options_list[i].flag;
      return;
    }
}
/*}}}*/
/*{{{  file I/O */
/**
*** File I/O uses a strange mixture of Helios and Posix file I/O, whichever
*** seems more appropriate at the time.
**/

word	object_exists(char *name)
{ Object	*x = Locate(Null(Object), name);

  if (x eq Null(Object)) 
   { searchbuffer.st_mode = S_IFREG;
     return(FALSE);
   }
  else
   {	/* Zap searchbuffer, used for the macro object_isadirectory	*/
     if ((x->Type & Type_Flags) eq Type_Directory)
      searchbuffer.st_mode = S_IFDIR;
     else
      searchbuffer.st_mode = S_IFREG;
     Close(x);
     return(TRUE);
   }
}

word	create_directory(char *name)
{ if (mkdir(name, 0777) eq -1)
   return(FALSE);
  else
   return(TRUE);
}

word	delete_directory(char *name)
{ if (rmdir(name) ne 0)
   return(FALSE);
  else
   return(TRUE);
}

word	delete_file(char *name)
{ if (unlink(name) ne 0)
   return(FALSE);
  else
   return(TRUE);
}

word	rename_object(char *from, char *to)
{
  if (rename(from, to) ne 0)
   return(FALSE);
  else
   return(TRUE);
}

word	create_file(char *name)
{ int handle = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (handle < 0)
   return(FALSE);
  else
   { close(handle);
     return(TRUE);
   }
}

word	get_file_info(char *name, ObjInfo *info)
{ word result = ObjectInfo(cdobj(), name, (BYTE *) info);
  return((result >= Err_Null) ? TRUE : FALSE);
}

word	set_file_date(char *name, word timestamp)
{ struct utimbuf timestamps;
  timestamps.ctime	=
  timestamps.actime	=
  timestamps.modtime	= timestamp;
  return((utime(name, &timestamps) eq 0) ? TRUE : FALSE);
}

word	get_drive_info(char *name, void *buffer)
{ name = name; buffer = buffer;
  return(FALSE);
}

int	open_file(char *name, word mode)
{ int handle = open(name, mode);
  if (handle < 0) 
   return(0);
  else
   return(handle);
}

word	close_file(int handle)
{ return((close(handle) eq 0) ? TRUE : FALSE);
}

word	read_from_file(int handle, word amount, BYTE *buffer)
{ return(read(handle, buffer, amount));
}

word	write_to_file(int handle, word amount, BYTE *buffer)
{ if (write(handle, buffer, amount) < 0)
   return(FALSE);
  else
   return(TRUE);
}

word	seek_in_file(int handle, word offset, word mode)
{ int result = (mode eq 0) ?	lseek(handle, offset, SEEK_SET) :
				lseek(handle, offset, SEEK_END);
  return(result);
}

word	get_file_size(int handle, word currentpos)
{ return(GetFileSize(fdstream(handle)));
  currentpos = currentpos;
}

typedef struct DirEntryNode {
	Node		node;
	DirEntry	direntry;
	word		size;
	word		account;
} DirEntryNode;

word	search_directory(char *name, List *entries)
{ Stream	*x = Open(cdobj(), name, O_ReadOnly);
  DirEntry	 entry;
  DirEntryNode	*newnode;
  int		 count	= 0;

  if (x eq Null(Stream)) return(-1);

  while(Read(x, (BYTE *) &entry, sizeof(DirEntry), -1) > 0)
   { newnode = New(DirEntryNode);
     if (newnode eq NULL)
      { count = -1; break; }
     memcpy(&(newnode->direntry), &entry, sizeof(DirEntry));
     AddTail(&(newnode->node), entries);
     count++;
    }

  Close(x);
  if (count eq -1)
   FreeList(entries); 
  return(count);
}


/*}}}*/
/*{{{  link I/O and network administration */
/*{{{  variables */
	/* These variables are part of the linkio module and have to be	*/
	/* set by resetlnk().						*/
extern int  (*rdrdy_fn)();
extern int  (*wrrdy_fn)();
extern int  (*byte_to_link_fn)();
extern int  (*byte_from_link_fn)();
extern int  (*send_block_fn)();
extern int  (*fetch_block_fn)();
extern void (*reset_fn)();
extern void (*analyse_fn)();

PRIVATE	bool		 Link_Initialised = FALSE;
PRIVATE int		 LinkToUse	 = -1;
PRIVATE bool		 Terminating	 = FALSE;
extern	int		 Default_BootLink;
PRIVATE	Semaphore	 Linkdata_Waiting;
PRIVATE Semaphore	 LinkReader_Continue;
PRIVATE	int		 LinkChar;
	bool		 network_reset(word proc);
	bool		 network_analyse(word proc);
PRIVATE RmProcessor	*Network_Vec;
PRIVATE RmNetwork	 Obtained_Network = NULL; 
/*}}}*/
/*{{{  link I/O functions */
	/* This thread is used to fetch protocol bytes from the link.	*/
	/* It will also get the first byte of a block, so fetch_block()	*/
	/* has to synchronise...					*/
PRIVATE void LinkReader(void)
{ BYTE	buf[2];

  while (!Terminating)
   { if (LinkIn(1, LinkToUse, buf, 2 * OneSec) < Err_Null) continue;
     LinkChar	= buf[0];
     Signal(&(Linkdata_Waiting));
     Wait(&(LinkReader_Continue));
   }
}
	/* rdrdy() takes care of suspending the I/O Server if nothing	*/
	/* is happening.						*/
PRIVATE int rdrdy(void)
{ if (TestSemaphore(&(Linkdata_Waiting)) > 0)
   return(TRUE);
  else
   { Delay(1000);	/* deschedule while other things are happening */
     return(FALSE);
   }
}

PRIVATE int wrrdy(void)
{ return(TRUE);
}

PRIVATE int byte_to_link(int value)
{ BYTE	buf[2];
  buf[0] = value;
  if (LinkOut(1, LinkToUse, buf, 2 * OneSec) < Err_Null)
   return(1);
  else
   return(0);
}

PRIVATE int byte_from_link(char *buff)
{ int	i;
  for (i = 0; i < 10; i++)
   if (TestWait(&(Linkdata_Waiting)))
    { *buff = LinkChar;
      Signal(&(LinkReader_Continue));
      return(0);
    }
   else
    Delay(20000);
  return(1);
}

PRIVATE int fetch_block(int count, char *data, int timeout)
{ int	i;
  int	result = 0;

  for (i = 0; i < 10; i++)
   if (TestWait(&(Linkdata_Waiting)))
    { *data = LinkChar;
      if (count > 1)
       if (LinkIn(count - 1, LinkToUse, &(data[1]), 5 * OneSec) < Err_Null)
        result = count - 1;
      Signal(&(LinkReader_Continue));        
      return(result);
    }
   else
    Delay(20000);
  return(count);
  timeout = timeout;
}

PRIVATE int send_block(int count, char *data, int timeout)
{
  if (LinkOut(count, LinkToUse, data, 5 * OneSec) < Err_Null)
   return(count);
  else
   return(0);
  timeout = timeout;
}

	/* The root processor is always processor 0 in terms of the	*/
	/* NetworkController functions.					*/
PRIVATE void reset_proc(void)
{ network_reset(0);
}

PRIVATE void analyse_proc(void)
{ network_analyse(0);
}
/*}}}*/
/*{{{  network_ functions for /NetworkController */
/**
*** For now only reset is supported, and can be implemented fairly trivially
*** using RmLib. Since configuration is not supported it is assumed that
*** the order of processors in the subnet resource map is the same as in
*** the main resource map, thus ensuring that there are no problems with
*** processor identification.
**/
bool	network_reset(word proc)
{ if (RmResetProcessors(1, &(Network_Vec[proc])) ne RmE_Success)
   return(FALSE);
  else
   return(TRUE);
}

	/* RmLib does not have an analyse, assume that a reset is	*/
	/* implemented as a hardware analyse. This is true for most	*/
	/* hardware.							*/
bool	network_analyse(word proc)
{ if (RmResetProcessors(1, &(Network_Vec[proc])) ne RmE_Success)
   return(FALSE);
  else
   return(TRUE);
}

bool	network_connect(word src, word srclink, word dst, word dstlink)
{ src = srclink; dst = dstlink;
  return(FALSE);
}

bool	network_disconnect(word src, word srclink)
{ src = srclink;
  return(FALSE);
}

bool	network_enquire(word src, word srclink)
{ src = srclink;
  return(FALSE);
}
/*}}}*/
/*{{{  resetlnk() auxiliaries */
	/* Clear the private flag, to allow unwanted processors to be	*/
	/* removed later on.						*/
PRIVATE int	resetlnk_clear(RmProcessor processor, ...)
{ RmSetProcessorPrivate(processor, 0);
  return(0);
}

	/* Remove unwanted processors...				*/
PRIVATE int	resetlnk_remove(RmProcessor processor, ...)
{ if (RmGetProcessorPrivate(processor) eq 0)
   RmFreeProcessor(RmRemoveProcessor(processor));
  return(0);
}

	/* Alternative to LookupProcessor().				*/
PRIVATE int	resetlnk_find(RmProcessor processor, ...)
{ va_list	 args;
  char		*name_to_match;

  va_start(args, processor);
  name_to_match = va_arg(args, char *);
  va_end(args);

  if (*name_to_match eq '/') name_to_match++;
  
  if (!strcmp(name_to_match, (char *) RmGetProcessorId(processor)))
   return((int) processor);
  else
   return(0);
}

	/* Put an obtained processor in the table, and set it to	*/
	/* exclusive and native mode.					*/
PRIVATE int	resetlnk_table(RmProcessor processor, ...)
{ va_list	 args;
  int		*index_ptr;

  va_start(args, processor);
  index_ptr = va_arg(args, int *);
  va_end(args);

  Network_Vec[*index_ptr] = processor;
  *index_ptr += 1;

  RmSetProcessorPurpose(processor, RmP_Native);
  RmSetProcessorExclusive(processor);
  return(0);
}

	/* This routine is passed a processor within the template and	*/
	/* the real network, and is responsible for matching the two.	*/
	/* A special effort is needed for the I/O processor of the	*/
	/* native network, to sort out the bootlink and LinkToUse.	*/
PRIVATE int resetlnk_match(RmProcessor processor, ...)
{ va_list	args;
  RmNetwork	real_network;

  va_start(args, processor);
  real_network = va_arg(args, RmNetwork);
  va_end(args);

  if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_IO)
   { RmProcessor	 neighbour;
     RmProcessor 	 real_neighbour;     
     char		*is_really;
     RmProcessor	 real_IO;

	/* In template: given /IO, find the root processor		*/
	/* This gives us the Default_BootLink because, without dynamic	*/
	/* link reconfiguring, the resource map has to specify the	*/
	/* right link back to the host.					*/
     neighbour		= RmFollowLink(processor, 0, &Default_BootLink);
     if (neighbour eq (RmProcessor) NULL)
      { ServerDebug("That is one confused I/O processor you got there.");
        longjmp(exit_jmpbuf, 1);
      }

	/* Given the root processor, what is its real name ?		*/
     is_really = (char *) RmGetProcessorAttribute(neighbour, "really");
     if (is_really eq NULL)
      { ServerDebug("Missing mapping for the root processor %s", 
		RmGetProcessorId(neighbour));
        longjmp(exit_jmpbuf, 1);
      }

	/* Given the real name, find it in the real network.		*/
     real_neighbour = (RmProcessor) RmSearchProcessors(real_network, &resetlnk_find, is_really);     
     if (real_neighbour eq (RmProcessor) NULL)
      { ServerDebug("Cannot find processor %s in the network", is_really);
        longjmp(exit_jmpbuf, 1);
      }

	/* Given the real root processor, find the I/O processor within	*/
	/* the real network. It might be desirable to check that this	*/
	/* program is running on that processor... Anyway, this gives	*/
	/* the link from the I/O Server to the root processor.		*/
     real_IO = RmFollowLink(real_neighbour, Default_BootLink, &LinkToUse);
     if (real_IO eq NULL)
      { ServerDebug("The host processor does not appear to exist...");
        longjmp(exit_jmpbuf, 1);
      }
   }
  else		/* Not an I/O processor.			*/
   { char	*is_really = (char *) RmGetProcessorAttribute(processor, "really");
     RmProcessor real_proc;

     if (is_really eq NULL)
      { ServerDebug("Missing mapping for processor %s", RmGetProcessorId(processor));
        longjmp(exit_jmpbuf, 1);
      }
     real_proc = (RmProcessor) RmSearchProcessors(real_network, &resetlnk_find, is_really);
     if (real_proc eq NULL)
      { ServerDebug("Cannot find %s in the network", is_really);
        longjmp(exit_jmpbuf, 1);
      }
     RmSetProcessorPrivate(real_proc, 1);
   }
}
/*}}}*/

/**
*** resetlnk(). This is the big one.
*** 1) the routine may be called several times. It is only relevant the
***    first time around.
*** 2) the link I/O functions are set up so that linkio.c can call the
***    necessary machine-specific bits.
*** 3) the configuration file may specify a specific resource map.
***    Otherwise the resource map should be default.map in the etc directory
***    associated with this Helios session. The resource map is read in.
*** 4) every processor in the session's resource map should have an
***    attribute really=/xyz, where xyz is the processor name within the
***    main network. This is used to identify the processors within the
***    main network that should be obtained and switched to native mode.
*** 5) the output link is initialised, and the reader thread is spawned.
**/
void	resetlnk(void)
{ char		*resource_map;
  static char	 default_name[128];
  RmNetwork	 specified_network;
  RmNetwork	 real_network;

  if (Link_Initialised) return;
  Link_Initialised	= TRUE;
  rdrdy_fn		= &rdrdy;
  wrrdy_fn		= &wrrdy;
  byte_to_link_fn	= &byte_to_link;
  byte_from_link_fn	= &byte_from_link;
  send_block_fn		= &send_block;
  fetch_block_fn	= &fetch_block;
  reset_fn		= &reset_proc;
  analyse_fn		= &analyse_proc;

  Debug(Init_Flag, ("getting resource map details"));
  resource_map = get_config("resource_map");
  if (resource_map eq NULL)
   { if (etc_directory eq NULL)
      strcpy(default_name, "/helios/etc");
     else
      strcpy(default_name, etc_directory);
     pathcat(default_name, "default.map");
     resource_map = default_name;
   }

  if (RmRead(resource_map, &specified_network, NULL) ne RmE_Success)
   { ServerDebug("resetlnk: failed to read resource map %s\n", resource_map);
     longjmp(exit_jmpbuf, 1);
   }

  real_network = RmGetNetwork();
  if (real_network eq NULL)
   { ServerDebug("resetlnk: failed to get network details\n");
     longjmp(exit_jmpbuf, 1);
   }

  Debug(Init_Flag, ("matching resource map and real network"));
	/* Set private fields to 0					*/
  (void) RmApplyProcessors(real_network, &resetlnk_clear);

	/* Match template and real network.				*/
  (void) RmApplyProcessors(specified_network, &resetlnk_match, real_network);

	/* Remove unwanted processors from the real network		*/
  (void) RmApplyProcessors(real_network, &resetlnk_remove);

	/* Obtain the required processors.				*/
  Debug(Init_Flag, ("obtaining the required processors"));
  Obtained_Network = RmObtainNetwork(real_network, TRUE, NULL);
  if (Obtained_Network eq (RmNetwork) NULL)
   { ServerDebug("resetlnk: failed to obtain the specified processors.");
     longjmp(exit_jmpbuf, 1);
   }

	/* Free the real network and the template, no longer required	*/
  RmFreeNetwork(real_network);
  RmFreeNetwork(specified_network);

	/* Allocate a table to hold the obtained processors. This table	*/
	/* is used by the NetworkController functions to reset		*/
	/* processors.							*/
  Network_Vec = malloc(sizeof(RmProcessor) * RmCountProcessors(Obtained_Network));
  if (Network_Vec eq NULL)
   { ServerDebug("resetlnk: memory allocation failure");
     longjmp(exit_jmpbuf, 1);
   }

	/* Put all the obtained processors into a table			*/
  { int index = 0;
    (void) RmApplyProcessors(Obtained_Network, &resetlnk_table, &index);
  }

	/* Switch the obtained network to native mode, and reset them.	*/
  Debug(Init_Flag, ("setting the obtained network to native"));
  if (RmSetNetworkNative(Obtained_Network) ne RmE_Success)
   { ServerDebug("resetlnk: failed to set network to native mode, %x.", RmErrno);
     longjmp(exit_jmpbuf, 1);
   }

  (void) RmResetNetwork(Obtained_Network);

	/* The required processors are now native and reset...		*/
	/* The link to the root processor can be obtained, switched to	*/
	/* dumb, etc.							*/
  Debug(Init_Flag, ("configuring the link to the native sub-network"));
  { LinkInfo	info;
    LinkConf	conf;

    if (LinkData(LinkToUse, &info) < Err_Null)
     { ServerDebug("resetlnk: failed to get link details.");
       longjmp(exit_jmpbuf, 1);
     }
    conf.Flags	= info.Flags;
    conf.Id	= info.Id;
    conf.Mode	= Link_Mode_Dumb;
    conf.State	= Link_State_Dumb;
    if (Configure(conf) < Err_Null)
     { ServerDebug("resetlnk: failed to set link to dumb");
       longjmp(exit_jmpbuf, 1);
     }

    if (AllocLink(LinkToUse) < Err_Null)
     { ServerDebug("resetlnk: failed to claim link ownership");
       longjmp(exit_jmpbuf, 1);
     }

    InitSemaphore(&(Linkdata_Waiting), 0);
    InitSemaphore(&(LinkReader_Continue), 0);
    (void) Fork(2000, &LinkReader, 0);
  }
}

/**
*** tidy_link(). This starts by synchronising with the LinkReader() thread.
*** Then the link is switched back to not-connected and released, and
*** the native subnet is rebooted and released.
**/
void tidy_link(void)
{ Terminating = TRUE;
  Delay(3 * OneSec);	/* for the link reader to time out */

  { LinkInfo	info;
    LinkConf	conf;

    FreeLink(LinkToUse);
    LinkData(LinkToUse, &info);
    conf.Flags	= info.Flags;
    conf.Id	= info.Id;
    conf.Mode	= Link_Mode_Null;
    conf.State	= Link_State_Null;
    Configure(conf);
  }

  RmRevertNetwork(Obtained_Network);
  RmRebootNetwork(Obtained_Network);
  RmReleaseNetwork(Obtained_Network);
}


/*}}}*/
/*{{{  odds and ends */
word get_unix_time(void)
{ return((word) time(NULL));
}

void goto_sleep(word x)
{ Delay(x);
}
/*}}}*/

