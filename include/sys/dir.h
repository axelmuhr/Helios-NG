/* sys/dir.h: BSD compatability header					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: dir.h,v 1.1 90/09/05 11:09:10 nick Exp $ */

#ifdef _BSD

#include <dirent.h>
#define direct dirent

#define MAXNAMLEN	NAME_MAX

extern long telldir(DIR *dir);
extern void seekdir(DIR *dir, long pos);

#else
#error sys/dir.h included without _BSD set
#endif
