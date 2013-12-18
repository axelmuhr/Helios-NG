/* dirent.h : Posix Library directory entry header		*/
/* %W% %G% (C) Copyright 1990, Perihelion Software		*/
/* $Id: dirent.h,v 1.1 90/09/05 11:06:22 nick Exp $ */

#ifndef _dirent_h
#define _dirent_h

#include <limits.h>

/* Directory entry structure, corresponds exactly to a Helios	*/
/* DirEntry structure. DO NOT ALTER.				*/
struct dirent
{
	long		d_type;
	long		d_flags;
	long		d_matrix;
	char		d_name[NAME_MAX+1];
};

#define DIRBUFSIZE	10

typedef struct _dirdesc
{
	int		dd_fd;			/* file descriptor	*/
	long		dd_loc;			/* offset into buffer	*/
	long		dd_size;		/* no of entries in buf */
	long		dd_pos;			/* pos of buf[0]	*/
	struct dirent	dd_buf[DIRBUFSIZE];
} DIR;

extern DIR 		*opendir(char *pathname);
extern struct dirent 	*readdir(DIR *dir);
extern void 		rewinddir(DIR *dir);
extern int 		closedir(DIR *dir);

#endif

/* end of dirent.h */
