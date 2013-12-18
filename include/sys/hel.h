/* hel.h: Definitions for HELIOS IPC domain.				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: hel.h,v 1.1 90/09/05 11:09:16 nick Exp $ */

struct	sockaddr_hel {
	short	sh_family;		/* AF_HELIOS */
	char	sh_path[109];		/* path name (gag) */
};
