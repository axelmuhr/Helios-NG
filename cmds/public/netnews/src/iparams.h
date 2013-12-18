/*
 * iparams - parameters for inews.
 */

/*	@(#)iparams.h	2.18	11/19/87	*/

#include "params.h"
#include <errno.h>
extern int errno;

/* external declarations specific to inews */
extern	char	nbuf[LBUFLEN], *ARTICLE, *INFILE, *ALIASES, *PARTIAL;
#ifndef ROOTID
extern	int	ROOTID;
#endif

#ifdef NOTIFY
extern	char	*TELLME;
#endif /* NOTIFY */

#ifdef NFSCLIENT
extern	char	*NFSSYSNAME;
#endif /* NFSCLIENT */

struct msgtype {
	char *m_name;
	char *m_who_to;
	int (*m_func)();
};

extern struct msgtype msgtype[];

extern	FILE	*infp, *actfp;
extern	int	tty, is_ctl, is_mod_file_okay;
extern	char	filename[BUFLEN], is_mod[NAMELEN], not_here[SBUFLEN], *DFLTNG;
