#ifdef __HELIOS
/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/utils.h,v 1.3 1992/02/25 11:00:58 craig Exp $";
*/

/* scandir */
#ifndef __helios_h
#include <helios.h>
#endif
#ifdef OLDCODE
extern int scan_dir (char *, struct direct *(*[]), WordFnPtr, WordFnPtr) ;
#else
int scan_dir (char *dir_name, struct direct *(*addr[]), 
              int (*)(struct direct *),
              int (*)(struct direct **, struct direct **)) ;
#endif
extern int scan_free (struct direct **) ;

/* lockmode */
extern int get_mode (char *) ;
extern int set_mode (char *, int) ;

/* killtask */
#ifdef OLDCODE
extern int kill_task (char *, int) ;
#else
extern int kill_task (char *) ;
#endif

/* debugf */
extern void debugf (char *, ...) ;

/* f_lock */
#include "f_lock.h"
int f_lock (int, int) ;
void f_lock_exit (void) ;

#endif
