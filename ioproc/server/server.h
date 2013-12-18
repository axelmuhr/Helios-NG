/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      server.h                                                        --
--                                                                      --
--      Author:  BLV 8/10/87                                            --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: server.h,v 1.13 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.   			*/

#ifndef Daemon_Module

/**
***  The following lines declare all the shared variables in the system. The
***  space for these variables is allocated in module server.c, but all the
***  other modules need to have a declaration of them.
***
***  First come some pointers needed by the coroutine library. These are
***  explained in module server.c. Heliosnode is a pointer to a coroutine
***  list node used to maintain the linked lists in a sensible order.
***
***  mcb is the pointer to the server's message buffer, which is shared by all
***  devices.
***
***  CoCount is used to keep track of coroutine numbering. Each server and
***  stream coroutine has a unique identifier, obtained by incrementing
***  CoCount each time I create something. These identifiers are equivalent
***  to Helios ports, so all ports are unique.
***
***  Time_unit is used to convert host time in units of CLK_TCK per second
***  to Helios time in units of micro seconds. Startup_Time is a Unix time
***  stamp indicating when the system started up, and Now is the current
***  host time. Initial_stamp is a clock_t value corresponding to Startup_time.
***
***  IOname is used to store the result of the name conversion routine, which
***  is applied to all server requests.
***
***  Heliosdir is the name of the special Helios directory specified in
***  the host.con file. The system will try to use /Helios/lib, /Helios/bin
***  etc., and this drive Helios has to be mapped onto one of the local
***  directories identified by Heliosdir.
***
***  Maxdata imposes a limit on the size of the data vector in messsages
***  being passed between the Server and the transputer. It can be set in
***  the configuration file.
***
***  Err_buff is used to output() strings when the Server is running.
***
***  System_image holds a string specifying the system image to be booted
***  into the transputer.
***
***  Exit_jmpbuf is a C jump buffer set up by main() to allow the
***  initialisation routines to exit cleanly. 
***  
***  Special_Reboot is set when the user presses ctrl-shift-F10. Special_Exit
***  is set for ctrl-shift-F9, and Special_Status for ctrl-shift-F8.
***  DebugMode is toggled when the user presses ctrl-shift F7. These flags
***  are checked at regular intervals so that the user can switch systems
***  easily.
***
***  To keep coroutine stack sizes down I provide two miscellaneous buffers of
***  512 bytes each. These are used by e.g. the rename handlers to store file
***  names.
***
***  Next come the arrays for handling devices. All the routines are declared
***  in fundefs.h, and most of them are dummies of one sort or another.
***
***  Floppy_errno, RS232_errno, etc. are error flags, usually set by 
***  interrupt routines when a device error occurs e.g. attempting to
***  write to the floppy drive when there is no disk.
***
***  Device_count is used to keep track of the number of devices in the
***  server. This is used by the memory debugging routine.
***
***  Server_window is the main window, used for Server output in a multiple
***  windowing system and to store the console variables if no multiple
***  windows are available. Server_window_nopop controls the popping-up
***  of this window when pseudo-windows are used.
***
***  real_windows determines whether the windowing system in use involves
***  real multiple windows, e.g. Amiga, Sunview, X, or pseudo-windows,
***  e.g. PC, dumb terminal
***
***  Server_errno is an error variable that may be set by the local routines 
***  to give more accurate error messages. The main server sources always set
***  up a probable error code before any I/O operation, and the local
***  routines can change this error code after failure to reflect the real
***  error.
***
***  number_of_links specifies the number of link adapters which the server
***  has to monitor. The default is 1, to provide upward compatibility with
***  the older versions of the server where there could only ever be one
***  link adapter. current_link identifies the current link, to be used by
***  the link I/O routines. This avoids having to add a new argument to
***  all the link I/O routines to specify the link. link_table is a table of
***  link structures as described in structs.h.
***
***  Multi_nowait is used to indicate that there are coroutines running
***  which MUST be reactivated without a call to Multiwait(), which
***  could suspend the Server for up to half a second according to the
***  spec.
***
***  target_processor is used to indicate what type of processor the server
***  is communicating with.
**/

#ifdef Server_Module
word        target_processor;
List        *WaitingCo, *PollingCo, *SelectCo;
Node        *Heliosnode;
MCB         *mcb;
word        CoCount=1L;             /* counter for coroutine id numbering */
word        time_unit;
time_t      Startup_Time;
clock_t     Now, initial_stamp;
char        IOname[IOCDataMax];
char        *Heliosdir;
word        maxdata;
char        err_buff[256];
char        system_image[80];

#if SOLARIS
jmp_buf *   exit_jmpbuf;
#define	    longjmp_exit	longjmp (*exit_jmpbuf, 1)
#else
jmp_buf     exit_jmpbuf;
#define     longjmp_exit	longjmp (exit_jmpbuf, 1)
#endif

word        Special_Reboot=false, Special_Exit=false, Special_Status=false;
int         DebugMode = 0;
int 	    EnableThatLink = 0;       /* see module tload.c */
UBYTE       *bootstrap = NULL;        /* see module tload.c */
word        bootsize;
byte        misc_buffer1[512], misc_buffer2[512];
int         Device_count;
Window      Server_window;
int         Server_windows_nopop = 0;
#if multiple_windows
DirHeader   Window_List;
int         real_windows = 0;
#endif
word        Server_errno;
int         number_of_links = 1;
int         current_link    = 0;
PRIVATE     Trans_link root_link;
Trans_link  *link_table = &root_link;
int         Multi_nowait = 0;
int         Default_BootLink = 0;
/* C40HalfDuplex flags use of a special protocol to stop HalfDuplex links */
/* blocking.								  */
word       C40HalfDuplex = FALSE;

device_declaration devices[] = 

{ 
  { Type_Directory, DefaultServerName,
     { 
	IOPROC_InitServer, 
	IOPROC_TidyServer,  
	IOPROC_Private,
	(VoidConFnPtr)IOPROC_Testfun,
	IOPROC_Open,       
	IOPROC_Create,      
	IOPROC_Locate,
	IOPROC_ObjectInfo, 
	IOPROC_ServerInfo,  
	IOPROC_Delete,
       	IOPROC_Rename,     
	IOPROC_Link,        
	IOPROC_Protect,
       	IOPROC_SetDate,    
	IOPROC_Refine,      
	IOPROC_CloseObj
      }
  }

#if Romdisk_supported
 ,{ Type_Directory, "romdisk",
     { 
	RomDisk_InitServer,  
	RomDisk_TidyServer,  
	RomDisk_Private,
	(VoidConFnPtr)RomDisk_Testfun,
	RomDisk_Open,        
	RomDisk_Create,      
	RomDisk_Locate,
	RomDisk_ObjectInfo,  
	RomDisk_ServerInfo,  
	RomDisk_Delete,
	RomDisk_Rename,      
	RomDisk_Link,        
	RomDisk_Protect,
	RomDisk_SetDate,     
	RomDisk_Refine,      
	RomDisk_CloseObj
     }
  }
#endif

 ,{ Type_Directory, "helios",
      { 
	Helios_InitServer,  
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,    
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      }
  }

 ,{ Type_File, "logger",
      { 
	Logger_InitServer,  
	Logger_TidyServer,   
	Logger_Private,
	(VoidConFnPtr)Logger_Testfun,
	Logger_Open,        
	Logger_Create,      
	Logger_Locate,
	Logger_ObjectInfo,  
	Logger_ServerInfo,  
	Logger_Delete,    
	Logger_Rename,      
	Logger_Link,        
	Logger_Protect,
	Logger_SetDate,     
	Logger_Refine,      
	Logger_CloseObj
      }
  }

#if !(drives_are_special)
 ,{ Type_Directory, "files",
      { 
	Drive_InitServer,   
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,  
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,  
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      }
  }
#endif

 ,{ Type_File, "console",
     { 
	Console_InitServer,  
	Console_TidyServer,  
	Console_Private,
	(VoidConFnPtr)Console_Testfun,
	Console_Open,        
	Console_Create,      
	Console_Locate,
	Console_ObjectInfo,  
	Console_ServerInfo,  
	Console_Delete,   
	Console_Rename,      
	Console_Link,        
	Console_Protect,
	Console_SetDate,     
	Console_Refine,      
	Console_CloseObj
     }
  }

#if multiple_windows
 ,{ Type_Directory, "window",
     { 
	Window_InitServer,  
	Window_TidyServer,  
	Window_Private,
       	(VoidConFnPtr)Window_Testfun,
	Window_Open,        
	Window_Create,      
	Window_Locate,
	Window_ObjectInfo,  
	Window_ServerInfo,  
	Window_Delete,   
	Window_Rename,      
	Window_Link,        
	Window_Protect,
	Window_SetDate,     
	Window_Refine,      
	Window_CloseObj
     }
  }
#endif

#if Rawdisk_supported
 ,{ Type_Directory, "rawdisk",
     { 
	RawDisk_InitServer,  
	RawDisk_TidyServer,  
	RawDisk_Private,
	(VoidConFnPtr)RawDisk_Testfun,
	RawDisk_Open,        
	RawDisk_Create,      
	RawDisk_Locate,
	RawDisk_ObjectInfo,  
	RawDisk_ServerInfo,  
	RawDisk_Delete,
	RawDisk_Rename,      
	RawDisk_Link,        
	RawDisk_Protect,
	RawDisk_SetDate,     
	RawDisk_Refine,      
	RawDisk_CloseObj
     }
  }
#endif


 ,{ Type_Device, "clock", 
     { 
	Clock_InitServer,  
	Clock_TidyServer,   
	Clock_Private,
	(VoidConFnPtr)Clock_Testfun,
	Clock_Open,        
	Clock_Create,       
	Clock_Locate,
	Clock_ObjectInfo,  
	Clock_ServerInfo,   
	Clock_Delete,
	Clock_Rename,      
	Clock_Link,         
	Clock_Protect,
	Clock_SetDate,     
	Clock_Refine,       
	Clock_CloseObj
     }
  }

#if interaction_supported
 ,{ Type_File, machine_name, 
     { 
	Host_InitServer,  
	Host_TidyServer,   
	Host_Private,
	(VoidConFnPtr)Host_Testfun,
	Host_Open,        
	Host_Create,       
	Host_Locate,
	Host_ObjectInfo,  
	Host_ServerInfo,   
	Host_Delete,
	Host_Rename,      
	Host_Link,         
	Host_Protect,
	Host_SetDate,     
	Host_Refine,       
	Host_CloseObj
     }
  }
#endif

#if RS232_supported

 ,{ 
    Type_Directory, "rs232",
     { 
	RS232_InitServer,  
	RS232_TidyServer,   
	RS232_Private,
	(VoidConFnPtr)RS232_Testfun,
	RS232_Open,        
	RS232_Create,       
	RS232_Locate,
	RS232_ObjectInfo,  
	RS232_ServerInfo,   
	RS232_Delete,
	RS232_Rename,      
	RS232_Link,         
	RS232_Protect,
	RS232_SetDate,     
	RS232_Refine,       
	RS232_CloseObj
     }
  }

#endif

#if Centronics_supported

 ,{
    Type_Directory, "centronics",
     { 
	Centronics_InitServer,  
	Centronics_TidyServer, 
	Centronics_Private,
	(VoidConFnPtr)Centronics_Testfun,
	Centronics_Open,        
	Centronics_Create,     
	Centronics_Locate,
	Centronics_ObjectInfo,  
	Centronics_ServerInfo, 
	Centronics_Delete, 
	Centronics_Rename,      
	Centronics_Link,       
	Centronics_Protect,
	Centronics_SetDate,     
	Centronics_Refine,     
	Centronics_CloseObj
     }
  }

#endif

#if Printer_supported

 ,{ Type_Directory, "printers",
     { 
	Printer_InitServer,     
	Printer_TidyServer,    
	Printer_Private,
	(VoidConFnPtr)Printer_Testfun,
	Printer_Open,           
	Printer_Create,        
	Printer_Locate,
	Printer_ObjectInfo,     
	Printer_ServerInfo,    
	Printer_Delete,
	Printer_Rename,         
	Printer_Link,          
	Printer_Protect,
	Printer_SetDate,        
	Printer_Refine,        
	Printer_CloseObj
     }
  }

#endif

#if Midi_supported

 ,{ Type_Directory, "midi",
     { 
	Midi_InitServer,     
	Midi_TidyServer,    
	Midi_Private,
	(VoidConFnPtr)Midi_Testfun,
	Midi_Open,           
	Midi_Create,        
	Midi_Locate,
	Midi_ObjectInfo,     
	Midi_ServerInfo,    
	Midi_Delete,
	Midi_Rename,         
	Midi_Link,          
	Midi_Protect,
	Midi_SetDate,        
	Midi_Refine,        
	Midi_CloseObj
     }
  }

#endif

#if Ether_supported
  ,{ Type_File, "ether",
      { 
	Ether_InitServer, 
	Ether_TidyServer, 
	Ether_Private,
	(VoidConFnPtr)Ether_Testfun,
	Ether_Open, 
	Ether_Create, 
	Ether_Locate,
	Ether_ObjectInfo, 
	Ether_ServerInfo, 
	Ether_Delete,
	Ether_Rename, 
	Ether_Link, 
	Ether_Protect,
	Ether_SetDate, 
	Ether_Refine, 
	Ether_CloseObj
      }
  }

#endif

#if internet_supported

 ,{ Type_Directory, "internet",
     { 
	Internet_InitServer,    
	Internet_TidyServer,     
	Internet_Private,
	(VoidConFnPtr)Internet_Testfun,
	Internet_Open,          
	Internet_Create,         
	Internet_Locate,
	Internet_ObjectInfo,    
	Internet_ServerInfo,     
	Internet_Delete,
	Internet_Rename,        
	Internet_Link,           
	Internet_Protect,
	Internet_SetDate,       
	Internet_Refine,         
	Internet_CloseObj
     }
  }
#endif

#if mouse_supported

 ,{ Type_File, "mouse",
     { 
	Mouse_InitServer,  
	Mouse_TidyServer,   
	Mouse_Private,
	(VoidConFnPtr)Mouse_Testfun,
	Mouse_Open,        
	Mouse_Create,       
	Mouse_Locate,
	Mouse_ObjectInfo,  
	Mouse_ServerInfo,   
	Mouse_Delete,
	Mouse_Rename,      
	Mouse_Link,         
	Mouse_Protect,
	Mouse_SetDate,     
	Mouse_Refine,       
	Mouse_CloseObj
     }
  }

#endif

#if keyboard_supported

 ,{ Type_File, "keyboard",
     { 
	Keyboard_InitServer,  
	Keyboard_TidyServer, 
	Keyboard_Private,
	(VoidConFnPtr)Keyboard_Testfun,
	Keyboard_Open,        
	Keyboard_Create,     
	Keyboard_Locate,
	Keyboard_ObjectInfo,  
	Keyboard_ServerInfo, 
	Keyboard_Delete,
	Keyboard_Rename,      
	Keyboard_Link,       
	Keyboard_Protect,
	Keyboard_SetDate,     
	Keyboard_Refine,     
	Keyboard_CloseObj
     }
  }

#endif

#if gem_supported

 ,{ Type_File, "gem",  
     { 
	Gem_InitServer,  
	Gem_TidyServer,   
	Gem_Private,
	(VoidConFnPtr)Gem_Testfun,
	Gem_Open,        
	Gem_Create,       
	Gem_Locate,
	Gem_ObjectInfo,  
	Gem_ServerInfo,   
	Gem_Delete,
	Gem_Rename,      
	Gem_Link,         
	Gem_Protect,
	Gem_SetDate,     
	Gem_Refine,       
	Gem_CloseObj
     }
  }
#endif

#if X_supported

 ,{ Type_Directory, "x",
     { 
	X_InitServer,    
	X_TidyServer,     
	X_Private,
	(VoidConFnPtr)X_Testfun,
	X_Open,          
	X_Create,         
	X_Locate,
	X_ObjectInfo,    
	X_ServerInfo,     
	X_Delete,
	X_Rename,        
	X_Link,           
	X_Protect,
	X_SetDate,       
	X_Refine,         
	X_CloseObj
     }
  }
#endif

#if Network_supported

 ,{ Type_File, "NetworkController",
     { 
	Network_InitServer,     
	Network_TidyServer,    
	Network_Private,
	(VoidConFnPtr)Network_Testfun,
	Network_Open,           
	Network_Create,        
	Network_Locate,
	Network_ObjectInfo,     
	Network_ServerInfo,    
	Network_Delete,
	Network_Rename,         
	Network_Link,          
	Network_Protect,
	Network_SetDate,        
	Network_Refine,        
	Network_CloseObj
     }
  }
#endif

#if (MSWINDOWS)
   ,{ Type_Directory, "graphics",
      {
	
	Graph_InitServer,   
	Graph_TidyServer,	
	Graph_Private,
	(VoidConFnPtr)Graph_Testfun,
	Graph_Open,	    
	Graph_Create,	
	Graph_Locate,
	Graph_ObjectInfo,   
	Graph_ServerInfo,	
	Graph_Delete,
	Graph_Rename,	    
	Graph_Link, 	
	Graph_Protect,
	Graph_SetDate,	    
	Graph_Refine,	
	Graph_CloseObj
      }
    }
#endif

 ,{ 0L, NULL,  
     {  (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL
      }
  }

};

#if drives_are_special
VoidConFnPtr Drive_Handlers[handler_max] =
      { 
	Drive_InitServer,   
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      };
#endif

VoidConFnPtr IOPROC_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)IOPROC_InitStream, 
	(VoidConFnPtr)IOPROC_TidyStream,
        IOPROC_PrivateStream,
        IOPROC_Read,        
	IOPROC_Write,          
	IOPROC_GetSize,
	IOPROC_SetSize,     
	IOPROC_Close,          
	IOPROC_Seek,
	IOPROC_GetAttr,     
	IOPROC_SetAttr,        
	IOPROC_EnableEvents,
	IOPROC_Acknowledge, 
	IOPROC_NegAcknowledge, 
	IOPROC_Select
      };

VoidConFnPtr Logger_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Logger_InitStream,  
	(VoidConFnPtr)Logger_TidyStream,
        Logger_PrivateStream,
        Logger_Read,        
	Logger_Write,          
	Logger_GetSize,
	Logger_SetSize,     
	Logger_Close,          
	Logger_Seek,
	Logger_GetAttr,     
	Logger_SetAttr,        
	Logger_EnableEvents,
	Logger_Acknowledge, 
	Logger_NegAcknowledge, 
	Logger_Select
      };


VoidConFnPtr Console_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Console_InitStream,  
	(VoidConFnPtr)Console_TidyStream,
        Console_PrivateStream,
	Console_Read,        
	Console_Write,          
	Console_GetSize,
	Console_SetSize,     
	Console_Close,          
	Console_Seek,
	Console_GetAttr,     
	Console_SetAttr,        
	Console_EnableEvents,
	Console_Acknowledge, 
	Console_NegAcknowledge, 
	Console_Select
      };

#if multiple_windows
VoidConFnPtr WindowDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)WindowDir_InitStream, 
	(VoidConFnPtr)WindowDir_TidyStream,
	WindowDir_PrivateStream,
	WindowDir_Read,         
	WindowDir_Write,        
	WindowDir_GetSize,
	WindowDir_SetSize,      
	WindowDir_Close,        
	WindowDir_Seek,
	WindowDir_GetAttr,      
	WindowDir_SetAttr,      
	WindowDir_EnableEvents,
	WindowDir_Acknowledge,  
	WindowDir_NegAcknowledge, 
	WindowDir_Select
      };
#endif

#if Rawdisk_supported

VoidConFnPtr Rawdisk_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RawDisk_InitStream,  
	(VoidConFnPtr)RawDisk_TidyStream,
	RawDisk_PrivateStream,
	RawDisk_Read,          
	RawDisk_Write,        
	RawDisk_GetSize,
	RawDisk_SetSize,       
	RawDisk_Close,        
	RawDisk_Seek,
	RawDisk_GetAttr,       
	RawDisk_SetAttr,      
	RawDisk_EnableEvents,
	RawDisk_Acknowledge,   
	RawDisk_NegAcknowledge, 
	RawDisk_Select
      };
#endif
#if Romdisk_supported

VoidConFnPtr Romdisk_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RomDisk_InitStream,  
	(VoidConFnPtr)RomDisk_TidyStream,
	RomDisk_PrivateStream,
	RomDisk_Read,          
	RomDisk_Write,        
	RomDisk_GetSize,
	RomDisk_SetSize,       
	RomDisk_Close,        
	RomDisk_Seek,
	RomDisk_GetAttr,       
	RomDisk_SetAttr,      
	RomDisk_EnableEvents,
	RomDisk_Acknowledge,   
	RomDisk_NegAcknowledge, 
	RomDisk_Select
      };
#endif

#if gem_supported
VoidConFnPtr Gem_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Gem_InitStream,  
	(VoidConFnPtr)Gem_TidyStream,
	Gem_PrivateStream,
	Gem_Read,        
	Gem_Write,          
	Gem_GetSize,
	Gem_SetSize,     
	Gem_Close,          
	Gem_Seek,
	Gem_GetAttr,     
	Gem_SetAttr,        
	Gem_EnableEvents,
	Gem_Acknowledge, 
	Gem_NegAcknowledge, 
	Gem_Select
      };
#endif

#if interaction_supported
VoidConFnPtr Host_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Host_InitStream,  
	(VoidConFnPtr)Host_TidyStream,
	Host_PrivateStream,
	Host_Read,        
	Host_Write,          
	Host_GetSize,
	Host_SetSize,     
	Host_Close,          
	Host_Seek,
	Host_GetAttr,     
	Host_SetAttr,        
	Host_EnableEvents,
	Host_Acknowledge, 
	Host_NegAcknowledge, 
	Host_Select
      };
#endif


#if (Ports_used || Rawdisk_supported)
VoidConFnPtr PortDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)PortDir_InitStream, 
	(VoidConFnPtr)PortDir_TidyStream,
	PortDir_PrivateStream,
	PortDir_Read,         
	PortDir_Write,          
	PortDir_GetSize,
	PortDir_SetSize,      
	PortDir_Close,          
	PortDir_Seek,
	PortDir_GetAttr,      
	PortDir_SetAttr,        
	PortDir_EnableEvents,
	PortDir_Acknowledge,  
	PortDir_NegAcknowledge, 
	PortDir_Select
      };
#endif

#if RS232_supported
VoidConFnPtr RS232_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RS232_InitStream, 
	(VoidConFnPtr)RS232_TidyStream,
	RS232_PrivateStream,
	RS232_Read,         
	RS232_Write,          
	RS232_GetSize,
	RS232_SetSize,      
	RS232_Close,          
	RS232_Seek,
	RS232_GetAttr,      
	RS232_SetAttr,        
	RS232_EnableEvents,
	RS232_Acknowledge,  
	RS232_NegAcknowledge, 
	RS232_Select
      };
#endif

#if Centronics_supported
VoidConFnPtr Centronics_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Centronics_InitStream, 
	(VoidConFnPtr)Centronics_TidyStream,
	Centronics_PrivateStream,
	Centronics_Read,        
	Centronics_Write,       
	Centronics_GetSize,
	Centronics_SetSize,     
	Centronics_Close,       
	Centronics_Seek,
	Centronics_GetAttr,     
	Centronics_SetAttr,     
	Centronics_EnableEvents,
	Centronics_Acknowledge, 
	Centronics_NegAcknowledge, 
	Centronics_Select
      };
#endif

#if Printer_supported
VoidConFnPtr Printer_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Printer_InitStream, 
	(VoidConFnPtr)Printer_TidyStream,
	Printer_PrivateStream,
	Printer_Read,           
	Printer_Write,          
	Printer_GetSize,
	Printer_SetSize,        
	Printer_Close,          
	Printer_Seek,
	Printer_GetAttr,        
	Printer_SetAttr,        
	Printer_EnableEvents,
	Printer_Acknowledge,    
	Printer_NegAcknowledge, 
	Printer_Select 
     };
#endif

#if Midi_supported
VoidConFnPtr Midi_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Midi_InitStream, 
	(VoidConFnPtr)Midi_TidyStream,
	Midi_PrivateStream,
	Midi_Read,           
	Midi_Write,          
	Midi_GetSize,
	Midi_SetSize,        
	Midi_Close,          
	Midi_Seek,
	Midi_GetAttr,        
	Midi_SetAttr,        
	Midi_EnableEvents,
	Midi_Acknowledge,    
	Midi_NegAcknowledge, 
	Midi_Select
      };
#endif

#if Ether_supported
VoidConFnPtr Ether_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Ether_InitStream, 
	(VoidConFnPtr)Ether_TidyStream,
	Ether_PrivateStream,
	Ether_Read,          
	Ether_Write,          
	Ether_GetSize,
	Ether_SetSize,       
	Ether_Close,          
	Ether_Seek,
	Ether_GetAttr,       
	Ether_SetAttr,        
	Ether_EnableEvents,
	Ether_Acknowledge,   
	Ether_NegAcknowledge, 
	Ether_Select
     };
#endif

#if mouse_supported
VoidConFnPtr Mouse_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Mouse_InitStream, 
	(VoidConFnPtr)Mouse_TidyStream,
	Mouse_PrivateStream,
	Mouse_Read,         
	Mouse_Write,          
	Mouse_GetSize,
	Mouse_SetSize,      
	Mouse_Close,          
	Mouse_Seek,
	Mouse_GetAttr,      
	Mouse_SetAttr,        
	Mouse_EnableEvents,
	Mouse_Acknowledge,  
	Mouse_NegAcknowledge, 
	Mouse_Select
      };
#endif

#if keyboard_supported
VoidConFnPtr Keyboard_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Keyboard_InitStream, 
	(VoidConFnPtr)Keyboard_TidyStream,
	Keyboard_PrivateStream,
	Keyboard_Read,        
	Keyboard_Write,        
	Keyboard_GetSize,
	Keyboard_SetSize,     
	Keyboard_Close,        
	Keyboard_Seek,
	Keyboard_GetAttr,     
	Keyboard_SetAttr,      
	Keyboard_EnableEvents,
	Keyboard_Acknowledge, 
	Keyboard_NegAcknowledge, 
	Keyboard_Select
      };
#endif

#if X_supported
VoidConFnPtr XDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)XDir_InitStream, 
	(VoidConFnPtr)XDir_TidyStream,
	XDir_PrivateStream,
	XDir_Read,            
	XDir_Write,            
	XDir_GetSize,
	XDir_SetSize,         
	XDir_Close,            
	XDir_Seek,
	XDir_GetAttr,         
	XDir_SetAttr,          
	XDir_EnableEvents,
	XDir_Acknowledge,     
	XDir_NegAcknowledge,   
	XDir_Select
      };

VoidConFnPtr X_Handlers[Stream_max] =
      { (VoidConFnPtr)X_InitStream, 
	(VoidConFnPtr)X_TidyStream,
	X_PrivateStream,
	X_Read,            
	X_Write,            
	X_GetSize,
	X_SetSize,         
	X_Close,            
	X_Seek,
	X_GetAttr,         
	X_SetAttr,          
	X_EnableEvents,
	X_Acknowledge,     
	X_NegAcknowledge,   
	X_Select 
      };

#endif /* X_supported */

#if Network_supported
VoidConFnPtr Network_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Network_InitStream, 
	(VoidConFnPtr)Network_TidyStream,
	Network_PrivateStream,
	Network_Read,        
	Network_Write,          
	Network_GetSize,
	Network_SetSize,     
	Network_Close,          
	Network_Seek,
	Network_GetAttr,     
	Network_SetAttr,        
	Network_EnableEvents,
	Network_Acknowledge, 
	Network_NegAcknowledge, 
	Network_Select
      };
#endif
                
VoidConFnPtr File_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)File_InitStream,  
	(VoidConFnPtr)File_TidyStream,
	File_PrivateStream,
	File_Read,        
	File_Write,          
	File_GetSize,
	File_SetSize,     
	File_Close,          
	File_Seek,
	File_GetAttr,     
	File_SetAttr,        
	File_EnableEvents,
	File_Acknowledge, 
	File_NegAcknowledge, 
	File_Select
      };

VoidConFnPtr Dir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Dir_InitStream, 
	(VoidConFnPtr)Dir_TidyStream,
	Dir_PrivateStream,
	Dir_Read,         
	Dir_Write,          
	Dir_GetSize,
	Dir_SetSize,      
	Dir_Close,          
	Dir_Seek,
	Dir_GetAttr,      
	Dir_SetAttr,        
	Dir_EnableEvents,
	Dir_Acknowledge,  
	Dir_NegAcknowledge, 
	Dir_Select
      };

#if internet_supported
VoidConFnPtr InternetDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)InternetDir_InitStream, 
	(VoidConFnPtr)InternetDir_TidyStream,
	InternetDir_PrivateStream,
	InternetDir_Read,        
	InternetDir_Write,         
	InternetDir_GetSize,
	InternetDir_SetSize,     
	InternetDir_Close,         
	InternetDir_Seek,
	InternetDir_GetAttr,     
	InternetDir_SetAttr,  
	InternetDir_EnableEvents,
	InternetDir_Acknowledge,
	InternetDir_NegAcknowledge,
	InternetDir_Select 
      };

VoidConFnPtr Internet_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Internet_InitStream,  
	(VoidConFnPtr)Internet_TidyStream,
	Internet_PrivateStream,
	Internet_Read,            
	Internet_Write,         
	Internet_GetSize,
	Internet_SetSize,         
	Internet_Close,         
	Internet_Seek,
	Internet_GetInfo,         
	Internet_SetInfo,       
	Internet_EnableEvents,
	Internet_Acknowledge,     
	Internet_NegAcknowledge,
	Internet_Select
      };

#endif /* Internet_supported */

#if (MSWINDOWS)
VoidConFnPtr   Graph_Handlers[Stream_max] =
      {
     	(VoidConFnPtr)Graph_InitStream,   
	(VoidConFnPtr)Graph_TidyStream,
	Graph_PrivateStream,
	Graph_Read,	     
	Graph_Write,	   
	Graph_GetSize,
	Graph_SetSize,	     
	Graph_Close,	   
	Graph_Seek,
	Graph_GetAttr,	     
	Graph_SetAttr,	   
	Graph_EnableEvents,
	Graph_Acknowledge,      
	Graph_NegAcknowledge, 
	Graph_Select
     };
#endif

int Server_Mode = Mode_Normal;

#else       /* declarations for the other modules */

extern word         target_processor;
extern int          Server_Mode;
extern List         *WaitingCo, *PollingCo, *SelectCo;
extern MCB          *mcb;
extern Node         *Heliosnode;
extern word         CoCount;
extern word         time_unit;
extern time_t       Startup_Time;
extern clock_t      Now, initial_stamp;
extern char         IOname[];
extern char         *Heliosdir;
extern char         err_buff[];
extern word         maxdata;
extern char         system_image[];
#if SOLARIS
extern jmp_buf *      	exit_jmpbuf;
#define longjmp_exit	longjmp (*exit_jmpbuf, 1)
#else
extern jmp_buf      	exit_jmpbuf;
#define longjmp_exit	longjmp (exit_jmpbuf, 1)
#endif
extern word         Special_Reboot, Special_Exit, Special_Status;
extern int          DebugMode;
extern int          EnableThatLink;
extern char         *bootstrap;
extern word         bootsize;
extern byte         misc_buffer1[], misc_buffer2[];
extern int          Device_count;
extern Window       Server_window;
extern int          Server_windows_nopop;
#if multiple_windows
extern DirHeader    Window_List;
extern int          real_windows;
#endif
extern word         Server_errno;
extern int          number_of_links, current_link;
extern Trans_link   *link_table;
extern int          Multi_nowait;
extern int          Default_BootLink;
extern word         C40HalfDuplex;

extern VoidConFnPtr Dir_Handlers[];
extern VoidConFnPtr File_Handlers[];
extern VoidConFnPtr IOPROC_Handlers[];
extern VoidConFnPtr Console_Handlers[];
extern VoidConFnPtr Logger_Handlers[];
#if multiple_windows
extern VoidConFnPtr WindowDir_Handlers[];
#endif

#if Rawdisk_supported
extern VoidConFnPtr Rawdisk_Handlers[];
#endif
#if Romdisk_supported
extern VoidConFnPtr Romdisk_Handlers[];
#endif

#if gem_supported
extern VoidConFnPtr Gem_Handlers[];
#endif
#if interaction_supported
extern VoidConFnPtr Host_Handlers[];
#endif
#if (Ports_used || Rawdisk_supported)
extern VoidConFnPtr PortDir_Handlers[];
#endif
#if RS232_supported
extern VoidConFnPtr RS232_Handlers[];
#endif
#if Centronics_supported
extern VoidConFnPtr Centronics_Handlers[];
#endif
#if Printer_supported
extern VoidConFnPtr Printer_Handlers[];
#endif
#if Midi_supported
extern VoidConFnPtr Midi_Handlers[];
#endif
#if Ether_supported
extern VoidConFnPtr Ether_Handlers[];
#endif
#if mouse_supported
extern VoidConFnPtr Mouse_Handlers[];
#endif
#if keyboard_supported
extern VoidConFnPtr Keyboard_Handlers[];
#endif
#if X_supported
extern VoidConFnPtr XDir_Handlers[];
extern VoidConFnPtr X_Handlers[];
#endif
#if Network_supported
extern VoidConFnPtr Network_Handlers[];
#endif
#if internet_supported
extern VoidConFnPtr InternetDir_Handlers[];
extern VoidConFnPtr Internet_Handlers[];
#endif
#if (MSWINDOWS)
extern VoidConFnPtr Graph_Handlers[];
#endif

#endif  /* not Server_Module */

/**
*** Device errors, e.g. writing to a floppy drive that does not contain a
*** disk, have the annoying habit of invoking interrupts rather than
*** producing nice error codes. This gives some problems when I try to
*** handle them in a general way. Roughly, I expect the local routines to
*** set the following error variables to suitable values when a device error
*** occurs, by fair means or foul, and I take care of processing the error.
*** Because they have to be provided by the local routines, I prefer to keep
*** them in the local modules - mainly because of problems with segments on
*** the PC.
**/
#ifndef Local_Module

#if RS232_supported
extern int   RS232_errno;
#endif

#if Centronics_supported
extern int Centronics_errno;
#endif

#if floppies_available
extern int floppy_errno;
#endif

#if Printer_supported
extern int Printer_errno;
#endif

#if Midi_supported
extern int Midi_errno;
#endif

#endif  /* not Local_Module */

#endif /* not daemon module */

