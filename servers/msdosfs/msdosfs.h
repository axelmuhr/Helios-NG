/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- msdosfs.h							--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

#include <codes.h>
#include <ctype.h>
#include <device.h>
#include <dev/fdcdev.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#ifdef __HELIOSTRAN
#define __HELIOSARM
#endif
#include <abcARM/fproto.h>
#ifdef __HELIOSTRAN
#undef __HELIOSARM
#endif
#include <gsp.h>
#include <helios.h>
#include <nonansi.h>
#include <posix.h>
#include <servlib.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

/*======================================================================*/
/*				CONSTANTS				*/
/*======================================================================*/

#define u_int unsigned int

#ifndef TRUE
  #define TRUE 		-1
  #define FALSE		0
#endif

#define Name_Max 	256
#define MAX_BUF 	20	 /* no of buffers in memory */

#define MAX_SECTOR_SIZE	1024

#ifdef debug
  #define MAX_TIME_ACCESS (240*OneSec)
#else
  #define MAX_TIME_ACCESS (180*OneSec)
#endif

#define Type_Nothing	 0x20  /* when object doesn't exist */

#define ReplyOK	        0
#define Command_Max     512
#define Console_Max     40
#define preserve	1
#define release		2
#define Stacksize	5000

#define Message_Limit 	10240		/* size of read buffer */
#define NCYLS		80
#define no_of_drives	1
#define Drive_Link	InvalidFun
#define Drive_Protect	InvalidFun
#define Drive_Refine	InvalidFun
#define MESSAGE_SIZE	80		/* size of IOdebug message */
#define MAX_DIR 	30 		/* max size of a dir, in sectors */
#define MAX_FAT		8 		/* max size of a FAT, in sectors */
#define FIRST_ROOT	0 		/* first cluster of root dir */
#define WRITE_ALL_FAT   -2 		/* all the fat must be rewritten */
#define MAX_RETRY 	3 /* max retry for flop_read & write */
#define Min(x,y) 	((x<y)?x:y)
#define DEFAULTPART 	0

#define	FSCK		0x08		/* disk checking flag */
#define	FSCK_WRITE	0x10		/* disk checking & correcting flag */

#define BAD_FAT		0x00000001	/* bad fat flag */
#define BAD_FAT_FIXED	0x00000002	/* bad fat fixed */
#define BAD_DIR		0x00000004	/* bad fat fixed */
#define BAD_CHKDSK	0x00000008	/* bad fat fixed */

/* transform a short in format high,low to short format low,high
** (DOS and Unix are inverted)
*/

#define stoS(s) ((u_short)(s[1]*256 + s[0]))

#define try_and_free(x) if(x) Free(x)

/*======================================================================*/
/*				STRUCTURES				*/
/*======================================================================*/

typedef struct Message {
	MCB	mcb;
	WORD	control[IOCMsgMax];
	BYTE	data[IOCDataMax];
} Message;

typedef struct FileInfo {
	char	Name[Name_Max];
	int	Pos;		/* position in the file */
	int	Use;		/* # of use of this file */
	int     Attrib; 	/* access type (read, write,..) */
 	WORD	Size;   	/* size in bytes */
	int	Cluster; 	/* first cluster of file */
 	int	DirCluster;   	/* first cluster of parent dir */
	int	NumEntry;  	/* num of entry in the dir for that file */
	time_t	Time;		/* unix time (seconds since 1970).*/
	Port	Port;   	/* ?? */
} FileInfo;

typedef struct DirStream {
	WORD	 number;
	WORD	 offset;
	BYTE	 entries[1];
} DirStream;


typedef struct MyDevice {    	/* 'dispatchInfo structure' for floppy */
	Port		Port;
	char		Name[32];	/* e.g. helios or console */
	VoidFnPtr	Handlers[12];
} MyDevice;


typedef struct SectorBuf	/* Definition of structure of sector cache */
{
	int Sector;	   	/* physical sector number */
	word LastUse;	   	/* to optimise buffer freeing */
	char *Buf;	   	/* contents of sector */
}SectorBuf;

typedef struct Boot 	   	/* structure of boot block */
{
   u_char Saut[3];
   char   Oem[8];
   u_char SectorSize[2]; 
   u_char ClusterSize;     	/* sectors per cluster */
   u_char BootSize[2];     	/* reserved sectors  */
   u_char NbFat;	   	/* no of fat (in general 2) */
   u_char NbEntry[2];	   	/* no of entries in root dirctory */
   u_char NbSector[2];	   	/* total sectors on disc */
   u_char DiskType;        	/* type of disc: FF = single sided
					    FE = double sided
					    FD = 8 sect/track
					    FC = 9 sect/track */
   u_char   FatSize[2];	   	/* no of sectors for fat */
   u_char   SectorTrack[2];	/* no of sectors per track */
   u_char   NbHead[2]; 	   	/* no of heads */
   u_char   HideSector[2];	/* no of hidden sectors */
}Boot;

typedef struct DosDirEntry	/* structure of a directory entry */
{
	u_char 	Name[8],
		Ext[3],
		Attrib,		/* 0x10 if dir, 0 or 0x20 if file */
		Reserved[10],
		Time[2],
		Date[2],
		Cluster[2],	/* first cluster in chain */
		Size[4];
}DosDirEntry;

typedef struct servinfo { WORD	type;
			  WORD	size;
			  WORD	available;
			  WORD	alloc;
} servinfo;

/*======================================================================*/
/*				VARIABLES 				*/
/*======================================================================*/

extern	BYTE		Machine_Name[256];
extern	DCB		*dcb;
extern	Date		Last_time;
extern	Date		Server_StartTime;
extern	DiscDevInfo	discinfo;
extern	DriveInfo	driveinfo;
extern	MyDevice	floppy;
extern	Object		*name_table_entry;
extern	PartitionInfo	partinfo;
extern	SectorBuf	Sector_buf[MAX_BUF];
extern	Semaphore	Time_access;
extern	Semaphore	Var_access;
extern	WORD		Host;
extern	char		*Fat;
extern	int		Cluster_byte;
extern	int		Cluster_size;
extern	int		Dir_size;
extern	int 		Dir_start;
extern	int		Fat_size;
extern	int		Good_fat;
extern	int		Nb_entries;
extern	int		Nb_fat;	
extern	int	 	Nb_sector;
extern	int		Sector_entries;
extern	int		Sector_size;
extern	int		num_worker;
extern	struct sigaction act;
