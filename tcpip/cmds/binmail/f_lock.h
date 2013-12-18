#ifdef __HELIOS
/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/binmail/RCS/f_lock.h,v 1.1 1992/01/17 10:44:09 craig Exp $";
*/

/*
-- crf: Note - f_lock() does not distinguish between shared and exclusive locks
*/

#define LOCK_SH		1	/* shared lock */
#define LOCK_EX		2	/* exclusive lock */
#define LOCK_UN		8	/* unlock */

#endif

