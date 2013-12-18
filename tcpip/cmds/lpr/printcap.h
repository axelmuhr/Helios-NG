#ifdef __HELIOS
/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/printcap.h,v 1.1 1992/01/16 18:03:52 craig Exp $";
*/

extern int	getprent(char *) ;
extern void	endprent(void) ;
extern int	pgetent(char *, char *) ;
extern int	pgetnum(char *) ;
extern int	pgetflag(char *) ;
extern char	*pgetstr(char *, char **) ; 

#endif
