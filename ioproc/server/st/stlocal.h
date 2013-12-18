/*------------------------------------------------------------------------
--									--
--          H E L I O S   I N P U T / O U T P U T   S E R V E R	        --
--          ---------------------------------------------------      	--
--									--
--		Copyright (C) 1987, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	stlocal.h          						--
--									--
--	Author:  BLV 22/5/88						--
--									--
------------------------------------------------------------------------*/
/* SccsId: 3.7 14/4/89\ Copyright (C) 1987, Perihelion Software Ltd.	*/

/**
*** This stuff is needed to handle all the various ST keyboards sensibly.
**/
#define country_normal  1
#define country_germany 2
#define country_spain   3
#define country_france  4

#ifdef Devices_Module
int	country = country_normal;
#else
extern int country;
#endif

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

#define OpenMode_ReadOnly	0L
#define OpenMode_WriteOnly	1L
#define OpenMode_ReadWrite	2L

#define SEEK_SET                0
#define SEEK_CUR                1
#define SEEK_END                2

typedef struct { char          junk[21];   /* MSdos private info */
                 BYTE          attr;       /* file attributes */
                 unsigned int  time;       /* time stamp */
                 unsigned int  date;       /* date stamp */
                 WORD          size;       /* length in bytes */
                 char          name[13];
} searchinfo;

#ifdef Files_Module
searchinfo searchbuffer;
#else
extern searchinfo searchbuffer;
#endif

PUBLIC WORD fn( get_drives, (WORD *));

typedef int size_t;

