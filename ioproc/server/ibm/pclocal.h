/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  pclocal.h                                                           --
--                                                                      --
--  Author:  BLV 23/5/88                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1987, Perihelion Software Ltd. 			*/

/**
*** Some manifests and declarations needed for the ST and PC filing systems.
***
*** Drive_len is the length of a drive identifier, e.g. C:, and under
*** MSdos and TOS drives have to be treated separately from subdirectories.
*** Hence I need a quick check to see whether a name refers to a drive or
*** to something else, and this check is to compare the length of the local
*** name local_name with drive_len.
***
*** The various search and type manifests are needed with Fsfirst() and
*** Fsnext() system calls. When searching a drive the search_VolLabel is
*** required as an arguments, whereas search_FileOrDir is used with a 
*** subdirectory or a file. The calls put some information about the file
*** or directory into a static searchbuffer, with the structure shown below.
*** One of the fields, attr, can take values of FileAttr_Dir or
*** FileAttr_File.
***
*** Unfortunately I do not know where the search calls put their information
*** normally, so I have to provide my own buffer and indicate this to the
*** system by a call to Setfdta() in module server.c .
***
**/
#define drive_len      2     /* the length of a drive identifier string, d: */

#define search_VolLabel    8
#define search_FileOrDir  16

#define FileAttr_Dir      16
#define FileAttr_File      0

#define OpenMode_ReadOnly   0L
#define OpenMode_WriteOnly  1L
#define OpenMode_ReadWrite  2L

typedef struct { char          junk[21];   /* MSdos private info */
                 byte          attr;       /* file attributes */
                 unsigned int  time;       /* time stamp */
                 unsigned int  date;       /* date stamp */
                 word          size;       /* length in bytes */
                 char          name[13];
} searchinfo;

#ifdef Files_Module
searchinfo searchbuffer;
#else
extern searchinfo searchbuffer;
#endif

PUBLIC word fn( get_drives,       (word *));
PUBLIC word fn( create_file,      (char *));
PUBLIC word fn( get_file_info,    (char *, ObjInfo *));
PUBLIC word fn( rename_object,    (char *, char *));
PUBLIC word fn( set_file_date,    (char *, word));
PUBLIC word fn( get_drive_info,   (char *, servinfo *));
PUBLIC int  fn( open_file,        (char *, word));
PUBLIC word fn( get_file_size,    (int, word));
PUBLIC word fn( write_to_file,    (int, word, byte *));
PUBLIC word fn( read_from_file,   (int, word, byte *));
PUBLIC word fn( search_directory, (char *, List *));
PUBLIC word fn( seek_in_file,     (int, word, int));
extern void check_helios_name(char *);

/**
*** a macro for clear bytes, memset being supported by Microsoft C
**/
#define clear_bytes(a, b) (void) memset((void *) a, 0, (size_t) b)

/**
*** Support for the RS232 lines. This is shared between the /rs232
*** device and the VY86PID link interface routines.
**/
#define MaxRS232     7
#if 1
#define overflow_max 256
#else
#define overflow_max 8192
#endif

typedef struct RS232 { 
	byte		*inbuf;
	byte		*outbuf;
	Attributes	*attr;
	Port		 breakint;
	Port		 modemint;
	int		 id;
	uint		 flags;
	uint		 incount;
	uint		 inmax;
	uint		 outcount;
	uint		 outmax;
	int		 overflow_count;
	int		 port_base;
	int		 current_mcr;
	int		 current_lcr;
	int		 LCR;	/* for restoring	*/
	int		 BAUDH;
	int		 BAUDL;
	int		 IER;
	int		 MCR;
	byte		 overflow_buf[overflow_max];
} RS232;

/**
*** Flags for the Com structure
**/
#define RS232_inXOFF           0x01
#define RS232_outXOFF          0x02
#define RS232_BreakDetected    0x04
#define RS232_UseHardware      0x08
#define RS232_ClearToWrite     0x10
#define RS232_NeedToXoff       0x20
#define RS232_UseInXoff        0x40
#define RS232_UseOutXoff       0x80
#define RS232_StripIn          0x100
#define RS232_IgnorePar        0x200
#define RS232_ParMark          0x400
#define RS232_ParCheck         0x800
#define RS232_NeedToXon        0x1000
#define RS232_TXEmpty          0x2000
#define RS232_CTS              0x4000
#define RS232_RingDetected     0x8000

#define XONByte                0x11
#define XOFFByte               0x13

             /* hardware handshaking support */
#define RTS                    0x02
#define DTR                    0x01
#define CTS                    0x10
#define Ring                   0x40

extern word RS232_send(ComsPort *, word, UBYTE *);
extern word RS232_pollwrite(ComsPort *);
extern word RS232_abortwrite(ComsPort *);
extern word RS232_receive(ComsPort *, word, UBYTE *);
extern word RS232_pollread(ComsPort *);
extern word RS232_abortread(ComsPort *);
extern void RS232_done(ComsPort *);
extern void RS232_configure(ComsPort *);
extern void RS232_error_handler(void);
extern void RS232_control_fifo(ComsPort *, bool);

extern RS232    *RS232_table;
extern ComsPort *RS232_coms;
#define Default_Port	MaxRS232
#define VY86PID_Port	MaxRS232 + 1

#if _MSC_VER <= 600
#define	_segread	segread
#define _REGS		REGS
#define _SREGS		SREGS
#define _int86x		int86x
#define _intdos		intdos
#define _mkdir		mkdir
#define _rmdir		rmdir
#define _lseek		lseek
#endif

#if _MSC_VER > 600
#define	fileno	_fileno
#endif


