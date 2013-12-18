/* sys/un.h: BSD compatibility header					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: un.h,v 1.1 90/09/05 11:09:34 nick Exp $ */

#ifdef _BSD

/*
 * Definitions for UNIX IPC domain.
 */
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[109];		/* path name (gag) */
};

#else
#error sys/un.h included without _BSD set
#endif
