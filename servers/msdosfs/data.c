/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- data.c							--
--                                                              --
--	The data definitions for MSDOS server			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /giga/HeliosRoot/Helios/servers/msdosfs/RCS/data.c,v 1.1 91/03/07 11:35:18 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*======================================================================*/
/*				VARIABLES				*/
/*======================================================================*/

WORD Host; /* host system type (Unix,PC,..) */
BYTE Machine_Name[256];

DiscDevInfo discinfo;
PartitionInfo partinfo;
DriveInfo driveinfo;
DCB *dcb;
struct sigaction act;

/* re-init semaphore */

Semaphore	Time_access;
Date Last_time;

/** semaphore for Fat and other specific disk stuff
**/

  Semaphore	Var_access;

  char *Fat;

/** sector cache */
  
  SectorBuf	Sector_buf[MAX_BUF];

/** disc information - set up by reload()
**/


  int		Cluster_size;	/* # sectors per cluster*/
  int		Cluster_byte;	/* # bytes per cluster	*/
  int		Nb_fat;		/* # fat (1 or 2)	*/
  int		Good_fat;	/* # fat (0 or 1) that's ok */
  int		Fat_size;	/* # sectors per fat	*/
  int		Nb_entries;	/* # entrys in root dir */
  int	 	Nb_sector;	/* # sectors on disk	*/
  int 		Dir_start;	/* # first sector of root dir */
  int		Dir_size;	/* # sectors in root dir*/
  int		Sector_size;	/* # byte in sector */
  int		Sector_entries;	/* # entries in sector */


#ifdef debug
  int num_worker = 0;   /* identify the workers, for debug */
#endif
 

Object *name_table_entry = Null(Object);

Date		Server_StartTime;

	/** pseudo dispatchInfo structure variable for floppy **/

MyDevice floppy = {
	NullPort,
	"dos",
	{   	Drive_Open,
		Drive_Create,
		Drive_Locate,
		Drive_ObjInfo,
		Drive_ServerInfo,
		Drive_Delete,
		Drive_Rename,
		Drive_Link,
		Drive_Protect,
		Drive_SetDate,
		Drive_Refine,
		NullFn
	}
   
};
