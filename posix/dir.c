/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- dir.c								--
--                                                                      --
--	Directory handling for Posix.					--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: dir.c,v 1.9 1993/07/12 10:35:41 nickc Exp $ */


#include <helios.h>	/* standard header */

#define __in_dir 1	/* flag that we are in this module */


#include <posix.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>

#include "pposix.h"

DIR *opendir( char * pathname )
{
	int e = 0;
	int fd= -1;
	DIR *dir = NULL;
	char *path = (char *)Malloc(PATH_MAX + 1);
	int length = strlen(pathname);

	CHECKSIGS();
	if( path == NULL )
	{ e = ENOMEM; goto done; }
		
	strcpy(path, pathname);
	if ((length > 1) && (path[length - 1] == '/')) path[length - 1] = '\0';
	
	if ((fd = open(path, O_RDONLY)) == -1) {e = errno; goto done; }

	unless ( checkfd(fd)->pstream->stream->Type & Type_Directory )
	{ e = ENOTDIR; goto done; }
	
	if ((dir = (DIR *)Malloc(sizeof(DIR))) == NULL)
	{
		e = ENOMEM;
		goto done;
	}
	else {
		fcntl(fd,F_SETFD,FD_CLOEXEC);		
		dir->dd_fd = fd;
		dir->dd_loc = 0;
		dir->dd_pos = 0;
		dir->dd_size = 0;
	}

done:
	if( path != NULL ) Free(path);
	if( e != 0 )
	{
		if( fd != -1 ) close( fd );
		if( dir != NULL ) Free(dir);
		errno = e;
		dir = NULL;
	}
	CHECKSIGS();
	return dir;
}

struct dirent *readdir( DIR * dir )
{
	int rdsize;

	CHECKSIGS();
	if( dir->dd_loc == dir->dd_size )
	{
		if ( (rdsize=read(dir->dd_fd,(byte *)dir->dd_buf,
				 DIRBUFSIZE*sizeof(struct dirent))) <= 0 )
		{
				return NULL;
		}
		dir->dd_pos += dir->dd_size;
		dir->dd_loc = 0;
		dir->dd_size = (long)rdsize/sizeof(struct dirent);
	}
	
	return &(dir->dd_buf[dir->dd_loc++]);
}

void rewinddir(DIR *dir)
{
	fdentry *fde = checkfd(dir->dd_fd);
	Stream *s = fde->pstream->stream;
	Stream *s1 = CopyStream(s);
	s1->Pos = 0;
	fde->pstream->stream = s1;
	Close(s);
	ReOpen(s1);
	dir->dd_loc = 0;
	dir->dd_pos = 0;
	dir->dd_size = 0;
}

int closedir( DIR * dir )
{
	close(dir->dd_fd);
	Free(dir);

	return 0;
}

/* end of dir.c */
