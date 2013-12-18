/* sys/file.h: BSD compatibility header					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: file.h,v 1.2 90/10/16 10:46:50 nick Exp $ */

#ifdef _BSD

#include <fcntl.h>
#include <unistd.h>

#define L_SET	SEEK_SET
#define L_INCR	SEEK_CUR
#define L_XTND	SEEK_END

#define FWRITE	O_WRONLY
#define FREAD	O_RDONLY

#else
#error sys/file.h included without _BSD set
#endif
