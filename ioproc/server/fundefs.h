/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      fundefs.h                                                       --
--                                                                      --
--         Declarations of the functions shared between modules         --
--                                                                      --
--      Author:  BLV 8/10/87                                            --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: fundefs.h,v 1.14 1994/06/29 13:42:25 tony Exp nickc $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

#ifdef __cplusplus
#define iofree(x)	free ((void *)(x))
#else
#define iofree(x)	free (x)
#endif

/**
*** Linked list library, the same linked lists as under AmigaDos and Helios
**/
PUBLIC List *fn( MakeHeader,  (void));
PUBLIC void fn(  InitList,    (List *));
PUBLIC void fn(  WalkList,    (List *, VoidNodeFnPtr, ...));
PUBLIC void fn(  FreeList,    (List *));
PUBLIC Node *fn( AddHead,     (Node *, List *));
PUBLIC Node *fn( AddTail,     (Node *, List *));
PUBLIC Node *fn( listRemove,  (Node *));
PUBLIC Node *fn( NextNode,    (Node *));
PUBLIC word fn(  Wander,      (List *, WordNodeFnPtr, ...));
PUBLIC word fn(  TstList,     (List *));
PUBLIC word fn(  ListLength,  (List *));
PUBLIC void fn(  PreInsert,   (Node *, Node *));
PUBLIC void fn(  PostInsert,  (Node *, Node *));

/**
*** There are problems with Remove because all ANSI and pseudo-ANSI libraries
*** including the Microsoft one for the PC have a routine remove(), which
*** clashes with the list Remove with some linkers.
**/
#define Remove listRemove

/**
*** an 8086 has the same byte ordering as the transputer, a 68000 does not
**/
#if swapping_needed
PUBLIC word fn( swap, (word));
#else
#define swap(a) (a)
#endif

PUBLIC char *fn( get_config,      (char *));
PUBLIC word  fn(  get_int_config,  (char *));
PUBLIC word  fn(  mystrcmp,        (char *, char *));

PUBLIC int fn (setsu, (int));
PUBLIC void fn (convert_errno, (void));

#if (UNIX && !MEIKORTE && !UNIX386 && !IUNIX386)
PUBLIC void fn (check_helios_name, (char *));
#endif

#if (UNIX)
PUBLIC void fn (unix_restore_devices, (void));
PUBLIC void fn (unix_initialise_devices, (void));
#endif

#ifndef Daemon_Module

/**
*** A higher level coroutine library using the above linked lists.
**/
PUBLIC Conode *fn( NewCo,     (VoidConFnPtr));
PUBLIC void   fn(  TidyColib, (void));
PUBLIC void   fn(  StartCo,   (Conode *));
PUBLIC void   fn(  Suspend,   (void));
PUBLIC void   fn(  Seppuku,   (void));
PUBLIC word   fn(  InitColib, (void));
PUBLIC void   fn(  Wait,        (Semaphore *));
PUBLIC void   fn(  Signal,      (Semaphore *));
PUBLIC void   fn( InitSemaphore,(Semaphore *, int));

/**
*** Some general purpose request handlers. For example, doing things like
*** Rename on the Clock is not sensible, so Clock_Rename is #define'd to be
*** Invalidfn_handler
**/
PUBLIC  void fn( Invalidfn_handler,         (Conode *)); 
PUBLIC  void fn( Create_handler,            (Conode *));
PUBLIC  void fn( Dummy_handler,             (Conode *));

/*
 * PUBLIC  void fn( IgnoreVoid,                (void));
 */
#ifdef __cplusplus
PUBLIC  void fn (IgnoreVoid, 	(Conode *));
#else
PUBLIC  void fn (IgnoreVoid, (void));
#endif

PUBLIC  void fn( Device_ObjectInfo_handler, (Conode *));
PUBLIC  void fn( GetDefaultAttr,            (Conode *));
PUBLIC  void fn( Device_GetSize,            (Conode *));
PUBLIC  void fn( NullFn,                    (Conode *));
PUBLIC  word fn( Ignore,                    (void));
PUBLIC  void fn( Forward_handler,           (Conode *));
PUBLIC  void fn( Dir_Locate,                (Conode *));
PUBLIC  void fn( Dir_ObjInfo,               (Conode *));
PUBLIC  void fn( Dir_TidyServer,            (Conode *));
PUBLIC  ObjNode      *fn( Dir_find_node,    (Conode *));
#define Nullfn ((VoidFnPtr) NULL)
PUBLIC  ObjNode      *fn( NewObjNode,       (void));
#define NewDirEntryNode NewObjNode
PUBLIC  void fn( FreeObjNode,               (ObjNode *));
PUBLIC  void fn( Protect_handler,           (Conode *));
PUBLIC  void fn( Refine_handler,            (Conode *));
PUBLIC  void fn( Select_handler,            (Conode *));

/**
*** Some utilities
**/
PUBLIC word fn(  convert_name,    (void));
PUBLIC word fn(  flatten,         (char *));
PUBLIC word fn(  mystrcmp,        (char *, char *));
PUBLIC word fn(  get_unix_time,   (void));
PUBLIC void fn(  NewStream,       (word, word, word, VoidConFnPtr *));
PUBLIC void fn(  AddAttribute,    (Attributes *, Attribute));
PUBLIC void fn(  RemoveAttribute, (Attributes *, Attribute));
PUBLIC void fn(  SetInputSpeed,   (Attributes *, word));
PUBLIC void fn(  SetOutputSpeed,  (Attributes *, word));
PUBLIC void fn(  InitAttributes,  (Attributes *));
PUBLIC void fn(  CopyAttributes,  (Attributes *, Attributes *));
PUBLIC word fn(  IsAnAttribute,   (Attributes *, Attribute));
PUBLIC word fn(  GetInputSpeed,   (Attributes *));
PUBLIC word fn(  GetOutputSpeed,  (Attributes *));
PUBLIC word fn(  Request_Stat,    (void));
PUBLIC void fn(  Request_Return,  (word, word, word));
PUBLIC void fn(  goto_sleep,      (word));
PUBLIC word fn(  FormOpenReply,   (word, word, word, word));
PUBLIC void fn(  copy_event,      (IOEvent *, IOEvent *));
PUBLIC void fn(  pathcat,         (char *, char *));

/**
*** The various options for console/window devices.
**/
PUBLIC void fn( initialise_windowing, (void));
PUBLIC void fn( restore_windowing,    (void));
PUBLIC void fn( restart_windowing,    (void));

#if !(multiple_windows)

#if use_ANSI_emulator

#define output(a)     ANSI_out(a, &Server_window)
#define window_output ANSI_out 
PUBLIC  void fn( ANSI_out, (char *, Window *));

#if PC
PUBLIC  void fn( send_ch, (int));
#endif
#if ST
#define send_ch(x) Bconout(2, x)
#endif

#else /* use_ANSI_emulator */

#define window_output(a, b)  output(a)
PUBLIC void fn( output, (char *));

#endif /* use_ANSI_emulator */

#else /* multiple_windows */

#ifdef __cplusplus
extern "C"
{
#endif
PUBLIC word fn( create_a_window, (char *));
PUBLIC void fn( window_size,     (word, word *, word *));
PUBLIC void fn( close_window,    (word));
PUBLIC void fn( send_to_window,  (char *, Window *));
#ifdef __cplusplus
}
#endif
#if use_ANSI_emulator

PUBLIC  void fn( ANSI_out, (char *, Window *));
#define window_output ANSI_out

#else /* use_ANSI_emulator */

#define window_output send_to_window

#endif /* use_ANSI_emulator     */

#define output(a) window_output(a, &Server_window)

#endif /* multiple_windows */

PUBLIC void fn( ServerDebug, (char *, ...));
PUBLIC void fn( outputch,    (int, Window *));
#if use_ANSI_emulator
PUBLIC word fn( Init_Ansi,   (Window *, word, word));
PUBLIC void fn( Tidy_Ansi,   (Screen *));
PUBLIC void fn( Resize_Ansi, (Window *, word, word, word));
#endif

#if MSWINDOWS
	/* Under windows, printf calls should be replaced by an	*/
	/* error box mechanism.					*/
#define printf err_out
#endif

/**
*** The following declarations are used in header file server.h and module
*** server.c to indicate the functions available on each device.
*** Many functions are no-ops or cannot be implemented satisfactorily, so
*** they are hash-defined to suitable defaults. Also, some devices can
*** share code to handle particular requests.
**/

PUBLIC  void fn( Helios_InitServer, (Conode *));
#define Drive_InitServer            IgnoreVoid
#define Drive_TidyServer            IgnoreVoid
#define Drive_Private               Invalidfn_handler
#define Drive_Testfun               Nullfn
PUBLIC  void fn( Drive_Locate,      (Conode *));
PUBLIC  void fn( Drive_Open,        (Conode *));
PUBLIC  void fn( Drive_Create,      (Conode *));
PUBLIC  void fn( Drive_Delete,      (Conode *));
PUBLIC  void fn( Drive_ObjectInfo,  (Conode *));
PUBLIC  void fn( Drive_ServerInfo,  (Conode *));
PUBLIC  void fn( Drive_Rename,      (Conode *));
PUBLIC  void fn( Drive_Link,        (Conode *));
#define Drive_Protect               Protect_handler
PUBLIC  void fn( Drive_SetDate,     (Conode *));
#define Drive_Refine                Refine_handler
#define Drive_CloseObj              Invalidfn_handler

PUBLIC  word fn( File_InitStream, (Conode *));
PUBLIC  word fn( File_TidyStream, (Conode *));
#define File_PrivateStream        Invalidfn_handler
PUBLIC  void fn( File_Read,       (Conode *));
PUBLIC  void fn( File_Write,      (Conode *));
PUBLIC  void fn( File_Close,      (Conode *));
PUBLIC  void fn( File_GetSize,    (Conode *));
#define File_SetSize              Invalidfn_handler
PUBLIC  void fn( File_Seek,       (Conode *));
#define File_GetAttr              GetDefaultAttr
#define File_SetAttr              NullFn
#define File_EnableEvents         Invalidfn_handler
#define File_Acknowledge          Invalidfn_handler
#define File_NegAcknowledge       Invalidfn_handler
#define File_Select               Select_handler

PUBLIC  word fn( Dir_InitStream,  (Conode *));
PUBLIC  word fn( Dir_TidyStream,  (Conode *));
#define Dir_PrivateStream         Invalidfn_handler
PUBLIC  void fn( Dir_Read,        (Conode *));
#define Dir_Write                 Invalidfn_handler
PUBLIC  void fn( Dir_Close,       (Conode *));
PUBLIC  void fn( Dir_GetSize,     (Conode *));
#define Dir_SetSize               Invalidfn_handler
#define Dir_Seek                  Invalidfn_handler
#define Dir_GetAttr               GetDefaultAttr
#define Dir_SetAttr               NullFn
#define Dir_EnableEvents          Invalidfn_handler
#define Dir_Acknowledge           Invalidfn_handler
#define Dir_NegAcknowledge        Invalidfn_handler
#define Dir_Select                Select_handler

PUBLIC  void fn( IOPROC_InitServer, (Conode *));
PUBLIC  void fn( IOPROC_TidyServer, (Conode *));
PUBLIC  void fn( IOPROC_Private,    (Conode *));
#define IOPROC_Testfun              Nullfn
PUBLIC  void fn( IOPROC_Open,       (Conode *));
PUBLIC  void fn( IOPROC_Locate,     (Conode *));
#define IOPROC_Create               IOPROC_Locate
#define IOPROC_Delete               Forward_handler
PUBLIC  void fn( IOPROC_ObjectInfo, (Conode *));
#define IOPROC_ServerInfo           Forward_handler
PUBLIC  void fn( IOPROC_Rename,     (Conode *));
#define IOPROC_Link                 Forward_handler
#define IOPROC_Protect              Forward_handler
#define IOPROC_SetDate              Forward_handler
#define IOPROC_Refine               Forward_handler
#define IOPROC_CloseObj             Forward_handler

#define IOPROC_InitStream           Ignore
#define IOPROC_TidyStream           Ignore
#define IOPROC_PrivateStream        Invalidfn_handler
#define IOPROC_Read                 Dir_Read
#define IOPROC_Write                Invalidfn_handler
PUBLIC  void fn( IOPROC_Close,      (Conode *));
#define IOPROC_GetSize              Dir_GetSize
#define IOPROC_SetSize              Invalidfn_handler
#define IOPROC_Seek                 Invalidfn_handler
#define IOPROC_GetAttr              GetDefaultAttr
#define IOPROC_SetAttr              NullFn
#define IOPROC_EnableEvents         Invalidfn_handler
#define IOPROC_Acknowledge          IgnoreVoid
#define IOPROC_NegAcknowledge       IgnoreVoid
#define IOPROC_Select               Select_handler

#if multiple_windows
PUBLIC  void fn( Window_InitServer, (Conode *));
PUBLIC  void fn( Window_TidyServer, (Conode *));
#define Window_Private              Invalidfn_handler
PUBLIC  void fn( Window_Testfun,    (bool *));
PUBLIC  void fn( Window_Open,       (Conode *));
#define Window_Locate               Dir_Locate
PUBLIC  void fn( Window_Create,     (Conode *));
PUBLIC  void fn( Window_Delete,     (Conode *));
#define Window_ObjectInfo           Dir_ObjInfo
#define Window_ServerInfo           Invalidfn_handler
#define Window_Rename               Invalidfn_handler
#define Window_Link                 Invalidfn_handler
#define Window_Protect              Protect_handler
#define Window_SetDate              Invalidfn_handler
#define Window_Refine               Refine_handler
#define Window_CloseObj             Invalidfn_handler

#define WindowDir_InitStream        Ignore
#define WindowDir_TidyStream        Ignore
#define WindowDir_PrivateStream     Invalidfn_handler
#define WindowDir_Read              Dir_Read
#define WindowDir_Write             Invalidfn_handler
#define WindowDir_Close             IOPROC_Close
#define WindowDir_GetSize           Dir_GetSize
#define WindowDir_SetSize           Invalidfn_handler
#define WindowDir_Seek              Invalidfn_handler
#define WindowDir_GetAttr           GetDefaultAttr
#define WindowDir_SetAttr           NullFn
#define WindowDir_EnableEvents      Invalidfn_handler
#define WindowDir_Acknowledge       IgnoreVoid
#define WindowDir_NegAcknowledge    IgnoreVoid
#define WindowDir_Select            Select_handler
#endif

/**
*** The console device is always optionally available.
**/

PUBLIC  void fn( Console_InitServer, (Conode *));
#define Console_TidyServer           IgnoreVoid
#define Console_Private              Invalidfn_handler
#if multiple_windows
PUBLIC  void fn( Console_Testfun,    (bool *));
#else
#define Console_Testfun              Nullfn
#endif
PUBLIC  void fn( Console_Open,       (Conode *));
#define Console_Locate               Create_handler
#define Console_Create               Create_handler
#define Console_Delete               Invalidfn_handler
#define Console_ObjectInfo           Device_ObjectInfo_handler
#define Console_ServerInfo           Invalidfn_handler
#define Console_Rename               Invalidfn_handler
#define Console_Link                 Invalidfn_handler
#define Console_Protect              Protect_handler
#define Console_SetDate              Invalidfn_handler
#define Console_Refine               Refine_handler
#define Console_CloseObj             Invalidfn_handler

PUBLIC  word fn( Console_InitStream,   (Conode *));
PUBLIC  word fn( Console_TidyStream,   (Conode *));
#define Console_PrivateStream          Invalidfn_handler
PUBLIC  void fn( Console_Read,         (Conode *));
PUBLIC  void fn( Console_Write,        (Conode *));
PUBLIC  void fn( Console_Close,        (Conode *));
#define Console_GetSize                Device_GetSize
#define Console_SetSize                Invalidfn_handler
#define Console_Seek                   Invalidfn_handler
PUBLIC  void fn( Console_GetAttr,      (Conode *));
PUBLIC  void fn( Console_SetAttr,      (Conode *));
PUBLIC  void fn( Console_EnableEvents, (Conode *));
#define Console_Acknowledge            IgnoreVoid
#define Console_NegAcknowledge         IgnoreVoid
PUBLIC  void fn(Console_Select,	       (Conode*));

PUBLIC  void fn( write_to_log,      (char *));
PUBLIC  void fn( init_logger,       (void));
PUBLIC  void fn( tidy_logger,       (void));

#define Logger_InitServer           IgnoreVoid
#define Logger_TidyServer           IgnoreVoid
#define Logger_Private              Invalidfn_handler
#define Logger_Testfun              Nullfn
PUBLIC  void fn( Logger_Open,       (Conode *));
#define Logger_Locate               Create_handler
#define Logger_Create               Create_handler
PUBLIC  void fn( Logger_Delete,     (Conode *));
PUBLIC  void fn( Logger_ObjectInfo, (Conode *));
#define Logger_ServerInfo           Invalidfn_handler
#define Logger_Rename               Invalidfn_handler
#define Logger_Link                 Invalidfn_handler
#define Logger_Protect              Protect_handler
#define Logger_SetDate              Invalidfn_handler
#define Logger_Refine               Refine_handler
#define Logger_CloseObj             Invalidfn_handler

#define Logger_InitStream             Ignore
#define Logger_TidyStream             Ignore
PUBLIC  void fn( Logger_PrivateStream,(Conode *));
PUBLIC  void fn( Logger_Read,         (Conode *));
PUBLIC  void fn( Logger_Write,        (Conode *));
PUBLIC  void fn( Logger_Close,        (Conode *));
PUBLIC  void fn( Logger_GetSize,      (Conode *));
#define Logger_SetSize                Invalidfn_handler
PUBLIC  void fn( Logger_Seek,         (Conode *));
#define Logger_GetAttr                Invalidfn_handler
#define Logger_SetAttr                Invalidfn_handler
#define Logger_EnableEvents           Invalidfn_handler
#define Logger_Acknowledge            IgnoreVoid
#define Logger_NegAcknowledge         IgnoreVoid
#define Logger_Select                 Select_handler

#define Clock_InitServer           IgnoreVoid
#define Clock_TidyServer           IgnoreVoid
#define Clock_Private              Invalidfn_handler
#define Clock_Testfun              Nullfn
#define Clock_Locate               Create_handler
#define Clock_Open                 Invalidfn_handler
#define Clock_Create               Create_handler
#define Clock_Delete               Invalidfn_handler
PUBLIC  void fn( Clock_ObjectInfo, (Conode *));
#define Clock_ServerInfo           Invalidfn_handler
#define Clock_Rename               Invalidfn_handler
#define Clock_Link                 Invalidfn_handler
#define Clock_Protect              Protect_handler
#if (UNIX || MAC || HELIOS)
#define Clock_SetDate              Invalidfn_handler
#else
PUBLIC  void fn( Clock_SetDate,    (Conode *));
#endif
#define Clock_Refine               Refine_handler
#define Clock_CloseObj             Invalidfn_handler

#if interaction_supported
#define Host_InitServer              IgnoreVoid
#define Host_TidyServer              IgnoreVoid
#define Host_Private                 Invalidfn_handler
#define Host_Testfun                 Nullfn
#define Host_Locate                  Create_handler
PUBLIC  void fn( Host_Open,          (Conode *));
#define Host_Create                  Create_handler
#define Host_Delete                  Invalidfn_handler
#define Host_ObjectInfo              Device_ObjectInfo_handler
#define Host_ServerInfo              Invalidfn_handler
#define Host_Rename                  Invalidfn_handler
#define Host_Link                    Invalidfn_handler
#define Host_Protect                 Protect_handler
#define Host_SetDate                 Invalidfn_handler
#define Host_Refine                  Refine_handler
#define Host_CloseObj                Invalidfn_handler

#define Host_InitStream              Ignore
#define Host_TidyStream              Ignore
PUBLIC  void fn( Host_PrivateStream, (Conode *));
#define Host_Read                    Invalidfn_handler 
#define Host_Write                   Invalidfn_handler
PUBLIC  void fn( Host_Close,         (Conode *));
#define Host_GetSize                 Invalidfn_handler
#define Host_SetSize                 Invalidfn_handler
#define Host_Seek                    Invalidfn_handler
#define Host_GetAttr                 Invalidfn_handler
#define Host_SetAttr                 Invalidfn_handler
#define Host_EnableEvents            Invalidfn_handler
#define Host_Acknowledge             IgnoreVoid
#define Host_NegAcknowledge          IgnoreVoid
#define Host_Select                  Select_handler
#endif /* interaction_supported */

#if (MSWINDOWS)
PUBLIC  void  fn(Graph_InitServer,   (Conode *));
PUBLIC  void  fn(Graph_TidyServer,   (Conode *));
#define Graph_Private                Forward_handler
PUBLIC  void  fn(Graph_Testfun,      (bool *));
PUBLIC  void  fn(Graph_Locate,       (Conode *));
PUBLIC  void  fn(Graph_Open,         (Conode *));
#define Graph_Create		     Graph_Locate
PUBLIC  void  fn(Graph_Delete,       (Conode *));
PUBLIC  void  fn(Graph_ObjectInfo,   (Conode *));
#define Graph_ServerInfo	     Invalidfn_handler
#define Graph_Rename		     Invalidfn_handler
#define Graph_Link		     Invalidfn_handler
#define Graph_Protect		     Protect_handler
#define Graph_SetDate		     Invalidfn_handler
#define Graph_Refine		     Refine_handler
#define Graph_CloseObj 	             Invalidfn_handler

PUBLIC  word fn(Graph_InitStream,    (Conode *));
PUBLIC  word fn(Graph_TidyStream,    (Conode *));
PUBLIC  void fn(Graph_PrivateStream, (Conode *));
PUBLIC  void fn(Graph_Read,          (Conode *));
PUBLIC  void fn(Graph_Write,         (Conode *));
PUBLIC  void fn(Graph_Close,         (Conode *));
#define Graph_GetSize                Device_GetSize
#define Graph_SetSize                Invalidfn_handler
#define Graph_Seek                   Invalidfn_handler
PUBLIC  void fn(Graph_GetAttr,       (Conode *));
PUBLIC  void fn(Graph_SetAttr,       (Conode *));
#define Graph_EnableEvents           Invalidfn_handler
#define Graph_Acknowledge            IgnoreVoid
#define Graph_NegAcknowledge         IgnoreVoid
#define Graph_Select                 Select_handler
#endif

#if gem_supported
PUBLIC  void fn( Gem_InitServer,     (Conode *));
PUBLIC  void fn( Gem_TidyServer,     (Conode *));
#define Gem_Private                  Invalidfn_handler
#if Gem_Always_Available
#define Gem_Testfun                  Nullfn
#else
PUBLIC  void fn( Gem_Testfun,        (bool *));
#endif
#define Gem_Locate                   Create_handler
PUBLIC  void fn( Gem_Open,           (Conode *));
#define Gem_Create                   Create_handler
#define Gem_Delete                   Invalidfn_handler
#define Gem_ObjectInfo               Device_ObjectInfo_handler
#define Gem_ServerInfo               Invalidfn_handler
#define Gem_Rename                   Invalidfn_handler
#define Gem_Link                     Invalidfn_handler
#define Gem_Protect                  Protect_handler
#define Gem_SetDate                  Invalidfn_handler
#define Gem_Refine                   Refine_handler
#define Gem_CloseObj                 Invalidfn_handler

PUBLIC  word fn( Gem_InitStream,     (Conode *));
#define Gem_TidyStream               Ignore
PUBLIC  void fn( Gem_PrivateStream,  (Conode *));
#define Gem_Read                     Invalidfn_handler
#define Gem_Write                    Invalidfn_handler
PUBLIC  void fn( Gem_Close,          (Conode *));
#define Gem_GetSize                  Invalidfn_handler
#define Gem_SetSize                  Invalidfn_handler
#define Gem_Seek                     Invalidfn_handler
#define Gem_GetAttr                  Invalidfn_handler
#define Gem_SetAttr                  Invalidfn_handler
PUBLIC  void fn( Gem_EnableEvents,   (Conode *));
#define Gem_Acknowledge              IgnoreVoid
#define Gem_NegAcknowledge           IgnoreVoid
#define Gem_Select                   Select_handler

PUBLIC void fn( poll_gem,    (void));
PUBLIC void fn( restart_gem, (void));
PUBLIC void fn( vdi,         (int **));
#endif /* gem_supported */

/**
*** The RS232, Centronics Midi and Printer devices share most of their code
**/
#if (Ports_used || Rawdisk_supported)
PUBLIC void fn( Port_Open,       (Conode *));
PUBLIC void fn( Port_TidyServer, (Conode *));
PUBLIC void fn( Port_Close,      (Conode *));
PUBLIC void fn( Port_Rename,     (Conode *));
PUBLIC void fn( Port_Read,       (Conode *));
PUBLIC void fn( Port_Write,      (Conode *));
PUBLIC void fn( Port_GetAttr,    (Conode *));
PUBLIC void fn( Port_SetAttr,    (Conode *));

#define PortDir_InitStream       Ignore
#define PortDir_TidyStream       Ignore
#define PortDir_PrivateStream    Invalidfn_handler
#define PortDir_Read             Dir_Read
#define PortDir_Write            Invalidfn_handler
#define PortDir_Close            IOPROC_Close
#define PortDir_GetSize          Dir_GetSize
#define PortDir_SetSize          Invalidfn_handler
#define PortDir_Seek             Invalidfn_handler
#define PortDir_GetAttr          GetDefaultAttr
#define PortDir_SetAttr          NullFn
#define PortDir_EnableEvents     Invalidfn_handler
#define PortDir_Acknowledge      IgnoreVoid
#define PortDir_NegAcknowledge   IgnoreVoid
#define PortDir_Select           Select_handler

#endif

#if RS232_supported
PUBLIC  void fn( RS232_InitServer,   (Conode *));
PUBLIC  void fn( RS232_TidyServer,   (Conode *));
#define RS232_Private                Invalidfn_handler
#if RS232_Always_Available
#define RS232_Testfun                Nullfn
#else
PUBLIC  void fn( RS232_Testfun,      (bool *));
#endif
#define RS232_Locate                 Dir_Locate
#define RS232_Open                   Port_Open
#define RS232_Create                 Dir_Locate
#define RS232_Delete                 Invalidfn_handler
#define RS232_ObjectInfo             Dir_ObjInfo
#define RS232_ServerInfo             Invalidfn_handler
#define RS232_Rename                 Port_Rename
#define RS232_Link                   Invalidfn_handler
#define RS232_Protect                Protect_handler
#define RS232_SetDate                Invalidfn_handler
#define RS232_Refine                 Refine_handler
#define RS232_CloseObj               Invalidfn_handler
#define RS232_InitStream             Ignore
#define RS232_TidyStream             Ignore
#define RS232_PrivateStream          Invalidfn_handler
#define RS232_Read                   Port_Read
#define RS232_Write                  Port_Write
#define RS232_Close                  Port_Close
#define RS232_GetSize                Device_GetSize
#define RS232_SetSize                Invalidfn_handler
#define RS232_Seek                   Invalidfn_handler
#define RS232_GetAttr                Port_GetAttr
#define RS232_SetAttr                Port_SetAttr
PUBLIC  void fn( RS232_EnableEvents, (Conode *));
#define RS232_Acknowledge            IgnoreVoid
#define RS232_NegAcknowledge         IgnoreVoid
#define RS232_Select                 Select_handler

PUBLIC  word fn( RS232_initlist,       (List *, ComsPort **));
PUBLIC  void fn( RS232_check_events,   (void));
PUBLIC  void fn( RS232_disable_events, (ComsPort *));
PUBLIC  void fn( RS232_enable_events,  (ComsPort *, word, word));
#endif

#if Centronics_supported
PUBLIC  void fn( Centronics_InitServer, (Conode *));
#define Centronics_TidyServer           Port_TidyServer
#define Centronics_Private              Invalidfn_handler
#if Centronics_Always_Available
#define Centronics_Testfun              Nullfn
#else
PUBLIC  void fn( Centronics_Testfun,    (bool *));
#endif
#define Centronics_Locate               Dir_Locate
#define Centronics_Open                 Port_Open
#define Centronics_Create               Dir_Locate
#define Centronics_Delete               Invalidfn_handler
#define Centronics_ObjectInfo           Dir_ObjInfo
#define Centronics_ServerInfo           Invalidfn_handler
#define Centronics_Rename               Port_Rename
#define Centronics_Link                 Invalidfn_handler
#define Centronics_Protect              Protect_handler
#define Centronics_SetDate              Invalidfn_handler
#define Centronics_Refine               Refine_handler
#define Centronics_CloseObj             Invalidfn_handler

#define Centronics_InitStream           Ignore
#define Centronics_TidyStream           Ignore
#define Centronics_PrivateStream        Invalidfn_handler
#if Centronics_readable
#define Centronics_Read                 Port_Read
#else
#define Centronics_Read                 Invalidfn_handler
#endif
#define Centronics_Write                Port_Write
#define Centronics_Close                Port_Close
#define Centronics_GetSize              Device_GetSize
#define Centronics_SetSize              Invalidfn_handler
#define Centronics_Seek                 Invalidfn_handler
#define Centronics_GetAttr              GetDefaultAttr
#define Centronics_SetAttr              NullFn
#define Centronics_EnableEvents         Invalidfn_handler
#define Centronics_Acknowledge          IgnoreVoid
#define Centronics_NegAcknowledge       IgnoreVoid
#define Centronics_Select               Select_handler

PUBLIC  word fn( Centronics_initlist,   (List *, ComsPort **));
#endif

#if Printer_supported
PUBLIC  void fn( Printer_InitServer,    (Conode *));
#define Printer_TidyServer              Port_TidyServer
#define Printer_Private                 Invalidfn_handler
#if Printer_Always_Available
#define Printer_Testfun                 Nullfn
#else
PUBLIC  void fn( Printer_Testfun,       (bool *));
#endif
#define Printer_Locate                  Dir_Locate
#define Printer_Open                    Port_Open
#define Printer_Create                  Dir_Locate
#define Printer_Delete                  Invalidfn_handler
#define Printer_ObjectInfo              Dir_ObjInfo
#define Printer_ServerInfo              Invalidfn_handler
#define Printer_Rename                  Port_Rename
#define Printer_Link                    Invalidfn_handler
#define Printer_Protect                 Protect_handler
#define Printer_SetDate                 Invalidfn_handler
#define Printer_Refine                  Refine_handler
#define Printer_CloseObj                Invalidfn_handler

#define Printer_InitStream              Ignore
#define Printer_TidyStream              Ignore
#define Printer_PrivateStream           Invalidfn_handler
#define Printer_Read                    Invalidfn_handler
#define Printer_Write                   Port_Write
#define Printer_Close                   Port_Close
#define Printer_GetSize                 Device_GetSize
#define Printer_SetSize                 Invalidfn_handler
#define Printer_Seek                    Invalidfn_handler
#define Printer_GetAttr                 Port_GetAttr
#define Printer_SetAttr                 Port_SetAttr
#define Printer_EnableEvents            Invalidfn_handler
#define Printer_Acknowledge             IgnoreVoid
#define Printer_NegAcknowledge          IgnoreVoid
#define Printer_Select                  Select_handler

PUBLIC  word fn( Printer_initlist,      (List *, ComsPort **));
#endif

#if Midi_supported
PUBLIC  void fn( Midi_InitServer,       (Conode *));
#define Midi_TidyServer                 Port_TidyServer
#define Midi_Private                    Invalidfn_handler
#if Midi_Always_Available
#define Midi_Testfun                    Nullfn
#else
PUBLIC  void fn( Midi_Testfun,          (bool *));
#endif
#define Midi_Locate                     Dir_Locate
#define Midi_Open                       Port_Open
#define Midi_Create                     Dir_Locate
#define Midi_Delete                     Invalidfn_handler
#define Midi_ObjectInfo                 Dir_ObjInfo
#define Midi_ServerInfo                 Invalidfn_handler
#define Midi_Rename                     Port_Rename
#define Midi_Link                       Invalidfn_handler
#define Midi_Protect                    Protect_handler
#define Midi_SetDate                    Invalidfn_handler
#define Midi_Refine                     Refine_handler
#define Midi_CloseObj                   Invalidfn_handler

#define Midi_InitStream                 Ignore
#define Midi_TidyStream                 Ignore
#define Midi_PrivateStream              Invalidfn_handler
#define Midi_Read                       Invalidfn_handler
#define Midi_Write                      Port_Write
#define Midi_Close                      Port_Close
#define Midi_GetSize                    Device_GetSize
#define Midi_SetSize                    Invalidfn_handler
#define Midi_Seek                       Invalidfn_handler
#define Midi_GetAttr                    Port_GetAttr
#define Midi_SetAttr                    Port_SetAttr
#define Midi_EnableEvents               Invalidfn_handler
#define Midi_Acknowledge                IgnoreVoid
#define Midi_NegAcknowledge             IgnoreVoid
#define Midi_Select                     Select_handler

PUBLIC  word fn( Midi_initlist,         (List *, ComsPort **));
#endif

#if Ether_supported
#define Ether_InitServer                IgnoreVoid
#define Ether_TidyServer                IgnoreVoid
#define Ether_Private                   Invalidfn_handler
extern  void fn( Ether_Testfun,         (bool *));
extern  void fn( Ether_Open,            (Conode *));
#define Ether_Locate                    Create_handler
#define Ether_Create                    Create_handler
#define Ether_Delete                    Invalidfn_handler
#define Ether_ObjectInfo                Device_ObjectInfo_handler
#define Ether_ServerInfo                Invalidfn_handler
#define Ether_Rename                    Invalidfn_handler
#define Ether_Link                      Invalidfn_handler
#define Ether_Protect                   Invalidfn_handler
#define Ether_SetDate                   Invalidfn_handler
#define Ether_Refine                    Invalidfn_handler
#define Ether_CloseObj                  Invalidfn_handler

#define Ether_InitStream                Ignore
#define Ether_TidyStream                Ignore
#define Ether_PrivateStream             Invalidfn_handler
extern  void fn( Ether_Read,            (Conode *));
extern  void fn( Ether_Write,           (Conode *));
extern  void fn( Ether_Close,           (Conode *));
#define Ether_GetSize                   Invalidfn_handler
#define Ether_SetSize                   Invalidfn_handler
#define Ether_Seek                      Invalidfn_handler
extern  void fn( Ether_GetAttr,         (Conode *));
#define Ether_SetAttr                   Invalidfn_handler
#define Ether_EnableEvents              Invalidfn_handler
#define Ether_Acknowledge               IgnoreVoid
#define Ether_NegAcknowledge            IgnoreVoid
#define Ether_Select                    Select_handler
#endif

#if Rawdisk_supported
PUBLIC  void fn( RawDisk_InitServer,    (Conode *));
#define RawDisk_TidyServer              Dir_TidyServer
#define RawDisk_Private                 Invalidfn_handler
PUBLIC  void fn( RawDisk_Testfun,       (bool *));
PUBLIC  void fn( RawDisk_Open,          (Conode *));
#define RawDisk_Locate                  Dir_Locate
#define RawDisk_Create                  Dir_Locate
#define RawDisk_Delete                  Invalidfn_handler
#define RawDisk_ObjectInfo              Dir_ObjInfo
#define RawDisk_ServerInfo              Invalidfn_handler
#define RawDisk_Rename                  Invalidfn_handler
#define RawDisk_Link                    Invalidfn_handler
#define RawDisk_Protect                 Protect_handler
#define RawDisk_SetDate                 Invalidfn_handler
#define RawDisk_Refine                  Refine_handler
#define RawDisk_CloseObj                Invalidfn_handler

#define RawDisk_InitStream         Ignore
#define RawDisk_TidyStream         Ignore
#define RawDisk_PrivateStream      Invalidfn_handler
PUBLIC  void fn( RawDisk_Read,     (Conode *));
PUBLIC  void fn( RawDisk_Write,    (Conode *));
PUBLIC  void fn( RawDisk_Close,    (Conode *));
PUBLIC  void fn( RawDisk_GetSize,  (Conode *));
#define RawDisk_SetSize            Invalidfn_handler
PUBLIC  void fn( RawDisk_Seek,     (Conode *));
#define RawDisk_GetAttr            Invalidfn_handler
#define RawDisk_SetAttr            Invalidfn_handler
#define RawDisk_EnableEvents       Invalidfn_handler
#define RawDisk_Acknowledge        IgnoreVoid
#define RawDisk_NegAcknowledge     IgnoreVoid
#define RawDisk_Select             Select_handler

#endif
#if Romdisk_supported
PUBLIC  void fn( RomDisk_InitServer,    (Conode *));
#define RomDisk_TidyServer              Dir_TidyServer
#define RomDisk_Private                 Invalidfn_handler
PUBLIC  void fn( RomDisk_Testfun,       (bool *));
PUBLIC  void fn( RomDisk_Open,          (Conode *));
#define RomDisk_Locate                  Dir_Locate
#define RomDisk_Create                  Dir_Locate
#define RomDisk_Delete                  Invalidfn_handler
#define RomDisk_ObjectInfo              Dir_ObjInfo
#define RomDisk_ServerInfo              Invalidfn_handler
#define RomDisk_Rename                  Invalidfn_handler
#define RomDisk_Link                    Invalidfn_handler
#define RomDisk_Protect                 Protect_handler
#define RomDisk_SetDate                 Invalidfn_handler
#define RomDisk_Refine                  Refine_handler
#define RomDisk_CloseObj                Invalidfn_handler

#define RomDisk_InitStream         Ignore
#define RomDisk_TidyStream         Ignore
#define RomDisk_PrivateStream      Invalidfn_handler
PUBLIC  void fn( RomDisk_Read,     (Conode *));
#define RomDisk_Write      	   Invalidfn_handler
PUBLIC  void fn( RomDisk_Close,    (Conode *));
PUBLIC  void fn( RomDisk_GetSize,  (Conode *));
#define RomDisk_SetSize            Invalidfn_handler
#define RomDisk_Seek           	   Invalidfn_handler
#define RomDisk_GetAttr            Invalidfn_handler
#define RomDisk_SetAttr            Invalidfn_handler
#define RomDisk_EnableEvents       Invalidfn_handler
#define RomDisk_Acknowledge        IgnoreVoid
#define RomDisk_NegAcknowledge     IgnoreVoid
#define RomDisk_Select             Select_handler

#endif

#if mouse_supported
PUBLIC  void fn( Mouse_InitServer,      (Conode *));
PUBLIC  void fn( Mouse_TidyServer,      (Conode *));
#define Mouse_Private                   Invalidfn_handler
#if Mouse_Always_Available
#define Mouse_Testfun                   Nullfn
#else                                                                                                                                          
PUBLIC  void fn( Mouse_Testfun,         (bool *));
#endif
PUBLIC  void fn( Mouse_Open,            (Conode *));
#define Mouse_Locate                    Create_handler
#define Mouse_Create                    Create_handler
#define Mouse_Delete                    Invalidfn_handler
#define Mouse_ObjectInfo                Device_ObjectInfo_handler
#define Mouse_ServerInfo                Invalidfn_handler
#define Mouse_Rename                    Invalidfn_handler
#define Mouse_Link                      Invalidfn_handler
#define Mouse_Protect                   Protect_handler
#define Mouse_SetDate                   Invalidfn_handler
#define Mouse_Refine                    Refine_handler
#define Mouse_CloseObj                  Invalidfn_handler

PUBLIC  word fn( Mouse_InitStream,      (Conode *));
PUBLIC  word fn( Mouse_TidyStream,      (Conode *));
#define Mouse_PrivateStream             Invalidfn_handler
#define Mouse_Read                      Invalidfn_handler
#define Mouse_Write                     Invalidfn_handler
PUBLIC  void fn( Mouse_Close,           (Conode *));
#define Mouse_GetSize                   Invalidfn_handler
#define Mouse_SetSize                   Invalidfn_handler
#define Mouse_Seek                      Invalidfn_handler
#define Mouse_GetAttr                   Invalidfn_handler
#define Mouse_SetAttr                   Invalidfn_handler
PUBLIC  void fn( Mouse_EnableEvents,    (Conode *));
PUBLIC  void fn( Mouse_Acknowledge,     (Conode *));
PUBLIC  void fn( Mouse_NegAcknowledge,  (Conode *));
#define Mouse_Select                    Select_handler

PUBLIC  void fn( initialise_mouse,      (void));
PUBLIC  void fn( tidy_mouse,            (void));
PUBLIC  void fn( start_mouse,           (void));
PUBLIC  void fn( stop_mouse,            (void));
#endif

#if keyboard_supported
PUBLIC  void fn( Keyboard_InitServer,     (Conode *));
PUBLIC  void fn( Keyboard_TidyServer,     (Conode *));
#define Keyboard_Private                  Invalidfn_handler
#if Keyboard_Always_Available
#define Keyboard_Testfun                  Nullfn
#else
PUBLIC  void fn( Keyboard_Testfun,        (bool *));
#endif
PUBLIC  void fn( Keyboard_Open,           (Conode *));
#define Keyboard_Locate                   Create_handler
#define Keyboard_Create                   Create_handler
#define Keyboard_Delete                   Invalidfn_handler
#define Keyboard_ObjectInfo               Device_ObjectInfo_handler
#define Keyboard_ServerInfo               Invalidfn_handler
#define Keyboard_Rename                   Invalidfn_handler
#define Keyboard_Link                     Invalidfn_handler
#define Keyboard_Protect                  Protect_handler
#define Keyboard_SetDate                  Invalidfn_handler
#define Keyboard_Refine                   Refine_handler
#define Keyboard_CloseObj                 Invalidfn_handler
PUBLIC  word fn( Keyboard_InitStream,     (Conode *));
PUBLIC  word fn( Keyboard_TidyStream,     (Conode *));
#define Keyboard_PrivateStream            Invalidfn_handler
#define Keyboard_Read                     Invalidfn_handler
#define Keyboard_Write                    Invalidfn_handler
PUBLIC  void fn( Keyboard_Close,          (Conode *));
#define Keyboard_GetSize                  Invalidfn_handler
#define Keyboard_SetSize                  Invalidfn_handler
#define Keyboard_Seek                     Invalidfn_handler
#define Keyboard_GetAttr                  Invalidfn_handler
#define Keyboard_SetAttr                  Invalidfn_handler
PUBLIC  void fn( Keyboard_EnableEvents,   (Conode *));
PUBLIC  void fn( Keyboard_Acknowledge,    (Conode *));
PUBLIC  void fn( Keyboard_NegAcknowledge, (Conode *));
#define Keyboard_Select                   Select_handler

PUBLIC  void fn( initialise_keyboard,     (void));
PUBLIC  void fn( tidy_keyboard,           (void));
PUBLIC  void fn( start_keyboard,          (void));
PUBLIC  void fn( stop_keyboard,           (void));
#endif

#if X_supported
PUBLIC  void fn( X_InitServer, (Conode *));
PUBLIC  void fn( X_TidyServer, (Conode *));
#define X_Private              Invalidfn_handler
PUBLIC  void fn( X_Testfun,    (bool *));
PUBLIC  void fn( X_Open,       (Conode *));
#define X_Locate               Dir_Locate
PUBLIC  void fn( X_Create,     (Conode *));
PUBLIC  void fn( X_Delete,     (Conode *));
#define X_ObjectInfo           Dir_ObjInfo
#define X_ServerInfo           Invalidfn_handler
#define X_Rename               Invalidfn_handler
#define X_Link                 Invalidfn_handler
#define X_Protect              Protect_handler
#define X_SetDate              Invalidfn_handler
#define X_Refine               Refine_handler
#define X_CloseObj             Invalidfn_handler

#define XDir_InitStream        Ignore
#define XDir_TidyStream        Ignore
#define XDir_PrivateStream     Invalidfn_handler
#define XDir_Read              Dir_Read
#define XDir_Write             Invalidfn_handler
#define XDir_Close             IOPROC_Close
#define XDir_GetSize           Dir_GetSize
#define XDir_SetSize           Invalidfn_handler
#define XDir_Seek              Invalidfn_handler
#define XDir_GetAttr           GetDefaultAttr
#define XDir_SetAttr           NullFn
#define XDir_EnableEvents      Invalidfn_handler
#define XDir_Acknowledge       IgnoreVoid
#define XDir_NegAcknowledge    IgnoreVoid
#define XDir_Select            Select_handler

PUBLIC  word fn( X_InitStream,   (Conode *));
PUBLIC  word fn( X_TidyStream,   (Conode *));
#define X_PrivateStream          Invalidfn_handler
PUBLIC  void fn( X_Read,         (Conode *));
PUBLIC  void fn( X_Write,        (Conode *));
PUBLIC  void fn( X_Close,        (Conode *));
#define X_GetSize                Device_GetSize
#define X_SetSize                Invalidfn_handler
#define X_Seek                   Invalidfn_handler
#define X_GetAttr                GetDefaultAttr
#define X_SetAttr                NullFn
#define X_EnableEvents           Invalidfn_handler
#define X_Acknowledge            IgnoreVoid
#define X_NegAcknowledge         IgnoreVoid
#define X_Select                 Select_handler

#endif

#if Network_supported
#define Network_InitServer              IgnoreVoid
#define Network_TidyServer              IgnoreVoid
#define Network_Private                 Invalidfn_handler
#define Network_Testfun                 Nullfn
#define Network_Locate                  Create_handler
PUBLIC  void fn( Network_Open,          (Conode *));
#define Network_Create                  Create_handler
#define Network_Delete                  Invalidfn_handler
#define Network_ObjectInfo              Device_ObjectInfo_handler
#define Network_ServerInfo              Invalidfn_handler
#define Network_Rename                  Invalidfn_handler
#define Network_Link                    Invalidfn_handler
#define Network_Protect                 Protect_handler
#define Network_SetDate                 Invalidfn_handler
#define Network_Refine                  Refine_handler
#define Network_CloseObj                Invalidfn_handler

#define Network_InitStream              Ignore
#define Network_TidyStream              Ignore
PUBLIC  void fn( Network_PrivateStream, (Conode *));
#define Network_Read                    Invalidfn_handler 
#define Network_Write                   Invalidfn_handler
PUBLIC  void fn( Network_Close,         (Conode *));
#define Network_GetSize                 Invalidfn_handler
#define Network_SetSize                 Invalidfn_handler
#define Network_Seek                    Invalidfn_handler
#define Network_GetAttr                 Invalidfn_handler
#define Network_SetAttr                 Invalidfn_handler
#define Network_EnableEvents            Invalidfn_handler
#define Network_Acknowledge             IgnoreVoid
#define Network_NegAcknowledge          IgnoreVoid
#define Network_Select                  Select_handler

#endif /* Network_supported */

#if internet_supported
PUBLIC  void fn( Internet_InitServer, (Conode *));
PUBLIC  void fn( Internet_TidyServer, (Conode *));
#define Internet_Private              Internet_PrivateStream
#define Internet_Testfun              Nullfn
PUBLIC  void fn( Internet_Open,       (Conode *));
#define Internet_Create               Invalidfn_handler
#define Internet_Locate               Dir_Locate
PUBLIC  void fn( Internet_ObjectInfo, (Conode *));
#define Internet_ServerInfo           Invalidfn_handler
#define Internet_Delete               Invalidfn_handler
#define Internet_Rename               Invalidfn_handler
#define Internet_Link                 Invalidfn_handler
#define Internet_Protect              Protect_handler
#define Internet_SetDate              Invalidfn_handler
#define Internet_Refine               Refine_handler
#define Internet_CloseObj             Invalidfn_handler
#define Internet_Revoke               Invalidfn_handler

#define InternetDir_InitStream        Ignore
#define InternetDir_TidyStream        Ignore
#define InternetDir_PrivateStream     Invalidfn_handler
#define InternetDir_Read              Dir_Read
#define InternetDir_Write             Invalidfn_handler
#define InternetDir_Close             IOPROC_Close
#define InternetDir_GetSize           Dir_GetSize
#define InternetDir_SetSize           Invalidfn_handler
#define InternetDir_Seek              Invalidfn_handler
#define InternetDir_GetAttr           GetDefaultAttr
#define InternetDir_SetAttr           NullFn
#define InternetDir_EnableEvents      Invalidfn_handler
#define InternetDir_Acknowledge       IgnoreVoid
#define InternetDir_NegAcknowledge    IgnoreVoid
#define InternetDir_Select            Select_handler

PUBLIC  word fn( Internet_InitStream,   (Conode *));
#define Internet_TidyStream             Ignore
PUBLIC  void fn( Internet_PrivateStream,(Conode *));
PUBLIC  void fn( Internet_Read,         (Conode *));
PUBLIC  void fn( Internet_Write,        (Conode *));
PUBLIC  void fn( Internet_Close,        (Conode *));
PUBLIC  void fn( Internet_GetSize,      (Conode *));
PUBLIC  void fn( Internet_SetSize,      (Conode *));
#define Internet_Seek                   Invalidfn_handler
PUBLIC  void fn( Internet_GetInfo,      (Conode *));
PUBLIC  void fn( Internet_SetInfo,      (Conode *));
#define Internet_EnableEvents           Invalidfn_handler
#define Internet_Acknowledge            IgnoreVoid
#define Internet_NegAcknowledge         IgnoreVoid
PUBLIC  void fn( Internet_Select,       (Conode *));
#endif

/**
*** Explicit declarations of the memory allocation functions
***
*** For most of the hardware on which the server is expected to run malloc()
*** takes an unsigned integer as argument, which may limit it to 64K. The main
*** server sources never need buffers more than 64K, so that is fine.
***
*** If the system's memory allocation is unsatisfactory for one reason or
*** another, I have written my own. See st/stlocal.c and ibm/pclocal.c for
*** details.
**/

#if use_own_memory_management
PUBLIC void  fn(  initialise_memory, (void));
PUBLIC char *fn(  get_mem,           (uint));
PUBLIC void  fn(  free_mem,          (char *));
PUBLIC void  fn(  memory_map,        (void));       /* a debugging facility */
#define malloc(a)  get_mem(a)
#define free(a)    free_mem((char *) (a))

#else

#if !(SOLARIS || SUN3 || SUN4)

/* free/malloc declared in <stdlib.h> */

#if !(PC || MAC || HELIOS || RS6000)     /* The PC's header files declare malloc */
#if !(AMIGA)
PUBLIC char *fn( malloc,      (uint));
/* PUBLIC void fn( free,         (char *)); */	/* now included via <stdlib.h> */
#else
PUBLIC char *fn( malloc,      (uint));
PUBLIC void fn( free,         (void *));
#endif
#endif

#endif /* !SOLARIS */

#endif /* use_own_memory_management */

/**
*** Here are some functions to deal with the multi-tasking support.
*** The only implementation I have so far is for the Sun
**/
#if multi_tasking
/**
*** These routines are called when the Server starts-up and leaves
*** server mode, and during restarts. 
**/
PUBLIC void fn( InitMultiwait,    (void));
PUBLIC void fn( RestartMultiwait, (void));
PUBLIC void fn( TidyMultiwait,    (void));
/**
*** This routine is called from inside the Server's main loop. It should
*** return when one of the I/O's the Server is currently waiting for is
*** possible, or after 1/2 a second to allow the Server to deal correctly
*** with timeouts.
**/
PUBLIC word fn( Multiwait, (void));
/**
*** This routine is called by the Server whenever it is waiting on some
*** form of I/O. The first argument is one of the constants defined in
*** structs.h, specifying the particular form of I/O. The second argument
*** is a pointer to an integer. Whenever the MultiWait is satisfied for the
*** event, that integer should be zapped to the value CoReady. Additional
*** arguments are available if required, e.g. to specify a particular
*** file descriptor.
**/
/**
*** ClearMultiwait() is just the inverse of AddMultiwait()
**/

#ifdef __cplusplus
extern "C"
{
#endif

#if (RS6000 || HP9000 || SCOUNIX)

PUBLIC void fn (AddMultiwait, (WORD, WORD *, ...));
PUBLIC void fn( ClearMultiwait, (WORD, ...));

#else

PUBLIC void fn (AddMultiwait, (long, long *, long, long));
PUBLIC void fn (ClearMultiwait, (long, long, long));

#endif

#ifdef __cplusplus
}
#endif

#endif

/**
*** Bits that had been left out before.
**/
PUBLIC  void fn( initialise_devices,      (void));
PUBLIC  void fn( initialise_files,        (void));
PUBLIC  void fn( restart_devices,         (void));
PUBLIC  void fn( restore_devices,         (void));
PUBLIC  void fn( poll_the_devices,        (void));
PUBLIC  int  fn( read_char_from_keyboard, (word));
PUBLIC  int  fn( init_boot,               (void));
PUBLIC  void fn( boot_processor,          (int));
PUBLIC  void fn( tidy_boot,               (void));
PUBLIC  void fn( init_main_message,       (void));
PUBLIC  void fn( free_main_message,       (void));
PUBLIC  word fn( GetMsg,                  (MCB *));
PUBLIC  word fn( PutMsg,                  (MCB *));
#if debugger_incorporated
PUBLIC  void fn( init_debug,              (void));
PUBLIC  int  fn( debug,                   (void));
PUBLIC  void fn( tidy_debug,              (void));
#endif

#if floppies_available
PUBLIC  word fn( format_floppy, (char *, word, word, word, char *));
#endif

PUBLIC  int  fn( loadimage,               (void));
PUBLIC  void fn( resetlnk,                (void));
PUBLIC  void fn( xpreset,                 (void));
PUBLIC  void fn( xpanalyse,               (void));
PUBLIC  word fn( xpwrbyte,                (word));
PUBLIC  word fn( xpwrrdy,                 (void));
PUBLIC  word fn( xprdrdy,                 (void));
PUBLIC  word fn( xpwrword,                (word));
PUBLIC  word fn( xpwrint,                 (word));
PUBLIC  word fn( xpwrdata,                (byte *, word));
PUBLIC  word fn( xprddata,                (byte *, word));
PUBLIC  word fn( dbwrword,                (word, word));
PUBLIC  word fn( dbwrint,                 (word, word));
PUBLIC  word fn( xprdint,                 (void));
PUBLIC  word fn( xprdword,                (void));
PUBLIC  word fn( xprdbyte,                (void));
PUBLIC  word fn( dbrdword,                (word));
PUBLIC  word fn( dbrdint,                 (word));

#if (PC || SUN3 || SUN4 || ARMBSD)
/* VLSI PID support */
PUBLIC  void fn( vy86pid_set_baudrate,    (word, bool));
PUBLIC  word fn( vy86pid_get_configbaud,  (void));
PUBLIC  void fn( vy86pid_setup_handshake, (void));
#endif

#endif /* Daemon_Module */

