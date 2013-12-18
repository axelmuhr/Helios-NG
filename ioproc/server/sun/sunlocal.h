/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  sunlocal.h                                                          --
--                                                                      --
--  Author:  DJCH (Bath University), BLV                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1987, Perihelion Software Ltd.        */

#ifndef Files_Module
extern struct stat searchbuffer;
#endif

#if ANSI_prototypes
extern void socket_read (int, BYTE *, int, char *);
extern void socket_write (int, BYTE *, int, char *);
#else
extern void socket_write();
extern void socket_read();
#endif

#ifdef Local_Module
#if ANSI_prototypes
static  void pipe_broken(char *);
#else
static  void pipe_broken();
#endif
#endif

#ifdef CLK_TCK
#undef CLK_TCK
#endif

#if !(i486V4 || SUN4 || SUN3 || RS6000 || HP9000)
#ifndef __clock_t
#define __clock_t 1
typedef long clock_t;
#endif
#endif

#define CLK_TCK       1	
#define clock() time(NULL)

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

extern char *sys_errlist[];
extern int  sys_nerr;

/**
*** These define the interface between the I/O Server and the
*** server window program
**/ 
#define FUNCTION_CODE	0xFF
#define WINDOW_SIZE     0x01
#define WINDOW_KILL     0x02
#define WINDOW_MESS	0x03
#define WINDOW_PANEL    0x04
#define WINDOW_DIED     0x05

/* third byte for debug function codes */

#define WIN_MEMORY      0x01
#define WIN_RECONF      0x02
#define WIN_MESSAGES	0x03
#define WIN_SEARCH	0x04
#define WIN_OPEN	0x05
#define WIN_CLOSE	0x06
#define IOWIN_NAME	0x07
#define WIN_READ	0x08
#define WIN_BOOT	0x09
#define WIN_KEYBOARD	0x0A
#define WIN_INIT	0x0B
#define WIN_WRITE	0x0C
#define WIN_QUIT	0x0D
#define WIN_GRAPHICS	0x0E
#define WIN_TIMEOUT     0x0F
#define WIN_OPENREPLY   0x10
#define WIN_FILEIO      0x11
#define WIN_DELETE      0x12
#define WIN_DIRECTORY   0x13
#ifdef NEVER
#define WIN_COM         0x15
#define WIN_HARDDISK    0x16
#endif
#define WIN_ALL		0x14
/* Not needed : nopop, listall */
/* ALL and logger are separate */

#define WIN_REBOOT	0x21
#define WIN_DEBUGGER	0x22
#define WIN_STATUS	0x23
#define WIN_EXIT	0x24
#define WIN_LOG_FILE	0x25
#define WIN_LOG_SCREEN	0x26
#define WIN_LOG_BOTH	0x27
#define WIN_DEBUG	0x28

#define WIN_OFF         0x00
#define WIN_ON		0x01
